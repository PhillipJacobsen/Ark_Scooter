/********************************************************************************
  This file contains functions used to configure hardware peripherals and various libraries.
********************************************************************************/

/********************************************************************************
  This routine configures the display and touchscreen
  There is also LITE pin which is not connected to any pads but you can use to control the backlight. Pull low to turn off the backlight. You can connect it to a PWM output pin.
  There is also an IRQ pin which is not connected to any pads but you can use to detect when touch events have occured.
  There is also an Card Detect (CD) pin which is not connected to any pads but you can use to detect when a microSD card has been inserted have occured. It will be shorted to ground when a card is not inserted.
********************************************************************************/
void setupDisplayTouchscreen() {

  //--------------------------------------------UpdateDisplayTime
  // setup 240x320 TFT display with custom font and clear screen
  // tft.setFont();    //configure standard adafruit font
  tft.begin();
  tft.fillScreen(BLACK);  //clear screen
  //  tft.setFont(&FreeSansBold9pt7b);
  tft.setFont(&FreeSans9pt7b);    //9pt = 12pixel height(I think)  https://reeddesign.co.uk/test/points-pixels.html


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



//https://github.com/esp8266/Arduino/issues/4749
void UpdateDisplayTime() {
  time_t now = time(nullptr);   //get current time

  if (now > 1500000000) {
    if (now != prevDisplayTime) { //update the display only if time has changed
      prevDisplayTime = now;

      Serial.print("time is: ");
      Serial.println(now);
      Serial.println(ctime(&now));

      tft.setTextColor(WHITE);
      tft.fillRect(0, 60 - 18, 240, 20, BLACK); //clear the last voltage reading
      tft.setCursor(0, 60);
      tft.print(ctime(&now));      //dislay the current time
    }
  }
}



//https://arduino.stackexchange.com/questions/42922/get-hour-with-ctime-time-library-with-esp8266
//use the above link for an example of how to change the below code.

/*
String getTimeStampString() {
  time_t rawtime = timeClient.getEpochTime();
  struct tm * ti;
  ti = localtime (&rawtime);

  uint16_t year = ti->tm_year + 1900;
  String yearStr = String(year);

  uint8_t month = ti->tm_mon + 1;
  String monthStr = month < 10 ? "0" + String(month) : String(month);

  uint8_t day = ti->tm_mday;
  String dayStr = day < 10 ? "0" + String(day) : String(day);

  uint8_t hours = ti->tm_hour;
  String hoursStr = hours < 10 ? "0" + String(hours) : String(hours);

  uint8_t minutes = ti->tm_min;
  String minuteStr = minutes < 10 ? "0" + String(minutes) : String(minutes);

  uint8_t seconds = ti->tm_sec;
  String secondStr = seconds < 10 ? "0" + String(seconds) : String(seconds);

  return yearStr + "-" + monthStr + "-" + dayStr + " " +
         hoursStr + ":" + minuteStr + ":" + secondStr;
}

*/

/********************************************************************************
  This routine configures the ESP32 internal clock to syncronize with NTP server.

  apparently this will enable the core to periodically sync the time with the NTP server. I don't really know how often this happens
  I am not sure if daylight savings mode works correctly. Previously it seems like this was not working on ESP2866
********************************************************************************/

/*
  void setupTime (){

  configTime(TIME_ZONE * 3600, DST, "pool.ntp.org");
  // printLocalTime();
  //  delay(100);

  //wait for time to sync from servers
  while (time(nullptr) <= 100000) {
    delay(100);
  }

  time_t now = time(nullptr);   //get current time
  Serial.print("time is: ");
  Serial.println(ctime(&now));

  //tft.setTextColor(WHITE);
  //  tft.setTextSize(1);
  //tft.print(ctime(&now));      //dislay the current time

  //  struct tm * timeinfo;
  //  time(&now);
  //  timeinfo = localtime(&now);
  //  Serial.println(timeinfo->tm_hour);
  }

*/

void ConfigureNeoPixels(RgbColor color) {
  strip.SetPixelColor(0, color);
  strip.SetPixelColor(1, color);
  strip.SetPixelColor(2, color);
  strip.SetPixelColor(3, color);
  strip.SetPixelColor(4, color);
  strip.SetPixelColor(5, color);
  strip.SetPixelColor(6, color);
  strip.SetPixelColor(7, color);
  strip.Show();
  Serial.print("writing new color to neopixels:");
}




//display received message to Serial
void test2Func (const String & payload) {
  Serial.println(payload);
}


// This function is called once everything is connected (Wifi and MQTT)
// WARNING : YOU MUST IMPLEMENT IT IF YOU USE EspMQTTClient
void onConnectionEstablished()
{
  //--------------------------------------------
  //  sync local time to NTP server
  // https://github.com/esp8266/Arduino/issues/4749  check this to see how to make this better
  configTime(TIME_ZONE * 3600, DST, "pool.ntp.org", "time.nist.gov");


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

  //  tft.fillScreen(BLACK);  //clear screen
  tft.fillRect(0, 0, 240, 42, BLACK);     //clear the first 2 lines of text

  tft.setCursor(0, 20);
  tft.print("IP address: ");
  tft.println(WiFi.localIP());
  tft.setCursor(0, 40);
  tft.println("Connected to MQTT broker");


  //wait for time to sync from servers
  while (time(nullptr) <= 100000) {
    delay(50);
  }
  //--------------------------------------------
  //  get time synced from NTP server
  UpdateDisplayTime();

}





/********************************************************************************
  Configure peripherals and system
********************************************************************************/
void setup()
{

  dacWrite(DAC1, 50);      //range 0>255, pin25 / A1
  //  dacWrite(DAC2, 255);      //range 0>255, pin26 / A0

  Serial.begin(115200);         // Initialize Serial Connection for debug / display
  while ( !Serial && millis() < 20 );

  pinMode(ledPin, OUTPUT);      // initialize digital ledPin as an output.
  digitalWrite(ledPin, LOW);    // initialize pin as off    //Adafruit HUZZAH32

  //--------------------------------------------
  // Optional Features of EspMQTTClient
  client.enableDebuggingMessages(); // Enable debugging messages sent to serial output
  client.enableHTTPWebUpdater(); // Enable the web updater. User and password default to values of MQTTUsername and MQTTPassword. These can be overwritten with enableHTTPWebUpdater("user", "password").
  client.enableLastWillMessage("TestClient/lastwill", "I am going offline");  // You can activate the retain flag by setting the third parameter to true

  //--------------------------------------------
  //configure the 2.4" TFT display and the touchscreen controller
  setupDisplayTouchscreen();

  // Bootup Screen
  // Display Ark bitmap on middle  portion of screen
  tft.drawBitmap(56, 120,  myBitmap, 128, 128, ArkRed);



  //--------------------------------------------
  // 9600 NMEA is the default baud rate for Adafruit MTK GPS's- some use 4800
  GPS.begin(9600);
  GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);   // turn on RMC (recommended minimum) and GGA (fix data) including altitude
  //GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCONLY);  //use this for just the minimum recommended data

  // For the parsing code to work nicely and have time to sort thru the data, and
  // print it out we don't suggest using anything higher than 1 Hz
  GPS.sendCommand(PMTK_SET_NMEA_UPDATE_1HZ); // 1 Hz update rate

  GPS.sendCommand(PGCMD_ANTENNA);  // Request updates on antenna status, comment out to keep quiet


  //
  delay(500);

  tft.setTextColor(WHITE);
  tft.setCursor(0, 20);
  tft.println("Connecting to WiFi");
  tft.setCursor(0, 40);
  tft.println(WIFI_SSID);

  // Delay for Bootup Screen to finish
  delay(1200);

  DisplayStatusPanel();
  UpdateBatteryStatus();



  //--------------------------------------------
  //  Configure NeoPixels.
  //  NOTE! If using the ESP8266 Make sure to call strip.Begin() after you call Serial.Begin because
  //    Din pin of NeoPixel is also connected to Serial RX pin(on ESP8266) and will configure the pin for usage by the DMA interface.
  //  strip.Begin();
  //  strip.ClearTo(RgbColor(0, 0, 0)); // Initialize all pixels to 'off'


  GPSSerial.println(PMTK_Q_RELEASE);// request firmware version

  //display QR code;
  QRcodeText = "ark:AUjnVRstxXV4qP3wgKvBgv1yiApvbmcHhx?amount=0.3";
  displayQRcode(QRcodeText);

}
