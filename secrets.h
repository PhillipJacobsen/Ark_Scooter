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


#ifdef RADIANS
const char* ARK_PEER = "37.34.60.90";  //RADIANS Testnet Peer
int ARK_PORT = 4040;

#else
const char* peer = "159.203.42.124";  //Nybble Testnet Peer

//const char* ARK_PEER = "167.114.29.55";  //Ark Devnet Peer
////const char* ARK_PEER = "173.230.133.235";  //Ark Devnet Peer SPECIAL!
int ARK_PORT = 4003;
#endif



const char* rate = "370000000";   //rate per minute

//Wallet Address on bridgechain
#ifdef RADIANS
const char* ArkAddress = "TRXA2NUACckkYwWnS9JRkATQA453ukAcD1";   //RADIANS testnet address
//const char* ArkAddress = "TEf7p5jf1LReywuits5orBsmpkMe8fLTkk";   //RADIANS testnet address    genesis


//char QRcodeArkAddress[] = "TRXA2NUACckkYwWnS9JRkATQA453ukAcD1";   //RADIANS testnet address
//const char* ArkPublicKey = "03e063f436ccfa3dfa9e9e6ee5e08a65a82a5ce2b2daf58a9be235753a971411e2";       

//Wallet Address on Ark Devnet or Nybble Testnet

#else
const char* ArkAddress = "TYWtGL6m9g3E7QrhuDfNQ7XqHnVEnwLswG";   //NYBBLE testnet address  
//  http://159.203.42.124:4200/#/wallets/TYWtGL6m9g3E7QrhuDfNQ7XqHnVEnwLswG

//devnet
//const char* ArkAddress = "DFcWwEGwBaYCNb1wxGErGN1TJu8QdQYgCt";   //Ark Devnet address
//char QRcodeArkAddress[] = "DFcWwEGwBaYCNb1wxGErGN1TJu8QdQYgCt";   //Ark Devnet address
//const char* ArkPublicKey = "029b2f577bd7afd878b258d791abfb379a6ea3c9436a73a77ad6a348ad48a5c0b9";       //Ark Devnet public key
////char *QRcodeArkAddress = "DHy5z5XNKXhxztLDpT88iD2ozR7ab5Sw2w";  //compiler may place this string in a location in memory that cannot be modified
#endif
