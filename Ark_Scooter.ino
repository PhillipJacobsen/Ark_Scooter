/********************************************************************************
    Proof of Concept Scooter Rental
    https://hackmd.io/j3Ha1rlVQp-jTyyw9MU8FQ?both#Functional-Requirements

  Create an electric scooter rental solution utilizing a mobile app and an associated Ark custom bridgechain.
  All communication between the App and the IOT device will be on chain through the use of custom transactions.

    Ark_Scooter.ino
    2019 @phillipjacobsen

    Program Features:
    This program is designed to run on ESP32 Adafruit Huzzah.
    This sketch uses the ARK Cpp-Client API to interact with an Ark V2.6 Devnet node.
    Ark Cpp Client available from Ark Ecosystem <info@ark.io>
    Ark API documentation:  https://docs.ark.io/sdk/clients/usage.html

    Electronic Hardware Peripherals:
		Adafruit TFT FeatherWing 2.4" 320x240 Touchscreen:  https://www.adafruit.com/product/3315
    Adafruit GPS FeatherWing: https://www.adafruit.com/product/3133
    Optional: GPS external antenna. https://www.adafruit.com/product/960
    Optional(Required when using external antenna): SMA to uFL cable: https://www.adafruit.com/product/851
    Adafruit FeatherWing Doubler: https://www.adafruit.com/product/2890
********************************************************************************/

/********************************************************************************
                              Conditional Assembly
********************************************************************************/
#define NYBBLE   //this configures system for my custom bridgechain. If undefined then system will be configured for Ark Devnet.
#define ARDUINOJSON_USE_LONG_LONG 1   //this may not be required. Was used previously for compatibility with Telegram which used JSON v5 library

/********************************************************************************
                              Private Data
  IMPORTANT - Modify the secrets.h file with your network connection details
********************************************************************************/
#include "secrets.h"

/********************************************************************************
    EspMQTTClient Library by @plapointe6 Version 1.6.2
    WiFi and MQTT connection handler for ESP32
    This library does a nice job of encapsulating the handling of WiFi and MQTT connections.
    You just need to provide your credentials and it will manage the connection and reconnections to the Wifi and MQTT networks.
    EspMQTTClient is a wrapper around the MQTT PubSubClient Library Version 2.7 by @knolleary
********************************************************************************/
#define MQTT_MAX_PACKET_SIZE 256  // the maximum message size, including header, is 128 bytes by default. Configurable in PubSubClient.h.
#include "EspMQTTClient.h"

// configure these parameters in secrets.h
EspMQTTClient client(
  WIFI_SSID,
  WIFI_PASS,
  MQTT_SERVER_IP,   // MQTT Broker server ip
  MQTT_USERNAME,    // Can be omitted if not needed
  MQTT_PASSWORD,    // Can be omitted if not needed
  MQTT_CLIENT_NAME, // Client name that uniquely identify your device
  MQTT_SERVER_PORT  // The MQTT port, default to 1883. this line can be omitted
);


/********************************************************************************
    Adafruit GPS Library
********************************************************************************/
#include <Adafruit_GPS.h>
#define GPSSerial Serial1
// Connect to the GPS on the hardware serial port
Adafruit_GPS GPS(&GPSSerial);


/********************************************************************************
  driver libraries for ILI9341 2.4" 240x320 TFT FeatherWing display + touchscreen
    http://www.adafruit.com/products/3315 
  Adafruit GFX libraries
    graphics primitives documentation:  https://learn.adafruit.com/adafruit-gfx-graphics-library/graphics-primitives
    top left corner is (0,0)
********************************************************************************/
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>       //hardware specific library for display
#include <Adafruit_STMPE610.h>      //hardware specific library for the touch sensor
#include "bitmaps.h"                //bitmaps stored in program memory
#include <Fonts/FreeSans9pt7b.h>      //add custom fonts
#include <Fonts/FreeSansBold9pt7b.h>  //add custom fonts

// pin connections
#define STMPE_CS 32
#define TFT_CS   15
#define TFT_DC   33
#define SD_CS    14

#define Lcd_X  240       //configure your screen dimensions.  We aren't using an LCD for this project so I should rename to something more generic               
#define Lcd_Y  320       //configure your screen dimensions    

// my calibrated touchscreen data
#define TS_MINX 3800  //adjust left side of screen.  default: 3800 
#define TS_MAXX 250   //adjust right side of screen.  default: 100 
#define TS_MINY 205   //default: 100 
#define TS_MAXY 3750  //default: 3750 
#define MINPRESSURE 10
#define MAXPRESSURE 1000


Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);    //create TFT display object
Adafruit_STMPE610 ts = Adafruit_STMPE610(STMPE_CS);         //create Touchscreen object

int CursorX = 0;         //used to store current cursor position of the display
int CursorY = 0;         //used to store current cursor position of the display


//  use these tools to get 16bit hex color definitions  "5-6-5 16-bit mode"
//  http://www.barth-dev.de/online/rgb565-color-picker/
//  http://henrysbench.capnfatz.com/henrys-bench/arduino-adafruit-gfx-library-user-guide/arduino-16-bit-tft-rgb565-color-basics-and-selection/


// RGB565 Color Definitions
// This is a good tool for color conversions into RGB565 format
//http://www.barth-dev.de/online/rgb565-color-picker/

#define BLACK  ILI9341_BLACK
#define WHITE  ILI9341_WHITE
#define RED  ILI9341_RED
#define GREEN  ILI9341_GREEN

#define ArkRed 0xF1A7       // rgb(241, 55, 58)
#define ArkLightRed 0xFCD3  // rgb(248, 155, 156)





/********************************************************************************
    QRCode by Richard Moore version 0.0.1
        https://github.com/ricmoo/QRCode
    The QR code data encoding algorithm defines a number of 'versions' that increase in size and store increasing amounts of data.
    The version (size) of a generated QR code depends on the amount of data to be encoded.
    Increasing the error correction level will decrease the storage capacity due to redundancy pixels being added.

    If you have a ? in your QR text then I think the QR code operates in "Byte" mode.
********************************************************************************/
#include "qrcode.h"
const int QRcode_Version = 10;  // set the version (range 1->40)
const int QRcode_ECC = 2;       // set the Error Correction level (range 0-3) or symbolic (ECC_LOW, ECC_MEDIUM, ECC_QUARTILE and ECC_HIGH)
QRCode qrcode;                  // Create the QR code object
// QRcode Version = 10 with ECC=2 gives 211 Alphanumeric characters or 151 bytes(any characters)
char* QRcodeText;



/********************************************************************************
                              Library Requirements
********************************************************************************/
const int ledPin = 13;    //LED integrated in Adafruit HUZZAH32
int ledStatus = 0;

#define DAC1 25
#define DAC2 26

#define QRcodeDarkPixelColor   0xF1A7

//int ARK_mtbs = 8000; //mean time between polling Ark API for new transactions
uint32_t previousTime_1 = millis();
uint32_t previousTime_2 = millis();
uint32_t previousTime_3 = millis();


bool MQTT_status = false;
bool WiFI_status = false;
bool GPS_status = false;
bool ARK_status = false;

int battery = 0;
float batteryFloat;









/********************************************************************************

  // I NEED TO UPDATE COMMENTS FOR ESP32 module. These comments are for ESP8266

    Makuna NeoPixel Library - optimized for ESP8266
      Available through Arduino Library Manager however development is done using lastest Master Branch on Github
      https://github.com/Makuna/NeoPixelBus/

      This library is optimized to use the DMA on the ESP8266 for minimal cup usage. The standard Adafruit library has the potential to interfere with the
      WiFi processing done by the low level SDK
      NeoPixelBus<FEATURE, METHOD> strip(pixelCount, pixelPin);
       NeoPixelBus<NeoGrbFeature, Neo800KbpsMethod> strip(16);
      On the ESP8266 the Neo800KbpsMethod method will use this underlying method: NeoEsp8266Dma800KbpsMethod
      The NeoEsp8266Dma800KbpsMethod is the underlying method that gets used if you use Neo800KbpsMethod on Esp8266 platforms. There should be no need to use it directly.
      The NeoEsp8266Dma800KbpsMethod only supports the RDX0/GPIO3 pin. The Pin argument is omitted. See other esp8266 methods below if you don't have this pin available.
      This method uses very little CPU for actually sending the data to NeoPixels but it requires an extra buffer for the DMA to read from.
      Thus there is a trade off of CPU use versus memory use. The extra buffer needed is four times the size of the primary pixel buffer.
       It also requires the use of the RDX0/GPIO3 pin. The normal feature of this pin is the "Serial" receive.
      Using this DMA method will not allow you to receive serial from the primary Serial object; but it will not stop you from sending output to the terminal program of a PC
      Due to the pin overlap, there are a few things to take into consideration.
      First, when you are flashing the Esp8266, some LED types will react to the flashing and turn on.
      This is important if you have longer strips of pixels where the power use of full bright might exceed your design.
      Second, the NeoPixelBus::Begin() MUST be called after the Serial.begin().
      If they are called out of order, no pixel data will be sent as the Serial reconfigured the RDX0/GPIO3 pin to its needs.
********************************************************************************/
#include <NeoPixelBus.h>
#define PixelPin 12        //Neopixel Data Pin  connected to DMA
#define PixelCount 8       //Length of Neopixel Strand
NeoPixelBus<NeoGrbFeature, Neo800KbpsMethod> strip(PixelCount, PixelPin); //default on ESP8266 is to use the D9(GPIO3,RXD0) pin with DMA.

#define colorSaturation 128
RgbColor red(colorSaturation, 0, 0);
RgbColor green(0, colorSaturation, 0);
RgbColor blue(0, 0, colorSaturation);
RgbColor off(0, 0, 0);
RgbColor redgreen(colorSaturation, colorSaturation, 0);
RgbColor greenblue(0, colorSaturation, colorSaturation);
RgbColor black(0);







/********************************************************************************
   Ark Client Library (version 1.2.0)
    Available through Arduino Library Manager
    https://github.com/ArkEcosystem/cpp-client
********************************************************************************/
#include <arkClient.h>
/**
    This is where you define the IP address of an Ark Node (Or bridgechain node).
    You can find more Ark Mainnet and Devnet peers here: https://github.com/ArkEcosystem/peers
    The Public API port for the V2 Ark network is '4003'
*/




//Wallet Address on bridgechain
#ifdef NYBBLE
const char* ArkAddress = "TPW83DRkPcU9KyVZfCKrXMeAKDKExMhAnE";   //NYBBLE testnet address
char QRcodeArkAddress[] = "TPW83DRkPcU9KyVZfCKrXMeAKDKExMhAnE";   //jakeIOT testnet address
const char* ArkPublicKey = "02060c5793d2d42f11c8b18018c2c1ed5d81ed0ffc0afd0fe8ef5cee2dfbd3b787";       //jakeIOT testnet public key

//Wallet Address on Ark Devnet
#else
const char* ArkAddress = "DFcWwEGwBaYCNb1wxGErGN1TJu8QdQYgCt";   //Ark Devnet address
char QRcodeArkAddress[] = "DFcWwEGwBaYCNb1wxGErGN1TJu8QdQYgCt";   //Ark Devnet address
const char* ArkPublicKey = "029b2f577bd7afd878b258d791abfb379a6ea3c9436a73a77ad6a348ad48a5c0b9";       //Ark Devnet public key
//char *QRcodeArkAddress = "DHy5z5XNKXhxztLDpT88iD2ozR7ab5Sw2w";  //compiler may place this string in a location in memory that cannot be modified
#endif

char VendorID[64];

//define the payment timeout in ms
#define PAYMENT_WAIT_TIME 90000



/**
   This is how you define a connection while speficying the API class as a 'template argument'
   You instantiate a connection by passing a IP address as a 'c_string', and the port as an 'int'.
*/
Ark::Client::Connection<Ark::Client::Api> connection(ARK_PEER, ARK_PORT);
/**/


//I think a structure here for transaction details would be better form
//I need to do some work here to make things less hacky
//struct transactionDetails {
//   const char*  id;
//   int amount;
//   const char* senderAddress;
//   const char* vendorField;
//};

//--------------------------------------------
// these are used to store the received transation details returned from wallet search
const char*  id;            //transaction ID
int amount;                 //transactions amount
const char* senderAddress;  //transaction address of sender
const char* vendorField;    //vendor field

int lastRXpage;             //page number of the last received transaction in wallet
int searchRXpage;           //page number that is used for wallet search



/********************************************************************************
   Arduino Json Libary - works with Version5.  NOT compatible with Version6
    Available through Arduino Library Manager
    Data returned from Ark API is in JSON format.
    This libary is used to parse and deserialize the reponse
********************************************************************************/
#include <ArduinoJson.h>


/********************************************************************************
  Time Library
  required for internal clock to syncronize with NTP server.
  I need to do a bit more work in regards to Daylight savings time and the periodic sync time with the NTP service after initial syncronization
********************************************************************************/
#include "time.h"
//#include <TimeLib.h>    //https://github.com/PaulStoffregen/Time
//defined items below in secrets.h
//int timezone = -6;        //set timezone:  MST
//int dst = 0;              //To enable Daylight saving time set it to 3600. Otherwise, set it to 0. Not sure if this works.

time_t prevDisplayTime = 0; // time that was displayed on TFT

unsigned long timeNow;  //variable used to hold current millis() time.
unsigned long payment_Timeout;
unsigned long timeAPIfinish;  //variable used to measure API access time
unsigned long timeAPIstart;  //variable used to measure API access time



/********************************************************************************
  State Machine

********************************************************************************/
enum VendingMachineStates {DRAW_HOME, WAIT_FOR_USER, WAIT_FOR_PAY, VEND_ITEM};   //The five possible states of the Vending state machine
VendingMachineStates vmState = DRAW_HOME;   //initialize the starting state.

int ARK_mtbs = 8000; //mean time between polling Ark API for new transactions
long ARKscan_lasttime;   //last time Ark API poll has been done



/********************************************************************************
  Function prototypes
  Arduino IDE normally does its automagic here and creates all the function prototypes for you.
  We have put functions in other files so we need to manually add some prototypes as the automagic doesn't work correctly
********************************************************************************/
void setup();
int searchReceivedTransaction(const char *const address, int page, const char* &id, int &amount, const char* &senderAddress, const char* &vendorField );

//NeoPixels not yet connected.
//void ConfigureNeoPixels(RgbColor color);

void ArkVendingMachine();
void UpdateDisplayTime();
/********************************************************************************
  End Function Prototypes
********************************************************************************/



/********************************************************************************
  MAIN LOOP
********************************************************************************/
void loop() {
  client.loop();  //handle the wifi and MQTT connections

  //We need to call GPS.read() constantly in the main loop to watch for data arriving on the serial port
  //The hardware serial port has some buffer and perhaps arduino also configures some sort of FIFO.  This may set he buffer size: Serial1.setRxBufferSize(1024);
  // I need to learn more about the hardware buffer available on the ESP32 serial port.
  char c = GPS.read();

  //ask if a new chunk of data has been received by calling GPS.newNMEAreceived(), if this returns true then we can ask the library to parse that data with GPS.parse(GPS.lastNMEA()).
  if (GPS.newNMEAreceived()) {
    if (!GPS.parse(GPS.lastNMEA())) // this also sets the newNMEAreceived() flag to false
      return; // we can fail to parse a sentence in which case we should just wait for another
  }

  //  https://kd7dmp.net/2016/09/10/gps-speedometer/

  UpdateWiFiConnectionStatus();
  UpdateGPSConnectionStatus();
  UpdateDisplayTime();

  if (millis() - previousTime_1 > 4000)  {
    Serial.println(client.isConnected());
    previousTime_1 += 4000;
  }

  if (millis() - previousTime_2 > 3000)  {
    GPStoMQTT();
    previousTime_2 += 3000;
  }

  if (millis() - previousTime_3 > 10000)  {
    UpdateBatteryStatus();
    previousTime_3 += 10000;
  }
}
