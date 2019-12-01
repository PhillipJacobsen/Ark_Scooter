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
#define RADIANS   //this configures system for my custom bridgechain. If undefined then system will be configured for Ark Devnet.
//#define ARDUINOJSON_USE_LONG_LONG 1   //this may not be required. Was used previously for compatibility with Telegram which used JSON v5 library


//#include <Arduino.h>
/********************************************************************************
                              Private Data
  IMPORTANT - Modify the secrets.h file with your network connection details
********************************************************************************/
#include "secrets.h"

/********************************************************************************
                              Library Requirements
********************************************************************************/
const int LED_PIN = 13;    //LED integrated in Adafruit HUZZAH32
int ledStatus = 0;

const int BAT_PIN = 35;    //ADC connected to Battery input pin (A13 = 35;)
//const int DAC1 = 25;      //declared in \packages\esp32\hardware\esp32\1.0.4\variants\feather_esp32/pins_arduino.h
//const int DAC2 = 26;


/********************************************************************************
                              Global Variables
********************************************************************************/
bool initialConnectionEstablished_Flag = false;
bool WiFi_status = false;
bool GPS_status = false;
bool ARK_status = false;
bool MQTT_status = false;



int batteryPercent = 0;
//float batteryFloat;


/********************************************************************************
   Arduino Json Libary -
    Data returned from Ark API is in JSON format.
    This libary is used to parse and deserialize the reponse

    This library is added by Ark crypto library so you do not need to include it here.
********************************************************************************/
//#include <ArduinoJson.h>

/********************************************************************************
  Library for reading/writing to the ESP32 flash memory.
  ESP32 Arduino libraries emulate EEPROM using a sector (4 kilobytes) of flash memory.
  The total flash memory size is The entire space is split between bootloader, application, OTA data, NVS, SPIFFS, and EEPROM.
  EEPROM library on the ESP32\allows using at most 1 sector (4kB) of flash.
********************************************************************************/
#include <EEPROM.h>


/********************************************************************************
    EspMQTTClient Library by @plapointe6 Version 1.6.2
    WiFi and MQTT connection handler for ESP32
    This library does a nice job of encapsulating the handling of WiFi and MQTT connections.
    You just need to provide your credentials and it will manage the connection and reconnections to the Wifi and MQTT networks.
    EspMQTTClient is a wrapper around the MQTT PubSubClient Library Version 2.7 by @knolleary
********************************************************************************/
#define MQTT_MAX_PACKET_SIZE 512  // the maximum message size, including header, is 128 bytes by default. Configurable in PubSubClient.h.
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
  This is the data that is sent to the CloudMQTT broker and then red by NodeRed client
*********************************************************************************/
struct MQTTpacket {
  const char* status;
  int battery;
  int fix;
  int satellites;
  float latitude;
  float longitude;
  float speedKPH;
  char walletBalance[64];
};
struct MQTTpacket NodeRedMQTTpacket;

/********************************************************************************
    Adafruit GPS Library
********************************************************************************/
#include <Adafruit_GPS.h>
#define GPSSerial Serial1
Adafruit_GPS GPS(&GPSSerial);     // Connect to the GPS on the hardware serial port


/********************************************************************************
  Libraries for ILI9341 2.4" 240x320 TFT FeatherWing display + touchscreen
    http://www.adafruit.com/products/3315
  Adafruit GFX libraries
    graphics primitives documentation:  https://learn.adafruit.com/adafruit-gfx-graphics-library/graphics-primitives
    top left corner is (0,0)

    http://oleddisplay.squix.ch/#/home     awesome tool for generating custom font.
********************************************************************************/
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>         //hardware specific library for display
#include <Adafruit_STMPE610.h>        //hardware specific library for the touch sensor
#include "bitmaps.h"                  //bitmaps stored in program memory
#include <Fonts/FreeSans9pt7b.h>      //add custom fonts
#include <Fonts/FreeSansBold18pt7b.h>  //add custom fonts
//#include <Fonts/FreeSansBold24pt7b.h>  //add custom fonts

#include <Fonts/Lato_Medium_36.h>  //add custom fonts
//#include <Fonts/Lato_Black_56.h>  //add custom fonts
#include <Fonts/Lato_Semibold_48.h>  //add custom fonts
//#include <Fonts/Lato_Black_88.h>  //add custom fonts
#include <Fonts/Lato_Black_96.h>  //add custom fonts

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

// RGB565 Color Definitions
// This is a good tool for color conversions into RGB565 format
// http://www.barth-dev.de/online/rgb565-color-picker/
#define BLACK   ILI9341_BLACK
#define WHITE   ILI9341_WHITE
#define RED     ILI9341_RED
#define GREEN   ILI9341_GREEN
#define ArkRed  0xF1A7                // rgb(241, 55, 58)
#define ArkLightRed 0xFCD3            // rgb(248, 155, 156)
#define OffWhite 0xCE59               // rgb(202, 202, 202)
#define SpeedGreen 0xAFF5             // rgb(170, 255, 170)
#define SpeedGreenDarker 0x0760       // rgb(0, 236, 0)
#define QRCODE_DARK_PIXEL_COLOR 0xF1A7

int CursorX = 0;         //used to store current cursor position of the display
int CursorY = 0;         //used to store current cursor position of the display


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

//char* QRcodeText;               // QRcode Version = 10 with ECC=2 gives 211 Alphanumeric characters or 151 bytes(any characters)
char* QRcodeHash_pntr;               // QRcodeHash. This is
char QRcodeHash[256];

/********************************************************************************
  Time Library
  required for internal clock to syncronize with NTP server.
********************************************************************************/
#include "time.h"

//use these if you want to use millis() for measuring elapsed time for the ride timer.
uint32_t rideTime_start_ms;
uint32_t rideTime_length_ms;     //milliseconds

time_t rideTime_start_seconds = 0;
//time_t rideTime_start_seconds = 0;

//use these if you want to use time for measuring elapsed time for the ride timer.
time_t prevDisplayTime = 0; // this is used if you want to update clock every second


//time variables use for clock on the display
//time_t prevDisplayTime = 0; // this is used if you want to update clock every second
int prevDisplayMinute = 0;  // this is used if you want to update clock every minute

//Frequency at which the MQTT packets are published
uint32_t UpdateInterval_MQTT_Publish = 15000;
uint32_t previousUpdateTime_MQTT_Publish = millis();

//Frequency at which the battery level is updated on the screen
uint32_t UpdateInterval_Battery = 7000;
uint32_t previousUpdateTime_Battery = millis();

//Frequency at which the WiFi Receive Signal Level is updated on the screen
uint32_t UpdateInterval_RSSI = 5000;
uint32_t previousUpdateTime_RSSI = millis();

//Frequency at which the Ark Network is polled looking for a rental start transaction
uint32_t UpdateInterval_RentalStartSearch = 8000;
uint32_t previousUpdateTime_RentalStartSearch = millis();

//Frequency at which the Speed and # Satellites is updated on the screen
uint32_t UpdateInterval_GPS = 5000;
uint32_t previousUpdateTime_GPS = millis();

//These are some variables used to measure the access time when reading from the Ark network.
unsigned long timeNow;  //variable used to hold current millis() time.
unsigned long timeAPIfinish;  //variable used to measure API access time
unsigned long timeAPIstart;  //variable used to measure API access time

/********************************************************************************

    Ark Crypto Library (version 0.7.0)
      https://github.com/ArkEcosystem/Cpp-Crypto

    Bip66 Library (version 0.2.0)
      https://github.com/sleepdefic1t/bip66

********************************************************************************/
#include <arkCrypto.h>
#include "arkCrypto_esp32.h"  // This is a helper header that includes all the Misc ARK C++ Crypto headers required for this sketch

// Namespaces
using namespace Ark::Crypto;
using namespace Ark::Crypto::identities;
using namespace Ark::Crypto::transactions;

//const auto publicKey    = identities::Keys::fromPassphrase(Passphrase).publicKey;
//const auto pubKeyHash   = Hash::ripemd160(publicKey.data());
//const auto address      = Base58::parsePubkeyHash(pubKeyHash.data(), 65);
//const auto addressString = Address::fromPassphrase(Passphrase).toString.c_str();

//BridgeChain Network Structure Model.  see Ark-Cpp-Crypto\src\common\network.hpp
const Network BridgechainNetwork = {
  BRIDGECHAIN_NETHASH,
  BRIDGECHAIN_SLIP44,
  BRIDGECHAIN_WIF,
  BRIDGECHAIN_VERSION,
  BRIDGECHAIN_EPOCH
};

// Load the Custom Network Configuration
const Configuration cfg(BridgechainNetwork);


/********************************************************************************
  Ark Client Library (version 1.3.0)
  https://github.com/ArkEcosystem/cpp-client

  https://docs.ark.io/iot/#which-sdk-supports-iot
  https://docs.ark.io/tutorials/iot/storing-data-on-the-blockchain.html#step-1-project-setup
  https://docs.ark.io/tutorials/iot/reacting-to-data-on-the-blockchain.html#step-1-project-setup
********************************************************************************/
#include <arkClient.h>
Ark::Client::Connection<Ark::Client::Api> connection(ARK_PEER, ARK_PORT);   // create ARK blockchain connection

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
// https://www.geeksforgeeks.org/difference-const-char-p-char-const-p-const-char-const-p/
//pointers to constant charaters. you cannot change the value but you can change the pointer
const char*  id;              //transaction ID
const char* amount;           //transactions amount
const char* senderAddress;    //transaction address of sender
const char* senderPublicKey;  //transaction address of sender
const char* vendorField;      //vendor field

//NOTES!!!!!!!!!!!!!!
//const char* is a pointer to memory that hopefully contains a null-terminated string.
//A char* points to the memory location of a sequence of multiple chars.
//char sz[] = {'t', 'e', 's', 't', 0};    //C-string
//const char *psz = "test";

//https://accu.org/index.php/journals/1445  char* x is the same as char *x


int lastRXpage = 0;             //page number of the last received transaction in wallet
int searchRXpage = 0;           //page number that is used for wallet search

char walletBalance[64];
uint64_t walletNonce_Uint64 = 1ULL;    
char walletNonce[64];
uint64_t walletBalance_Uint64 = 0ULL;  
   


/********************************************************************************
  State Machine

********************************************************************************/

enum State_enum {STATE_0, STATE_1, STATE_2, STATE_3, STATE_4, STATE_5, STATE_6};  //The possible states of the state machine
State_enum state = STATE_0;     //initialize the starting state.


/********************************************************************************
  Function prototypes
  Arduino IDE normally does its automagic here and creates all the function prototypes for you.
  We have put functions in other files so we need to manually add some prototypes as the automagic doesn't work correctly
********************************************************************************/
void setup();
int GetReceivedTransaction(const char *const address, int page, const char* &id, const char* &amount, const char* &senderAddress, const char* &senderPublicKey, const char* &vendorField );
int getMostRecentReceivedTransaction();
void UpdateDisplayTime();
void UpdateWiFiConnectionStatus();
void UpdateGPSConnectionStatus();
void UpdateGPSDataStatus();
void UpdateMQTTConnectionStatus();
void UpdateDisplayTime();
void GPStoMQTT();
void UpdateBatteryStatus();
void StateMachine();
void UpdateRSSIStatus();


/********************************************************************************
  MAIN LOOP
********************************************************************************/
void loop() {

  //--------------------------------------------
  // Process state machine
  StateMachine();

  //--------------------------------------------
  // Handle the WiFi and MQTT connections
  client.loop();

  //--------------------------------------------
  // Parse GPS data if available
  // We need to call GPS.read() constantly in the main loop to watch for data arriving on the serial port
  // The hardware serial port has some buffer and perhaps arduino also configures some sort of FIFO.  This may set he buffer size???: Serial1.setRxBufferSize(1024);
  // I need to learn more about the hardware buffer available on the ESP32 serial port.
  char c = GPS.read();

  //ask if a new chunk of data has been received by calling GPS.newNMEAreceived(), if this returns true then we can ask the library to parse that data with GPS.parse(GPS.lastNMEA()).
  if (GPS.newNMEAreceived()) {
    if (!GPS.parse(GPS.lastNMEA())) // this also sets the newNMEAreceived() flag to false
      return; // we can fail to parse a sentence in which case we should just wait for another
  }


  //--------------------------------------------
  // Update all the data displayed on the Status Bar
  UpdateWiFiConnectionStatus();     //update WiFi status bar
  UpdateMQTTConnectionStatus();     //update MQTT status bar
  UpdateGPSConnectionStatus();      //update GPS status bar
  UpdateGPSDataStatus();            //update GPS SAT and GPS Speed
  UpdateDisplayTime();              //update the clock every 1 second
  UpdateBatteryStatus();            //update battery status every UpdateInterval_Battery (5 seconds)
  UpdateRSSIStatus();               //update battery status every UpdateInterval_RSSI (5 seconds)

  //getMostRecentReceivedTransaction();




  //--------------------------------------------
  // Update all the data on the Status Bar
  //  if (millis() - previousTime_1 > 4000)  {
  //   Serial.println(client.isConnected());
  //   previousTime_1 += 4000;
  // }

  //--------------------------------------------
  // Publish MQTT data every UpdateInterval_MQTT_Publish (3 seconds)

  send_MQTTpacket();

  //  if (millis() - previousUpdateTime_MQTT_Publish > UpdateInterval_MQTT_Publish)  {
  //      GPStoMQTT();
  //    previousUpdateTime_MQTT_Publish += UpdateInterval_MQTT_Publish;
  //  }




}
