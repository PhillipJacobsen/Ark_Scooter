/*
  #define MQTT_SERVER_IP		"*****"
  #define MQTT_USERNAME		  "*****"
  #define MQTT_PASSWORD		  "*****"
  #define MQTT_CLIENT_NAME	"*****"
  #define	MQTT_SERVER_PORT 	*****
  #define WIFI_SSID         "*****"
  #define WIFI_PASS         "*****"
*/

//h
#define WIFI_SSID         "TELUS0357"
#define WIFI_PASS         "77kmm7r7hz"

//w
//#define WIFI_SSID         "TELUS6428"
//#define WIFI_PASS         "3mmkgc9gn2"

//configure MQTT Broker.
#define MQTT_SERVER_IP    "40.85.223.207"
#define MQTT_USERNAME     "esp32"
#define MQTT_PASSWORD     "compost2"
#define MQTT_CLIENT_NAME  "TestClient"
#define MQTT_SERVER_PORT  1883

//configure MQTT topic.
//NOTE. Thingsboard Dashboard is hardcoded with this topic so if you adjust the wallet do not modify the topic.
const char* MQTT_Base_Topic = "scooter/TRXA2NUACckkYwWnS9JRkATQA453ukAcD1/data";

//int8_t TIME_ZONE = -6;      //set timezone:  MST (use this in summer)
int8_t TIME_ZONE = -7;        //set timezone:  MST (use this in winter)
int16_t DST = 3600;           //To enable Daylight saving time set it to 3600. Otherwise, set it to 0. I am not sure if this actually works. Need to confirm in the fall...

//Configure Bridgechain Relay
//const char* ARK_PEER = "37.34.60.90";  //RADIANS Testnet Peer 
//int ARK_PORT = 4040;
const char* ARK_PEER = "138.197.165.189";  //RADIANS Testnet Peer (pj)
int ARK_PORT = 4103;

// Configure Radians Wallet for Scooter
const char* ArkAddress = "TUtc5kn9PnVJZKyAvovBBacHtmmiaK9Stv";
//const char* ArkPublicKey = "03e063f436ccfa3dfa9e9e6ee5e08a65a82a5ce2b2daf58a9be235753a971411e2";    //
static const auto PASSPHRASE  = "steel digital luxury lawsuit floor script pig knock uniform all sick embark";  //  TUtc5kn9PnVJZKyAvovBBacHtmmiaK9Stv

// Configure Bridgechain Parameters
static const auto BRIDGECHAIN_NETHASH   = "314ccfc8c437e10cccb527ee6726be606da8fbaebe54c5c105df30882511c25a"; // std::string
static const auto BRIDGECHAIN_SLIP44    = 1;          // uint8_t
static const auto BRIDGECHAIN_WIF       = 0x91;       // uint8_t      145
static const auto BRIDGECHAIN_VERSION   = 0x41;       // uint8_t      65
static const auto BRIDGECHAIN_EPOCH     = "2020-05-12T11:34:19.156Z";  // std::string
//constexpr uint64_t TYPE_0_FEE           = 1000000ULL;   //0.01RAD
//constexpr uint8_t TYPE_0_TYPE           = 0U;

//Configure the Rental rate of the scooter.  Units are RAD per Second
const char* RENTAL_RATE_STR = "2";                //rate per second
constexpr uint64_t RENTAL_RATE_UINT64 = 2ULL;     //.037RAD

//--------------------------------------------
// If you are reprogramming a new wallet address into an existing ESP32 module you need to erase the Flash which stores the number of received transactions in the wallet.
// 1. Define ERASE_FLASH below
// 2. Download firmware
// 3. undefine ERASE_FLASH and reprogram
//#define ERASE_FLASH

//--------------------------------------------
// Wireless Firmware Updating
// 1.to generate .bin firmware image go to "Export Compiled Binary" in the Arduino IDE's "Sketch" menu
// 2. In Adduino IDE go to tools->Partition Scheme and set to: Minimal SPIFFS(Large APPS with OTA)
// go to http://IPaddress   IPaddress of the ESP32 module is displayed on terminal after powerup.
// enter MQTT_USERNAME and MQTT_PASSWORD
//upload .bin file
#define ENABLE_WIRELESS_UPDATE
