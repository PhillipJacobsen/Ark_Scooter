# Proof of Concept Scooter Rental
https://radians.nl/#/

http://165.22.237.171:8080/dashboard/f88894b0-d519-11e9-b281-0bd830c6c87f?publicId=f62146f0-cb7c-11e9-b281-0bd830c6c87f

## Program Features
This program is designed to run on ESP32 Adafruit Huzzah.  
This sketch uses the ARK Cpp-Client API to interact with a custom ARK.io bridgechain.
Ark Cpp Client available from Ark Ecosystem <info@ark.io>  
Ark API documentation:  https://docs.ark.io/sdk/clients/usage.html  

## Electronic Hardware Peripherals
Adafruit TFT FeatherWing 2.4" 320x240 Touchscreen:  https://www.adafruit.com/product/3315  
Adafruit GPS FeatherWing: https://www.adafruit.com/product/3133  
Optional: GPS external antenna. https://www.adafruit.com/product/960  
Optional(Required when using external antenna): SMA to uFL cable: https://www.adafruit.com/product/851   
Adafruit FeatherWing Doubler: https://www.adafruit.com/product/2890  

## Embedded Firmware
Firmware is developed using Arduino / PlatformIO environment and Ark C++ SDK.  

### Arduino Development Environment Setup
The following are detailed steps on setting up Arduino programming environment.  
The following process is testing on Windows 10 environment but should be similar on other operating systems.
1. Download and install Arduino  
    * https://www.arduino.cc/en/Main/Software
2. Run Arduino
3. Install ESP32 processor board hardware library
    1. File->Preferences  
    2. Enter https://dl.espressif.com/dl/package_esp32_index.json into the “Additional Board Manager URLs” field and select ok
    3. Open the Boards Manager. Tools->Board->Boards Manager
    4. Search for ESP32 and install "ESP32 by Espressif Systems" version 1.0.4.  Some times this can take a while to download
    5. ESP32 support should now be installed  

### Install Firmware Library Dependencies
The following are steps to install dependencies for the Scooter fimware project. 

Install the following libraries via the Arduino library manager.  
Tools->Manage Libraries.  
You can then search and add each of the following libraries and versions indicated.

* EspMQTTClient by Patrick Lapointe Ver 1.8.0
* PubSubClient by Nick O'Leary Ver 2.7.0 
* Adafruit GPS by Adafruit Ver 1.4.1
* Adafruit GFX by Adafruit Ver 1.7.5
* Adafruit Touchscreen by Adafruit 1.0.5
* Adafruit ILI9341 by Adafruit Ver 1.5.4
* Adafruit STMPE610 by Adafruit Ver 1.1.1
* QRCode by Richard Moore Ver 0.0.1
* ArduinoJson by Benoit Blanchon Ver 6.13.0
* BIP66 by Ark Ecosystem Ver 0.3.2
* bcl by Project Nayuki Ver 0.0.5
* micro-ecc by Kenneth MacKay Ver 1.0.0
* Ark-Cpp-Client by Ark Ecosystem V 1.4.0-arduino

### Installing Ark SDK Crypto C++ Library with support for custom Radians bridgechain transactions
The standard Ark-Cpp-Crypto (Ver 1.0.0) by Ark.io was forked to include custom Radians transaction support.(thanks to @sleepdeficit for support)  

1. Download the chains\Radians branch(not the main branch) from:
    * https://github.com\sleepdefic1t\cpp-crypto\tree\chains\radians
2. Move cpp-crypto folder to \Arduino\libraries
3. rename cpp-crypto to Ark-Cpp-Crypto  

A bash script needs to be executed to format the library for use with Arduino development. Windows10 does not have native support for this. I use "git for windows" which has gitbash included.  
https://gitforwindows.org/  
After installation of gitbash right click in \Ark-Cpp-Crypto\extras\ folder and select "GIT Bash Here".

4. gitbash shell window should open. Run "sh ARDUINO_IDE.sh"
    * script should start and ask for confirmation to continue. Press y
    * script should complete with message "You can now use Cpp-Crypto with the Arduino IDE".

### Library Modification
In the Arduino\libraries folder open \PubSubClient\src\PubSubClient.h  
Change this line: #define MQTT_MAX_PACKET_SIZE 128 
to:   #define MQTT_MAX_PACKET_SIZE 512


### Compiling Scooter Firmware
1. Download Scooter firmware: 
https://github.com/PhillipJacobsen/Ark_Scooter
2. Open Ark_Scooter.ino in Arduino
3. TBD: Change compiler error/warning settings(not needed in windows)
    * Todo need to check this for compilation on MacOS 
    * need to check default error/warning settings after installing Arduino
5. Select the processor board 
    * Tools->Board->Adafruit ESP32 Feather
6. Compile Firmware
    * Sketch->Verify/Compile

### Download Firmware
TBD
