/********************************************************************************
  This file contains various fuctions
********************************************************************************/


/********************************************************************************
  converts latitude from Adafruit degree-minute format to decimal-degrees
  https://gis.stackexchange.com/questions/8650/measuring-accuracy-of-latitude-and-longitude/8674#8674
********************************************************************************/
double convertDegMinToDecDeg_lat (float degMin) {
  double min = 0.0;
  double decDeg = 0.0;

  //get the minutes, fmod() requires double
  min = fmod((double)degMin, 100.0);

  //rebuild coordinates in decimal degrees
  degMin = (int) ( degMin / 100 );
  decDeg = degMin + ( min / 60 );

  if ( decDeg == 'S') {
    decDeg = (0 - decDeg);
  }
  return decDeg;
}

/********************************************************************************
  converts longitude from Adafruit degree-minute format to decimal-degrees
  https://gis.stackexchange.com/questions/8650/measuring-accuracy-of-latitude-and-longitude/8674#8674
********************************************************************************/
double convertDegMinToDecDeg_lon (float degMin) {
  double min = 0.0;
  double decDeg = 0.0;

  //get the minutes, fmod() requires double
  min = fmod((double)degMin, 100.0);

  //rebuild coordinates in decimal degrees
  degMin = (int) ( degMin / 100 );
  decDeg = degMin + ( min / 60 );

  if ( decDeg == 'W') {
    decDeg = (0 - decDeg);
  }
  return decDeg;
}


/********************************************************************************
  Fill in the data structure to be sent via MQTT
  const char* status;
  int battery;
  int fix;
  int satellites;
  float latitude;
  float longitude;
  float speedKPH;
  char walletBalance[65];
  char signature[140];

 ********************************************************************************/
void build_MQTTpacket() {
  NodeRedMQTTpacket.battery = batteryPercent;
  strcpy( NodeRedMQTTpacket.walletBalance, bridgechainWallet.walletBalance);

  NodeRedMQTTpacket.status = scooterRental.rentalStatus;

  NodeRedMQTTpacket.fix = int(GPS.fix);
  if (NodeRedMQTTpacket.fix) {                          //check to see if there is a GPS lock
    NodeRedMQTTpacket.satellites = GPS.satellites;      //number of satellites
    NodeRedMQTTpacket.speedKPH = GPS.speed * 1.852;     //convert knots to kph
    //we need to do some fomatting of the GPS signal so it is suitable for mapping software on Thingsboard
    NodeRedMQTTpacket.latitude = convertDegMinToDecDeg_lat(GPS.latitude);
    NodeRedMQTTpacket.longitude = convertDegMinToDecDeg_lon(GPS.longitude);
  }
  else {        //we do not have a GPS fix. What should the GPS location be?
    //  NodeRedMQTTpacket.status = "Broken";
    NodeRedMQTTpacket.latitude = 53.53583908;         //default location
    NodeRedMQTTpacket.longitude = -113.27674103;      //default location
    NodeRedMQTTpacket.satellites = 0;                 //number of satellites
    NodeRedMQTTpacket.speedKPH = 0;                   //speed
  }
}

/********************************************************************************
  send structure to NodeRed MQTT broker
********************************************************************************/
void send_MQTTpacket() {

  if (millis() - previousUpdateTime_MQTT_Publish > UpdateInterval_MQTT_Publish)  {
    previousUpdateTime_MQTT_Publish += UpdateInterval_MQTT_Publish;

    if (WiFiMQTTclient.isMqttConnected()) {
      build_MQTTpacket();

      //NOTE!  I think sprintf() is better to use here. update when you have a chance
      // example: {"status":"Rented","fix":1,"lat":53.53849358,"lon":-113.27589669,"speed":0.74,"sat":5,"bal":99990386752,"bat":96}
      String  buf;
      buf += F("{");
      buf += F("\"status\":");
      buf += F("\"");
      buf += String(NodeRedMQTTpacket.status);
      buf += F("\"");
      buf += F(",\"fix\":");
      buf += String(NodeRedMQTTpacket.fix);
      buf += F(",\"lat\":");
      buf += String(NodeRedMQTTpacket.latitude, 8);    //use 8 decimal point precision.
      buf += F(",\"lon\":");
      buf += String(NodeRedMQTTpacket.longitude, 8);
      buf += F(",\"speed\":");
      buf += String(NodeRedMQTTpacket.speedKPH);
      buf += F(",\"sat\":");
      buf += String(NodeRedMQTTpacket.satellites);
      buf += F(",\"bal\":");
      buf += String(NodeRedMQTTpacket.walletBalance);
      buf += F(",\"bat\":");
      buf += String(NodeRedMQTTpacket.battery);

      //These are pointers! They are not copying
      const char * msg = buf.substring(1).c_str();    //get string without leading {

      //alternate method
      //we need to sign the buffer without the leading and ending {}.
      // buf substring( 1,buf.length() )      //start index is inclusive. Ending index is exclusive.
      //const char * msg = buf.substring( 1, buf.length() ).c_str();
      //     const char * msg = buf.c_str();

      char msgbackup[700 + 1];
      strcpy(msgbackup, msg);

      //sign the packet using Private Key
      Message message;
      message.sign(msg, PASSPHRASE);
      const auto signatureString = BytesToHex(message.signature);

      buf += F(",\"sig\":");
      buf += F("\"");
      buf += signatureString.c_str();   //append the signature
      buf += F("\"");
      buf += F("}");


      Serial.println("\n=================================");
      Serial.println("Signing MQTT packet");
      printf("Signature from Signed Message: %s\n", signatureString.c_str());
      const bool isValid = message.verify();        //verify the signature
      printf("Message Signature is valid? %s\n", isValid ? "true" : "false");
      Serial.print("message that was signed: ");
      Serial.println(msgbackup);

      Serial.print("Sending MQTT packet: ");
      Serial.println(buf);
      WiFiMQTTclient.publish(MQTT_Base_Topic, buf.c_str());
    }
  }
}


/********************************************************************************
  update the clock on the status bar
  https://github.com/esp8266/Arduino/issues/4749
  http://www.cplusplus.com/reference/ctime/strftime/
********************************************************************************/
void UpdateDisplayTime() {
  time_t now = time(nullptr);   //get current time

  if (now > 1500000000) {       //this is a check to see if NTP time has been synced. (this is a time equal to approximatly the current time)
    struct tm * timeinfo;
    time(&now);
    timeinfo = localtime(&now);

    if ((timeinfo->tm_min) != prevDisplayMinute) { //update the display only if time has changed (updates every minute)
      prevDisplayMinute = timeinfo->tm_min;
      Serial.println("\n=================================");
      Serial.print("Unix epoch time: ");
      Serial.println(now);
      char formattedTime [30];
      strftime (formattedTime, 30, "%R", timeinfo);
      Serial.print("24hr time: ");
      Serial.println(formattedTime);

      tft.setFont(&FreeSans9pt7b);
      tft.setTextColor(WHITE);
      tft.fillRect(70 + 13, 283 - 17, 65, 18, BLACK); //clear the previous time
      tft.setCursor(70 + 13, 283);
      tft.print(formattedTime);      //dislay the current time
    }
  }
}

/********************************************************************************
  Update status bar with ARK node connection status.
********************************************************************************/
void UpdateArkNodeConnectionStatus() {
  if (checkArkNodeStatus()) {
    if (!ARK_status) {
      tft.fillCircle(130, 319 - 6, 6, GREEN); //x,y,radius,color    //ARK Status
      ARK_status = true;
    }
  }
  else {
    if (ARK_status) {
      tft.fillCircle(130, 319 - 6, 6, RED); //x,y,radius,color    //ARK Status
      ARK_status = false;
    }
  }
}

/********************************************************************************
  read the WiFi RSSI and update status bar
  I don't know if the value can actually be measured as actual dBm or just some other relative measurement
********************************************************************************/
void UpdateRSSIStatus() {

  if (millis() - previousUpdateTime_RSSI > UpdateInterval_RSSI)  {
    previousUpdateTime_RSSI += UpdateInterval_RSSI;
    if (WiFiMQTTclient.isWifiConnected()) {
      long rssi = WiFi.RSSI();
      tft.fillRect(195, 283 - 18, 40, 20, BLACK);   //clear the last voltage reading
      tft.setFont(&FreeSans9pt7b);
      tft.setTextColor(WHITE);
      tft.setCursor(195, 283);
      tft.print(rssi);
    }
  }
}


/********************************************************************************
  read the battery voltage and update status bar
  the ESP32 ADC should really be calibrated so these readings are only good for relative measurements.
********************************************************************************/
void UpdateBatteryStatus() {
  if (millis() - previousUpdateTime_Battery > UpdateInterval_Battery)  {
    previousUpdateTime_Battery += UpdateInterval_Battery;
    int battery = analogRead(BAT_PIN);
    batteryPercent = map(battery, 1945, 2348, 0, 100);
    batteryPercent = constrain(batteryPercent, 0, 100);
    // batteryFloat = battery / 620.6; // battery(12 bit reading) / 4096 * 3.3V * 2(there is a resistor divider)
    float batteryFloat = battery / 559.5; //we needed to add fudge factor to calibrate readings. There must not be a 50% voltage divider on the input.
    tft.setFont(&FreeSans9pt7b);
    tft.setTextColor(WHITE);
    tft.fillRect(190, 301 - 17, 40, 19, BLACK);   //clear the last voltage reading
    tft.setCursor(190, 301);
    tft.print(batteryFloat);
    tft.print("V");
  }
}


/********************************************************************************
  Update status bar with WifI connection status
  Display updates only when connection status change
********************************************************************************/
void UpdateWiFiConnectionStatus() {
  if (WiFiMQTTclient.isWifiConnected()) {
    if (!WiFi_status) {
      tft.fillCircle(50, 301 - 6, 6, GREEN); //x,y,radius,color     //WiFi Status
      WiFi_status = true;
    }
  }
  else {
    if (WiFi_status) {
      tft.fillCircle(50, 301 - 6, 6, RED); //x,y,radius,color     //WiFi Status
      WiFi_status = false;
    }
  }
}


/********************************************************************************
  Update status bar with MQTT connection status.
  Display only updates on connection status change
********************************************************************************/
void UpdateMQTTConnectionStatus() {
  if (WiFiMQTTclient.isMqttConnected()) {
    if (!MQTT_status) {
      tft.fillCircle(130, 301 - 6, 6, GREEN); //x,y,radius,color    //MQTT Status
      MQTT_status = true;
    }
  }
  else {
    if (MQTT_status) {
      tft.fillCircle(130, 301 - 6, 6, RED); //x,y,radius,color      //MQTT Status
      MQTT_status = false;
    }
  }
}

/********************************************************************************
  if GPS fix then update display with GPS Speed and GPS Sat
********************************************************************************/
void UpdateGPSDataStatus() {
  if (GPS.fix) {
    if (millis() - previousUpdateTime_GPS > UpdateInterval_GPS)  {
      previousUpdateTime_GPS += UpdateInterval_GPS;

      tft.setFont(&FreeSans9pt7b);
      tft.setTextColor(WHITE);
      tft.fillRect(190, 319 - 17, 40, 18, ILI9341_BLACK); //clear the last GPS Sat reading
      tft.setCursor(190, 319);
      tft.print(GPS.satellites);

      tft.fillRect(0, 283 - 17, 35, 18, ILI9341_BLACK);   //clear the last speed reading
      tft.setCursor(0, 283);
      float speedkmh = GPS.speed * 1.852;
      tft.print(speedkmh, 1);

    }
  }
}


/********************************************************************************
  This routine will update the GPS Network Connection Icon on the TFT display
  Display only updates on connection status change
  When losing fix: Clear # satellites display. Clear Speed display
  When obtaining fix: Show # satellites. Show Speed
    Green Circle = GPS Fix achieved
    Red Circle = GPS Fix not available
 ********************************************************************************/
void UpdateGPSConnectionStatus() {
  if (GPS.fix) {
    if (!GPS_status) {
      tft.fillCircle(50, 319 - 6, 6, GREEN); //x,y,radius,color     //GPS Status

      tft.fillRect(190, 319 - 17, 40, 18, BLACK);   //clear the last # of satellite reading
      tft.setFont(&FreeSans9pt7b);
      tft.setTextColor(WHITE);
      tft.setCursor(190, 319);
      tft.print(GPS.satellites);
      tft.fillRect(0, 283 - 17, 35, 18, BLACK);   //clear the last speed reading
      tft.setCursor(0, 283);
      float speedkmh = GPS.speed * 1.852;
      tft.print(speedkmh, 1);
      GPS_status = true;
    }
  }
  else {
    if (GPS_status) {
      tft.fillCircle(50, 319 - 6, 6, RED); //x,y,radius,color     //GPS Status
      tft.fillRect(190, 319 - 17, 40, 18, BLACK);  //clear the last # of satellite reading
      tft.setFont(&FreeSans9pt7b);
      tft.setTextColor(WHITE);
      tft.setCursor(190, 319);
      tft.print('0');
      tft.fillRect(0, 283 - 17, 35, 18, BLACK);   //clear the last speed reading
      tft.setCursor(0, 283);
      tft.print('0');
      GPS_status = false;
    }
  }
}


/********************************************************************************
  Display Ark Splash Screen
********************************************************************************/
void DisplayArkBitmap() {
  clearMainScreen();
  tft.drawBitmap(56, 100, ArkBitmap, 128, 128, ArkRed);   // Display Ark bitmap on middle portion of screen
}


void clearMainScreen() {
  tft.fillRect(0, 0, 240, 265 - 20, BLACK);   //clear the screen except for the status bar
}


/********************************************************************************
  Load data from nonvolatile memory into RAM.
  Note. ESP32 has FLASH memory(not EEPROM) however the standard high level Arduino EEPROM arduino functions work.
********************************************************************************/
void loadEEPROM() {
  EEPROM.begin(512);
  EEPROM.get(0, lastRXpage_eeprom);
  char ok[2 + 1];
  EEPROM.get(0 + sizeof(lastRXpage_eeprom), ok);
  EEPROM.end();
  if (String(ok) != String("OK")) {
    lastRXpage_eeprom = 0;
  }
  Serial.println("Recovered credentials from FLASH");
  Serial.println(ok);
  Serial.println(lastRXpage_eeprom);
}


/********************************************************************************
  Store data in nonvolatile memory.
  Note. ESP32 has FLASH memory(not EEPROM) however the standard high level Arduino EEPROM arduino functions work.
  EEPROM.write does not write to flash immediately, instead you must call EEPROM.commit() whenever you wish to save changes to flash. EEPROM.end() will also commit, and will release the RAM copy of EEPROM contents.
********************************************************************************/
void saveEEPROM() {
  EEPROM.begin(512);
  EEPROM.put(0, lastRXpage_eeprom);
  char ok[2 + 1] = "OK";
  EEPROM.put(0 + sizeof(lastRXpage_eeprom), ok);
  EEPROM.commit();
  EEPROM.end();
  Serial.println("Saved credentials to FLASH");
}
