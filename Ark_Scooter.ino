/********************************************************************************
    Proof of Concept Scooter Rental
    https://hackmd.io/j3Ha1rlVQp-jTyyw9MU8FQ?both#Functional-Requirements

  Create an electric scooter rental solution utilizing a mobile app and an associated Ark custom bridgechain.
  All communication between the App and the IOT device will be on chain through the use of custom transactions.

    Ark_Scooter.ino
    2020 @phillipjacobsen

    Program Features:
    This program is designed to run on ESP32 Adafruit Huzzah.
    This sketch uses the ARK Cpp-Client API to interact with a custom Ark V2.6 bridgechain.
    Ark Cpp Client available from Ark Ecosystem <info@ark.io>
    Ark API documentation:  https://docs.ark.io/sdk/clients/usage.html

    Electronic Hardware Peripherals:
		Adafruit TFT FeatherWing 2.4" 320x240 Touchscreen:  https://www.adafruit.com/product/3315
    Adafruit GPS FeatherWing: https://www.adafruit.com/product/3133
    Optional: GPS external antenna. https://www.adafruit.com/product/960
    Optional(Required when using external antenna): SMA to uFL cable: https://www.adafruit.com/product/851
    Adafruit FeatherWing Doubler: https://www.adafruit.com/product/2890


    Ark Library Verions
    Tested with:
    Ark-CPP-client v1.4.0-arduino
    change from v1.3->v1.4: added 2.6 endpoint
    https://github.com/ArkEcosystem/cpp-client/pull/159

    *****************  this is Simons version with Radians transaction support. *******************8
    https://github.com/sleepdefic1t/cpp-crypto/tree/chains/radians

    Ark-CPP-crypto v1.0.0
    bipp66 0.3.2
    https://github.com/sleepdefic1t/bip66
  See this file for library dependencies
  https://github.com/ArkEcosystem/cpp-crypto/blob/master/library.json#L23

  https://github.com/sleepdefic1t/bcl/releases/tag/0.0.5

  //see this library file for the radians specific transactions
  D:\Documents\Arduino\libraries\Ark-Cpp-Crypto\src\transactions\types\radians
  D:\Documents\Arduino\libraries\Ark-Cpp-Crypto\test\transactions\types\radians


  see this file for some string to number conversion helpers D:\Documents\Arduino\libraries\Ark-Cpp-Crypto\src\utils\str.hpp

********************************************************************************/

/********************************************************************************
                              Conditional Assembly
********************************************************************************/


/********************************************************************************
                              Private Data
  IMPORTANT - Modify the secrets.h file with your secure network connection details
********************************************************************************/
#include "secrets.h"


/********************************************************************************
                            Misc I/O Definitions
********************************************************************************/
const int LED_PIN = 13;     //LED integrated on Adafruit HUZZAH32 module

const int BAT_PIN = 35;     //ADC connected to Battery input pin (A13 = 35;)
//const int DAC1 = 25;      //declared in \packages\esp32\hardware\esp32\1.0.4\variants\feather_esp32/pins_arduino.h
//const int DAC2 = 26;


/********************************************************************************
                              Various Global Variables
********************************************************************************/
bool initialConnectionEstablished_Flag = false;   //used to detect first run after power up

bool WiFi_status = false;   // = true when connected to WiFi access point
bool GPS_status = false;    // = true when GPS has signal lock
bool ARK_status = false;    // = true when communication to Radians Bridgechain Node is working
bool MQTT_status = false;   // = true when connected to MQTT broker

int batteryPercent = 0;     // use to store battery level in percentage

float previousSpeed = 0;

/********************************************************************************
   Arduino Json Libary - Tested with version 6.13
    Data returned from Ark API is in JSON format.
    This libary is used to parse and deserialize the reponse

    This library is added by Ark crypto library so you do not need to include it here.

********************************************************************************/
//#include <ArduinoJson.h>


/********************************************************************************
  Library for reading/writing to the ESP32 flash memory.
  ESP32 Arduino libraries emulate EEPROM using a sector (4 kilobytes) of flash memory.
  The total flash memory size is ???
  The entire space is split between bootloader, application, OTA data, NVS, SPIFFS, and EEPROM.
  EEPROM library on the ESP32 allows using at most 1 sector (4kB) of flash.
********************************************************************************/
#include <EEPROM.h>


/********************************************************************************
    EspMQTTClient Library by @plapointe6 Version 1.8.0
    WiFi and MQTT connection handler for ESP32
    This library does a nice job of encapsulating the handling of WiFi and MQTT connections.
    You just need to provide your credentials and it will manage the connection and reconnections to the Wifi and MQTT networks.
    EspMQTTClient is a wrapper around the MQTT PubSubClient Library Version 2.7 by @knolleary
********************************************************************************/
// The MQTT packets are larger then the allowed for the default setting of the libary.
// You need to update this line in PubSubClient.h. Setting it here does nothing.
// If you update this library you will need to update this setting as it will be overwritten.
// #define MQTT_MAX_PACKET_SIZE 512  // the maximum message size, including header, is 128 bytes by default. Configurable in \Arduino\libraries\PubSubClient\src\PubSubClient.h.

#include "EspMQTTClient.h"

// configure these parameters in secrets.h
EspMQTTClient WiFiMQTTclient(
  WIFI_SSID,
  WIFI_PASS,
  MQTT_SERVER_IP,   // MQTT Broker server ip
  MQTT_USERNAME,    // Can be omitted if not needed
  MQTT_PASSWORD,    // Can be omitted if not needed
  MQTT_CLIENT_NAME, // Client name that uniquely identify your device
  MQTT_SERVER_PORT  // The MQTT port, default to 1883. this line can be omitted
);

/********************************************************************************
  This is the data packet that is sent to the CloudMQTT broker and then read by NodeRed client
*********************************************************************************/
struct MQTTpacket {
  const char* status;
  int battery;
  int fix;
  int satellites;
  float latitude;
  float longitude;
  float speedKPH;
  char walletBalance[65];
  char signature[140];        //is the signature always 140 characters?
};
struct MQTTpacket NodeRedMQTTpacket;


/********************************************************************************
    Adafruit GPS Library
********************************************************************************/
#include <Adafruit_GPS.h>
#define GPSSerial Serial1
Adafruit_GPS GPS(&GPSSerial);     // Connect to the GPS module via the hardware serial port


/********************************************************************************
  Libraries for ILI9341 2.4" 240x320 TFT FeatherWing display + touchscreen
    http://www.adafruit.com/products/3315
  Adafruit GFX libraries
    graphics primitives documentation:  https://learn.adafruit.com/adafruit-gfx-graphics-library/graphics-primitives
    top left corner is (0,0)

    http://oleddisplay.squix.ch/#/home     great tool for generating custom fonts.
********************************************************************************/
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>         //hardware specific library for display
#include <Adafruit_STMPE610.h>        //hardware specific library for the touch sensor
#include "bitmaps.h"                  //bitmaps stored in program memory
#include <Fonts/FreeSans9pt7b.h>      //add additional fonts from library
#include <Fonts/FreeSansBold18pt7b.h> //add additional fonts from library

#include "Fonts/Lato_Medium_36.h"     //add custom fonts
#include "Fonts/Lato_Semibold_48.h"   //add custom fonts
#include "Fonts/Lato_Black_96.h"      //add custom fonts

// pin connections
#define STMPE_CS 32
#define TFT_CS   15
#define TFT_DC   33
#define SD_CS    14

#define Lcd_X  240       //configure your screen dimensions.         
#define Lcd_Y  320       //configure your screen dimensions    

// my calibrated touchscreen data. Note: Resistive touchscreen is currently not being used.
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
    QRcode Version = 10 with ECC=2 gives 211 Alphanumeric characters or 151 bytes(any characters).  ECC=1 = 213 characters
********************************************************************************/
#include "qrcode.h"
const int QRcode_Version = 10;    // set the version (range 1->40)
const int QRcode_ECC = 1;         // set the Error Correction level (range 0-3) or symbolic (ECC_LOW, ECC_MEDIUM, ECC_QUARTILE and ECC_HIGH)
QRCode qrcode;                    // Create the QR code object

char QRcodeHash[64 + 1];
byte shaResult[32];

/********************************************************************************
  Time Library
  required for internal clock to syncronize with NTP server.
********************************************************************************/
#include "time.h"

//use these if you want to use millis() for measuring elapsed time for the ride timer.
uint32_t rideTime_start_ms;
uint32_t rideTime_length_ms;              //milliseconds

uint32_t remainingRentalTime_previous_s;  // seconds

time_t rideTime_start_seconds = 0;

//use these if you want to use time for measuring elapsed time for the ride timer.
time_t prevDisplayTime = 0;           // this is used if you want to update clock every second

//time variables use for clock on the display
int prevDisplayMinute = 0;            // this is used if you want to update clock every minute



/********************************************************************************
  Update Intervals for various algorithms
********************************************************************************/
//Frequency at which the MQTT packets are published
uint32_t UpdateInterval_MQTT_Publish = 15000;           // 15 seconds
uint32_t previousUpdateTime_MQTT_Publish = millis();

//Frequency at which the battery level is updated on the screen
uint32_t UpdateInterval_Battery = 7000;                 // 7 seconds
uint32_t previousUpdateTime_Battery = millis();

//Frequency at which the WiFi Receive Signal Level is updated on the screen
uint32_t UpdateInterval_RSSI = 5000;                    // 5 seconds
uint32_t previousUpdateTime_RSSI = millis();

//Frequency at which the Ark Network is polled looking for a rental start transaction
uint32_t UpdateInterval_RentalStartSearch = 8000;       // 8 seconds
uint32_t previousUpdateTime_RentalStartSearch = millis();

//Frequency at which the Speed and # GPS Satellites are updated on the screen
uint32_t UpdateInterval_GPS = 5000;
uint32_t previousUpdateTime_GPS = millis();

//These are some variables used to measure the access time when reading from the Ark network. This is needed only for testing
unsigned long timeNow;  //variable used to hold current millis() time.
unsigned long timeAPIfinish;  //variable used to measure API access time
unsigned long timeAPIstart;  //variable used to measure API access time



/********************************************************************************
     mbed TLS Library for SHA256 function

  https://techtutorialsx.com/2018/05/10/esp32-arduino-mbed-tls-using-the-sha-256-algorithm/#more-25918
  support for sha256

  hash generator to check results of library.
  https://passwordsgenerator.net/sha256-hash-generator/
********************************************************************************/
#include "mbedtls/md.h"
mbedtls_md_context_t ctx;
mbedtls_md_type_t md_type = MBEDTLS_MD_SHA256;      //select SHA256 algorithm



/********************************************************************************
    Ark Crypto Library (version 1.0.0)
 ================  NOTE: Version 1.1.0 is available however I have not yet tested with it ===========
      https://github.com/ArkEcosystem/Cpp-Crypto
    NOTE:
    If this Repo was Cloned from github, run the 'ARDUINO_IDE.sh' script first.
    It's in the 'extras/' folder and extends compatability to the Arduino IDE.

    Bip66 Library (version 0.3.2)
      https://github.com/sleepdefic1t/bip66
********************************************************************************/
#include <arkCrypto.h>
#include "arkCrypto_esp32.h"  // This is a helper header that includes all the Misc ARK C++ Crypto headers required for this sketch
#include "transactions/builders/radians/radians.hpp"

// Namespaces
using namespace Ark::Crypto;
using namespace Ark::Crypto::identities;
using namespace Ark::Crypto::transactions;

//const auto publicKey    = identities::Keys::fromPassphrase(Passphrase).publicKey;
//const auto pubKeyHash   = Hash::ripemd160(publicKey.data());
//const auto address      = Base58::parsePubkeyHash(pubKeyHash.data(), 65);
//const auto addressString = Address::fromPassphrase(Passphrase).toString.c_str();

// BridgeChain Network Structure Model.  see Ark-Cpp-Crypto\src\common\network.hpp
// These are defined in secrets.h
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
  Ark Client Library (version 1.4.0)
 ================  NOTE: Version 1.4.1 is available however I have not yet tested with it ===========  
  https://github.com/ArkEcosystem/cpp-client

  https://docs.ark.io/iot/#which-sdk-supports-iot
  https://docs.ark.io/tutorials/iot/storing-data-on-the-blockchain.html#step-1-project-setup
  https://docs.ark.io/tutorials/iot/reacting-to-data-on-the-blockchain.html#step-1-project-setup
********************************************************************************/
#include <arkClient.h>
Ark::Client::Connection<Ark::Client::Api> connection(ARK_PEER, ARK_PORT);   // create ARK blockchain connection



/********************************************************************************
  This structure is used to store all the details of a Rental session
********************************************************************************/
struct rental {
  char senderAddress[34 + 1];
  char payment[64 + 1];
  uint64_t payment_Uint64;
  // char rentalRate[64+1] = RENTAL_RATE_STR;
  char rentalRate[64 + 1];
  uint64_t rentalRate_Uint64;
  float QRLatitude;
  float QRLongitude;
  uint32_t startTime;
  float startLatitude;
  float startLongitude;
  uint32_t endTime;
  float endLatitude;
  float endLongitude;
  char vendorField[256 + 1];
  char sessionID[64 + 1];
};
struct rental scooterRental;

//todo: put this in the rental struct
const char* rentalStatus;     // Options: Available, Broken, Rented, Charging

int lastRXpage = 0;               //page number of the last received transaction in wallet
int lastRXpage_eeprom = 0;        //page number of the last received transaction in wallet(mirror of eeprom value)

char walletBalance[64 + 1];       
uint64_t walletNonce_Uint64 = 1ULL;
char walletNonce[64 + 1];
uint64_t walletBalance_Uint64 = 0ULL;



/********************************************************************************
  State Machine
********************************************************************************/
enum State_enum {STATE_0, STATE_1, STATE_2, STATE_3, STATE_4, STATE_5, STATE_6};    //The possible states of the state machine
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
  WiFiMQTTclient.loop();

  //--------------------------------------------
  // Parse GPS data if available
  // We need to call GPS.read() constantly in the main loop to watch for data arriving on the serial port
  // The hardware serial port has some buffer and perhaps arduino also configures some sort of FIFO.  This may set the buffer size???: Serial1.setRxBufferSize(1024);
  // I need to learn more about the hardware buffer available on the ESP32 serial port.
  char c = GPS.read();

  //ask if a new chunk of data has been received by calling GPS.newNMEAreceived(), if this returns true then we can ask the library to parse that data with GPS.parse(GPS.lastNMEA()).
  if (GPS.newNMEAreceived()) {
    if (!GPS.parse(GPS.lastNMEA())) // this also sets the newNMEAreceived() flag to false
      return; // we can fail to parse a sentence in which case we should just wait for another
  }


  //--------------------------------------------
  // Update all the data displayed on the OLED Status Bar
  UpdateWiFiConnectionStatus();     //update WiFi status bar
  UpdateMQTTConnectionStatus();     //update MQTT status bar
  UpdateGPSConnectionStatus();      //update GPS status bar
  UpdateGPSDataStatus();            //update GPS SAT and GPS Speed
  UpdateDisplayTime();              //update the clock every 1 second
  UpdateBatteryStatus();            //update battery status every UpdateInterval_Battery (5 seconds)
  UpdateRSSIStatus();               //update battery status every UpdateInterval_RSSI (5 seconds)

  //--------------------------------------------
  // Publish MQTT data every UpdateInterval_MQTT_Publish (15 seconds)
  send_MQTTpacket();

  
  //getMostRecentReceivedTransaction();

}
