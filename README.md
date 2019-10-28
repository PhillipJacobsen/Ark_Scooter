# Proof of Concept Scooter Rental

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

TFT FeatherWing 2.4" 320x240 Touchscreen
  Source: https://www.adafruit.com/product/3315
		TFT_CS 	-> pin #15
		TFT_DC 	-> pin #33
		RT 		-> pin #32
		SD		-> pin #14
		SCK		-> SCK
		MISO	-> MISO
		MOSI	-> MOSI

Adafruit GPS FeatherWing:
  Source: https://www.adafruit.com/product/3133

pin 21: lock control

A0 / GPIO26: DAC2
A1 / GPIO25: DAC1

LED -> pin 13  (LED integrated on Huzzah module)

NEOPIXELS NOT CONNECTED YET.
	NEOPIXEL-> pin
	VCC -> 3.3V
	GND
