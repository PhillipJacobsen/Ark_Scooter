/********************************************************************************
  This file contains functions used to configure hardware peripherals and various libraries.
********************************************************************************/

/********************************************************************************
  This function is called once everything is connected (Wifi and MQTT)
********************************************************************************/
void onConnectionEstablished() {

  if (!initialConnectionEstablished_Flag) {     //execute this the first time we have established a WiFi and MQTT connection after powerup
    initialConnectionEstablished_Flag = true;
    //--------------------------------------------
    //  sync local time to NTP server
    // https://github.com/esp8266/Arduino/issues/4749  check this to see how to make this better
    configTime(TIME_ZONE * 3600, DST, "pool.ntp.org", "time.nist.gov");

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
    loadEEPROM(); //load page number from eeprom
    if (lastRXpage_eeprom < 1) {
      lastRXpage_eeprom = 0;
    }

    lastRXpage = getMostRecentReceivedTransaction(lastRXpage_eeprom + 1);  //lastRXpage is equal to the page number of the last received transaction in the wallet.

    lastRXpage_eeprom = lastRXpage;
    saveEEPROM();


    //    Serial.print("try scanning wallet a second time ");
    //    lastRXpage = getMostRecentReceivedTransaction(lastRXpage+1);  //lastRXpage is equal to the page number of the last received transaction in the wallet.
    //    Serial.print("try scanning wallet a third time ");
    //    lastRXpage = getMostRecentReceivedTransaction(lastRXpage+1);  //lastRXpage is equal to the page number of the last received transaction in the wallet.
    //    Serial.print("try scanning wallet a forth time ");
    //    lastRXpage = getMostRecentReceivedTransaction(lastRXpage+1);  //lastRXpage is equal to the page number of the last received transaction in the wallet.

    //--------------------------------------------
    //  query Ark Node to see if it is synced and update status bar
    UpdateArkNodeConnectionStatus();

    scooterRental.rentalRate_Uint64 = RENTAL_RATE_UINT64;
    strcpy(scooterRental.rentalRate, RENTAL_RATE_STR);

    // Subscribe to "mytopic/test" and display received message to Serial
    //  WiFiMQTTclient.subscribe("scooter/TRXA2NUACckkYwWnS9JRkATQA453ukAcD1/test", [](const String & payload) {
    //    Serial.println(payload);
    //  });

    // Subscribe to "mytopic/test2"
    //  WiFiMQTTclient.subscribe("scooter/TRXA2NUACckkYwWnS9JRkATQA453ukAcD1/test2", test2Func);


    // Publish a message to "mytopic/test"
    //  WiFiMQTTclient.publish("scooter/TRXA2NUACckkYwWnS9JRkATQA453ukAcD1/test", "This is a message"); // You can activate the retain flag by setting the third parameter to true

    // Execute delayed instructions
    //  client.executeDelayed(5 * 1000, []() {
    //    WiFiMQTTclient.publish("scooter/TRXA2NUACckkYwWnS9JRkATQA453ukAcD1/test2", "This is a message sent 5 seconds later");
    //  });

    //wait for time to sync from servers
    while (time(nullptr) <= 100000) {
      delay(50);
    }
    //--------------------------------------------
    //  get time synced from NTP server
    UpdateDisplayTime();
  }

  else {
    //--------------------------------------------
    //  update WiFi and MQTT connection status bar
    UpdateWiFiConnectionStatus();     //update WiFi status bar
    UpdateMQTTConnectionStatus();     //update MQTT status bar

  }


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


void clearMainScreen() {
  tft.fillRect(0, 0, 240, 265 - 20, BLACK);   //clear the screen except for the status bar
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






//display received message to Serial
void test2Func (const String & payload) {
  Serial.print("Received MQTT message: ");
  Serial.println(payload);
}





/********************************************************************************
  Configure peripherals and system
********************************************************************************/
void setup()
{
  //--------------------------------------------
  // test the integrated DAC
  dacWrite(DAC1, 50);           //range 0>255, pin25 / A1
  //dacWrite(DAC2, 255);      //range 0>255, pin26 / A0

  Serial.begin(115200);         // Initialize Serial Connection for debug
  while ( !Serial && millis() < 20 );

  pinMode(LED_PIN, OUTPUT);      // initialize on board LED control pin as an output.
  digitalWrite(LED_PIN, LOW);    // Turn LED off

  // encode_sha256();

  //--------------------------------------------
  // Optional Features of EspMQTTClient
  WiFiMQTTclient.enableDebuggingMessages(); // Enable debugging messages sent to serial output
  // WiFiMQTTclient.enableHTTPWebUpdater(); // Enable the web updater. User and password default to values of MQTTUsername and MQTTPassword. These can be overwritten with enableHTTPWebUpdater("user", "password").
  WiFiMQTTclient.enableLastWillMessage("scooter/TRXA2NUACckkYwWnS9JRkATQA453ukAcD1/lastwill", "I am going offline");  // You can activate the retain flag by setting the third parameter to true

  //--------------------------------------------
  //configure the 2.4" TFT display and the touchscreen controller
  setupDisplayTouchscreen();

  // Bootup Screen
  // Display Ark bitmap on middle portion of screen
  DisplayArkBitmap();

  //--------------------------------------------
  // 9600 NMEA is the default baud rate for Adafruit MTK GPS
  GPS.begin(9600);
  GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);   // turn on RMC (recommended minimum) and GGA (fix data) including altitude
  // GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCONLY);  //use this for just the minimum recommended data

  // For the parsing code to work nicely and have time to sort thru the data, and
  // print it out we don't suggest using anything higher than 1 Hz
  GPS.sendCommand(PMTK_SET_NMEA_UPDATE_1HZ); // 1 Hz update rate
  GPS.sendCommand(PGCMD_ANTENNA);  // Request updates on antenna status, comment out to keep quiet

  //show bootup screen for 500ms
  delay(500);

  tft.setTextColor(WHITE);
  tft.setFont(&FreeSans9pt7b);
  tft.setCursor(50, 280);
  tft.println("Connecting to WiFi");
  tft.setCursor(70, 300);
  tft.println(WIFI_SSID);

  // show bootup screen for additional 1200ms
  delay(1200);

  rentalStatus = "Broken";


  InitStatusBar();        //display the status bar on the bottom of the screen
  UpdateBatteryStatus();

  GPSSerial.println(PMTK_Q_RELEASE);// request firmware version


  //  clearMainScreen();
  //display initial speed
  //  float speedkmh = 12.34543;      //current speed
  //  previousSpeed = speedkmh;       //store as previous speed in full precision
  //  tft.setFont(&Lato_Black_96);
  //  tft.setTextColor(SpeedGreenDarker);
  //  tft.setCursor(60, 105);
  //  tft.print(speedkmh, 0);         //display speed with 0 decimal point

  //  delay(2000);

  //  updateSpeedometer();
  //breakpoint
  //  while (1) {
  //  }

}










void displaySpeedScreen() {

  // https://learn.adafruit.com/adafruit-gfx-graphics-library/using-fonts
  // https://www.youtube.com/watch?v=L8MmTISmwZ8
  // http://oleddisplay.squix.ch/#/home     awesome tool for generating custom font.
  clearMainScreen();
  //speedometer
  tft.setFont(&Lato_Black_96);
  tft.setTextColor(SpeedGreenDarker);
  tft.setCursor(60, 105);
  tft.print("12");

  tft.setFont(&Lato_Medium_36);
  tft.setTextColor(SpeedGreen);     // http://www.barth-dev.de/online/rgb565-color-picker/
  tft.setCursor(75, 150);
  tft.print("km/h");

  //countdown timer
  tft.setFont(&Lato_Semibold_48);
  tft.setTextColor(OffWhite);
  tft.setCursor(55, 230);
  tft.print("10:32");

  //breakpoint
  //  while (1) {
  //  }
}










/********************************************************************************
  Copy data from nonvolatile memory into RAM.
  Note. ESP32 has FLASH memory(not EEPROM) however the standard high level Arduino EEPROM arduino functions work.
********************************************************************************/
// Load data from EEPROM
// You need to call EEPROM.begin(size) before you start reading or writing, size being the number of bytes you want to use.
// Size can be anywhere between 4 and 4096 bytes.

void loadEEPROM() {
  EEPROM.begin(512);
  EEPROM.get(0, lastRXpage_eeprom);
  // EEPROM.get(0 + sizeof(ssid), password);
  // EEPROM.get(0 + sizeof(ssid) + sizeof(password), coinname);
  char ok[2 + 1];
  //EEPROM.get(0+sizeof(ssid)+sizeof(password), ok);
  //EEPROM.get(0 + sizeof(ssid) + sizeof(password) + sizeof(coinname), ok);
  EEPROM.get(0 + sizeof(lastRXpage_eeprom), ok);
  EEPROM.end();
  if (String(ok) != String("OK")) {
    lastRXpage_eeprom = 0;
  }
  Serial.println("Recovered credentials from FLASH");
  Serial.println(ok);
  Serial.println(lastRXpage_eeprom);
  // Serial.println(strlen(ssid) > 0 ? ssid : "<no ssid>");
  // Serial.println(strlen(password) > 0 ? "********" : "<no password>");
  //Serial.println(strlen(coinname) > 0 ? coinname : "<no coinname>");
}




/********************************************************************************
  Store data in nonvolatile memory.
  Note. ESP32 has FLASH memory(not EEPROM) however the standard high level Arduino EEPROM arduino functions work.
  EEPROM.write does not write to flash immediately, instead you must call EEPROM.commit() whenever you wish to save changes to flash. EEPROM.end() will also commit, and will release the RAM copy of EEPROM contents.

********************************************************************************/
void saveEEPROM() {
  EEPROM.begin(512);
  EEPROM.put(0, lastRXpage_eeprom);
  // EEPROM.put(0 + sizeof(ssid), password);
  // EEPROM.put(0 + sizeof(ssid) + sizeof(password), coinname);
  char ok[2 + 1] = "OK";
  //EEPROM.put(0+sizeof(ssid)+sizeof(password), ok);
  //EEPROM.put(0 + sizeof(ssid) + sizeof(password) + sizeof(coinname), ok);
  EEPROM.put(0 + sizeof(lastRXpage_eeprom), ok);
  EEPROM.commit();
  EEPROM.end();
  Serial.println("Saved credentials to FLASH");
}
