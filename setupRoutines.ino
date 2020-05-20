/********************************************************************************
  This file contains functions used to configure hardware peripherals and various libraries.
********************************************************************************/

/********************************************************************************
  Configure peripherals and system
********************************************************************************/
void setup()
{
  Serial.begin(115200);             // Initialize Serial Connection for debug
  while ( !Serial && millis() < 20 );

  pinMode(LED_PIN, OUTPUT);         // initialize on board LED control pin as an output.
  digitalWrite(LED_PIN, HIGH);      // Turn LED on

  //--------------------------------------------
  // If you are reprogramming a new wallet address into an existing ESP32 module you need to erase the Flash which stores the number of received transactions in the wallet.
  // 1. Define ERASE_FLASH in secrets.h
  // 2. Download firmware
  // 3. undefine ERASE_FLASH and reprogram
#ifdef ERASE_FLASH
  clearEEPROM();
  while (true) {
  }
#endif

  //--------------------------------------------
  // read the MAC address.
  // currently this is not being used but likely will be used as unique identifier as part of the MQTT connect sequence
  // Use this to get MAC address of ESP32 prior to WiFi being enabled.
  uint8_t baseMac[6];
  esp_read_mac(baseMac, ESP_MAC_WIFI_STA);      // Get MAC address for WiFi station
  char baseMacChr[18] = {0};
  sprintf(baseMacChr, "%02X%02X%02X%02X%02X%02X", baseMac[0], baseMac[1], baseMac[2], baseMac[3], baseMac[4], baseMac[5]);  // Example: B4E62DA8EF6D
  Serial.print("MAC Address: ");
  Serial.println(baseMacChr);

  //--------------------------------------------
  // Optional Features of EspMQTTClient
  //WiFiMQTTclient.enableDebuggingMessages(); // Enable debugging messages sent to serial output
#ifdef ENABLE_WIRELESS_UPDATE
  WiFiMQTTclient.enableHTTPWebUpdater(); // Enable the web updater. User and password default to values of MQTTUsername and MQTTPassword. These can be overwritten with enableHTTPWebUpdater("user", "password").
#endif
  WiFiMQTTclient.enableLastWillMessage("scooter/TRXA2NUACckkYwWnS9JRkATQA453ukAcD1/lastwill", "{\"status\":\"Broken\"}");  // You can activate the retain flag by setting the third parameter to true

  WiFiMQTTclient.enableMQTTConnect();               //experimental - requires modified MQTT library
  WiFiMQTTclient.enableMACaddress_for_ClientName(); //experimental - requires modified MQTT library

  //--------------------------------------------
  //configure the 2.4" TFT display and the touchscreen controller
  setupDisplayTouchscreen();

  //--------------------------------------------
  // Bootup Screen
  // Display Ark bitmap on middle portion of screen
  DisplayArkBitmap();

  //--------------------------------------------
  // 9600 NMEA is the default baud rate for Adafruit MTK GPS
  GPS.begin(9600);
  GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);   // turn on RMC (recommended minimum) and GGA (fix data) including altitude
  GPS.sendCommand(PMTK_SET_NMEA_UPDATE_1HZ);      // 1 Hz update rate
  GPS.sendCommand(PGCMD_ANTENNA);                 // Request updates on antenna status, comment out to keep quiet

  delay(500);                             //show bootup screen for 500ms

  tft.setTextColor(WHITE);
  tft.setFont(&FreeSans9pt7b);
  tft.setCursor(50, 280);
  tft.println("Connecting to WiFi");      // display WiFi connect message
  tft.setCursor(70, 300);
  tft.println(WIFI_SSID);

  delay(1200);                            // show bootup screen for additional 1200ms

  scooterRental.rentalStatus = "Broken";  // set default scooter status displayed in Analytics Dashboard

  InitStatusBar();                        // display the status bar on the bottom of the screen
  UpdateBatteryStatus();                  // update battery voltage on the status bar

  GPSSerial.println(PMTK_Q_RELEASE);      // request firmware version from GPS module. This can be used as a way to detect if GPS module is connected and operational.
}



/********************************************************************************
  This function is called once WiFi and MQTT connections are complete
********************************************************************************/
void onConnectionEstablished() {

  if (!initialConnectionEstablished_Flag) {     //execute this the first time we have established a WiFi and MQTT connection after powerup
    initialConnectionEstablished_Flag = true;

    //--------------------------------------------
    //  sync local time to NTP server
    configTime(TIME_ZONE * 3600, DST, "pool.ntp.org", "time.nist.gov");

    //--------------------------------------------
    //  display IP address
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());

    //--------------------------------------------
    //  update WiFi and MQTT connection status bar
    UpdateWiFiConnectionStatus();     //update WiFi status bar
    UpdateMQTTConnectionStatus();     //update MQTT status bar

    //--------------------------------------------
    //  query Ark Node to see if it is synced
    //  we need some error handling here!!!!!!  What do we do if there is no ark node connected?
    //  if (checkArkNodeStatus()) {

    //--------------------------------------------
    //  Retrieve Wallet Nonce and Balance
    getWallet();

    //--------------------------------------------
    //  Copy data stored in Flash into RAM
    bridgechainWallet.lastRXpage = loadEEPROM();                 //load page number from eeprom
    if (bridgechainWallet.lastRXpage < 1) {
      bridgechainWallet.lastRXpage = 0;
    }

    //--------------------------------------------
    //  Parse the wallet looking for the last received transaction
    bridgechainWallet.lastRXpage = getMostRecentReceivedTransaction(bridgechainWallet.lastRXpage + 1);  //lastRXpage is equal to the page number of the last received transaction in the wallet.

    //TODO.  Sometimes I find that the API reads will return with no data even if there are additional transactions to be read
    // I think this occurs when the WiFi connection or my network is a bit flakey.
    saveEEPROM(bridgechainWallet.lastRXpage);


    //--------------------------------------------
    //  query Ark Node to see if it is synced and update status bar
    UpdateArkNodeConnectionStatus();

    //--------------------------------------------
    scooterRental.rentalRate_Uint64 = RENTAL_RATE_UINT64;
    strcpy(scooterRental.rentalRate, RENTAL_RATE_STR);


    //wait for time to sync from NTP servers
    while (time(nullptr) <= 100000) {
      delay(50);
    }

    //--------------------------------------------
    //  Update clock on TFT display
    UpdateDisplayTime();

  }

  else {
    //--------------------------------------------
    //  update WiFi and MQTT connection status bar
    UpdateWiFiConnectionStatus();     //update WiFi status bar
    UpdateMQTTConnectionStatus();     //update MQTT status bar
  }

  //--------------------------------------------
  // Publish MQTT packet 1 seconds after connecting to broker 
  previousUpdateTime_MQTT_Publish = millis() - UpdateInterval_MQTT_Publish + 1000;  

}



/********************************************************************************
  Configures the TFT display and resistive touchscreen
  There is also LITE pin which is not connected to any pads but you can use to control the backlight. Pull low to turn off the backlight. You can connect it to a PWM output pin.
  There is also an IRQ pin which is not connected to any pads but you can use to detect when touch events have occured.
  There is also an Card Detect (CD) pin which is not connected to any pads but you can use to detect when a microSD card has been inserted have occured. It will be shorted to ground when a card is not inserted.
********************************************************************************/
void setupDisplayTouchscreen() {

  //To replace previously-drawn text when using a custom font, use the getTextBounds() function to determine the smallest rectangle encompassing a string,
  //erase the area using fillRect(), then draw new text.
  // https://learn.adafruit.com/adafruit-gfx-graphics-library/using-fonts

  //--------------------------------------------
  // setup 240x320 TFT display with custom font and clear screen
  // tft.setFont();    //configure standard adafruit font
  tft.begin();
  tft.fillScreen(BLACK);  //clear screen
  tft.setFont(&FreeSans9pt7b);    //9pt = 12pixel height(I think)  https://reeddesign.co.uk/test/points-pixels.html
  tft.setTextColor(WHITE);
  // we currently are not using the touchscreen so don't bother initializing it.
  /*
    //--------------------------------------------
    // setup touchscreencontroller.
    // NOTE:  When I push the reset button sometimes the controller does not start. I am not sure why. Perhaps there is a reset sequence on the control lines that should be implemented
    delay(300);
    if (!ts.begin()) {
      Serial.println("Couldn't start touchscreen controller");
      while (1);
    }
    Serial.println("Touchscreen started");
  */
}



/********************************************************************************
  Draw the status bar at the bottom of the screen
********************************************************************************/
void InitStatusBar() {
  tft.fillRect(0, 265 - 20, 240, 55 + 20, BLACK); //clear the status bar area + powered by ark.io text above it
  tft.setTextColor(ArkRed);
  tft.setFont(&FreeSans9pt7b);
  tft.setCursor(45, 260);
  tft.print("Powered by Ark.io");
  tft.setTextColor(WHITE);

  tft.setCursor(60 - 21, 283);
  tft.print("kmh");

  tft.setCursor(0, 301);
  tft.print("WiFi");
  tft.setCursor(70, 301);
  tft.print("MQTT");
  tft.setCursor(0, 319);
  tft.print("GPS");
  tft.setCursor(70, 319);
  tft.print("ARK");

  tft.setCursor(150, 283);
  tft.print("RSSI");
  tft.setCursor(150, 301);
  tft.print("BAT");
  tft.setCursor(150, 319);
  tft.print("SAT");

  tft.fillCircle(50, 301 - 6, 6, RED); //x,y,radius,color     //WiFi Status
  tft.fillCircle(130, 301 - 6, 6, RED); //x,y,radius,color    //MQTT Status
  tft.fillCircle(50, 319 - 6, 6, RED); //x,y,radius,color     //GPS Status
  tft.fillCircle(130, 319 - 6, 6, RED); //x,y,radius,color    //ARK Status
}
