# Proof of Concept Scooter Rental

Requires:
Ark-Cpp-Crypto v0.7.0
Ark-Cpp-Client v 1.4.0-arduino

notes: 
//IOT example projects   https://github.com/debsahu/ESP-MQTT-AWS-IoT-Core/issues
//good example of good reconnect code for mqtt and wifi. also good ntp time sync
//https://github.com/debsahu/ESP-MQTT-AWS-IoT-Core/blob/master/Arduino/PubSubClient/PubSubClient.ino


## Program Features
This program is designed to run on ESP32 Adafruit Huzzah.  
This sketch uses the ARK Cpp-Client API to interact with an Ark V2.6 Devnet node.  
Ark Cpp Client available from Ark Ecosystem <info@ark.io>  
Ark API documentation:  https://docs.ark.io/sdk/clients/usage.html  


## Electronic Hardware Peripherals
Adafruit TFT FeatherWing 2.4" 320x240 Touchscreen:  https://www.adafruit.com/product/3315  
Adafruit GPS FeatherWing: https://www.adafruit.com/product/3133  
Optional: GPS external antenna. https://www.adafruit.com/product/960  
Optional(Required when using external antenna): SMA to uFL cable: https://www.adafruit.com/product/851   
Adafruit FeatherWing Doubler: https://www.adafruit.com/product/2890  


## Electronic Pin Connections
ESP32 Adafruit Huzzah  
  Source: https://www.adafruit.com/product/3213  
  https://randomnerdtutorials.com/esp32-pinout-reference-gpios/  

**TFT FeatherWing 2.4" 320x240 Touchscreen**
Source: https://www.adafruit.com/product/3315  
TFT_CS 	-> pin #15  
TFT_DC 	-> pin #33  
RT 	-> pin #32  
SD	-> pin #14  
SCK	-> SCK  
MISO	-> MISO  
MOSI	-> MOSI  

**Adafruit GPS FeatherWing**
Source: https://www.adafruit.com/product/3133

pin 21: lock control

**ADC**
A0 / GPIO26: DAC2  
A1 / GPIO25: DAC1  

LED -> pin 13  (LED integrated on Huzzah module)  

NEOPIXELS NOT CONNECTED YET.  
	NEOPIXEL-> pin  
	VCC -> 3.3V  
	GND  
	
	
## Libraries 
### EspMQTTClient by @plapointe6 Version 1.6.2
    WiFi and MQTT connection handler for ESP32
    This library does a nice job of encapsulating the handling of WiFi and MQTT connections.
    You just need to provide your credentials and it will manage the connection and reconnections to the Wifi and MQTT networks.
      Available through Arduino Library Manager
        https://github.com/plapointe6/EspMQTTClient

    EspMQTTClient is a wrapper around the MQTT PubSubClient Library Version 2.7 by @knolleary
    PubSubClient is a MQTT client for doing simple publish/subscribe messaging
        https://github.com/knolleary/pubsubclient

    Full API documentation of PubSubClient is available here: https://pubsubclient.knolleary.net

    Limitations of PubSubClient
      It can only publish QoS 0 messages. It can subscribe at QoS 0 or QoS 1.
      The maximum message size, including header, is 128 bytes by default. This is configurable via MQTT_MAX_PACKET_SIZE in PubSubClient.h.
      The keepalive interval is set to 15 seconds by default. This is configurable via MQTT_KEEPALIVE in PubSubClient.h.
      The client uses MQTT 3.1.1 by default. It can be changed to use MQTT 3.1 by changing value of MQTT_VERSION in PubSubClient.h.


