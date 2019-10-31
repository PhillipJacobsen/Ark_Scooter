/********************************************************************************
  This file contains functions used to configure hardware peripherals and various libraries.
********************************************************************************/

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
  //  tft.setFont(&FreeSansBold9pt7b);
  tft.setFont(&FreeSans9pt7b);    //9pt = 12pixel height(I think)  https://reeddesign.co.uk/test/points-pixels.html

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
  tft.setCursor(60 - 21, 283);
  tft.print("kmh");

  tft.setCursor(0, 301);
  tft.print("WiFI");
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
  Serial.println(payload);
}




/********************************************************************************
  This function is called once everything is connected (Wifi and MQTT)
********************************************************************************/
void onConnectionEstablished()
{
  //--------------------------------------------
  //  sync local time to NTP server
  // https://github.com/esp8266/Arduino/issues/4749  check this to see how to make this better
  configTime(TIME_ZONE * 3600, DST, "pool.ntp.org", "time.nist.gov");

  tft.fillRect(0, 0, 240, 42, BLACK);     //clear the first 2 lines of text
  tft.setCursor(0, 20);
  tft.print("IP address: ");
  tft.println(WiFi.localIP());
  //  tft.setCursor(0, 40);
  //  tft.println("Connected to MQTT broker");

  //--------------------------------------------
  //  update WiFi and MQTT connection status bar
  UpdateWiFiConnectionStatus();     //update WiFi status bar
  UpdateMQTTConnectionStatus();     //update MQTT status bar

  //--------------------------------------------
  //  query Ark Node to see if it is synced and update status bar
  UpdateArkNodeConnectionStatus();


  // Subscribe to "mytopic/test" and display received message to Serial
  client.subscribe("mytopic/test", [](const String & payload) {
    Serial.println(payload);
  });

  // Subscribe to "mytopic/test2"
  client.subscribe("mytopic/test2", test2Func);


  // Publish a message to "mytopic/test"
  client.publish("mytopic/test", "This is a message"); // You can activate the retain flag by setting the third parameter to true

  // Execute delayed instructions
  client.executeDelayed(5 * 1000, []() {
    client.publish("mytopic/test", "This is a message sent 5 seconds later");
  });



  //wait for time to sync from servers
  while (time(nullptr) <= 100000) {
    delay(50);
  }
  //--------------------------------------------
  //  get time synced from NTP server
  UpdateDisplayTime();


  const auto blockchainResponse = connection.api.blockchain.get();
  Serial.print("\nBlockchain Response: ");
  Serial.println(blockchainResponse.c_str());

  delay(200);
  tft.fillRect(0, 0, 240, 42, BLACK);     //clear the first 2 lines of text

  tft.setFont(&FreeSansBold18pt7b);
  //  tft.setFont(&FreeSansBoldOblique24pt7b);
  tft.setTextColor(ArkRed);
  tft.setCursor(12, 45);
  tft.print("Scan to Ride");


  tft.setFont(&FreeSans9pt7b);
  tft.setTextColor(WHITE);

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

  //--------------------------------------------
  // Optional Features of EspMQTTClient
  client.enableDebuggingMessages(); // Enable debugging messages sent to serial output
  client.enableHTTPWebUpdater(); // Enable the web updater. User and password default to values of MQTTUsername and MQTTPassword. These can be overwritten with enableHTTPWebUpdater("user", "password").
  client.enableLastWillMessage("TestClient/lastwill", "I am going offline");  // You can activate the retain flag by setting the third parameter to true

  //--------------------------------------------
  //configure the 2.4" TFT display and the touchscreen controller
  setupDisplayTouchscreen();

  // Bootup Screen
  // Display Ark bitmap on middle portion of screen
  tft.drawBitmap(56, 120,  myBitmap, 128, 128, ArkRed);

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
  tft.setCursor(0, 20);
  tft.println("Connecting to WiFi");
  tft.setCursor(0, 40);
  tft.println(WIFI_SSID);

  // show bootup screen for additional 1200ms
  delay(1200);

  InitStatusBar();        //display the status bar on the bottom of the screen
  UpdateBatteryStatus();



  GPSSerial.println(PMTK_Q_RELEASE);// request firmware version

  //display QR code;
  QRcodeText = "ark:AUjnVRstxXV4qP3wgKvBgv1yiApvbmcHhx?amount=0.3";
  displayQRcode(QRcodeText);

}
