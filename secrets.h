/*
  #define MQTT_SERVER_IP		"*****"
  #define MQTT_USERNAME		  "*****"
  #define MQTT_PASSWORD		  "*****"
  #define MQTT_CLIENT_NAME	"*****"
  #define	MQTT_SERVER_PORT 	*****

  #define WIFI_SSID         "*****"
  #define WIFI_PASS         "*****"
*/


#define MQTT_SERVER_IP    "40.85.223.207"
#define MQTT_USERNAME     "esp32"
#define MQTT_PASSWORD     "compost2"
#define MQTT_CLIENT_NAME  "TestClient"
#define MQTT_SERVER_PORT  1883

//h
#define WIFI_SSID         "TELUS0183"
#define WIFI_PASS         "6z5g4hbdxi"

//w
//#define WIFI_SSID         "TELUS6428"
//#define WIFI_PASS         "3mmkgc9gn2"

//pj
//#define WIFI_SSID         "hppj"
//#define WIFI_PASS         "1compost2"

//use this library to handle daylight savings
// https://github.com/JChristensen/Timezone

//int8_t TIME_ZONE = -6;        //set timezone:  MST (works in summer)
int8_t TIME_ZONE = -7;        //set timezone:  MST (works in winter)
int16_t DST = 0;            //To enable Daylight saving time set it to 3600. Otherwise, set it to 0. Not sure if this works.


const char* ARK_PEER = "37.34.60.90";  //RADIANS Testnet Peer
int ARK_PORT = 4040;


//Wallet Address on bridgechain
const char* ArkAddress = "TRXA2NUACckkYwWnS9JRkATQA453ukAcD1";   //RADIANS testnet address  - nickname pjtest
//const char* ArkPublicKey = "03e063f436ccfa3dfa9e9e6ee5e08a65a82a5ce2b2daf58a9be235753a971411e2";       

static const auto PASSPHRASE        = "afford thumb forward wall salad diet title patch holiday metal cement wisdom";  //TRXA2NUACckkYwWnS9JRkATQA453ukAcD1
static const auto SecondPassphrase  = "this is a top secret passphrase too";
static const auto MessageText       = "Hello World";
//static const auto WifByte           = 0xaa;  // Ark Devnet

static const auto BRIDGECHAIN_NETHASH           = "f39a61f04d6136a690a0b675ef6eedbd053665bd343b4e4f03311f12065fb875"; // std::string
static const auto BRIDGECHAIN_SLIP44            = 1;          // uint8_t
static const auto BRIDGECHAIN_WIF               = 0xCE;       // uint8_t
static const auto BRIDGECHAIN_VERSION           = 0x41;       // uint8_t
static const auto BRIDGECHAIN_EPOCH             = "2019-10-25T09:05:40.856Z";  // std::string

constexpr uint64_t TYPE_0_FEE               = 1000000ULL;   //0.01RAD
constexpr uint8_t TYPE_0_TYPE               = 0U;
//constexpr uint32_t TYPE_0_TIMESTAMP         = 83506126UL;
//constexpr uint32_t TYPE_0_VF_TIMESTAMP      = 83506245;
//constexpr uint64_t TYPE_0_FEE               = 10000000ULL;
//constexpr uint64_t TYPE_0_AMOUNT            = 1ULL;
//constexpr uint32_t TYPE_0_EXPIRATION        = 0UL;

const char* RENTAL_RATE_STR = "3700000";                //rate per minute
constexpr uint64_t RENTAL_RATE_UINT64 = 3700000ULL;    //.037RAD
