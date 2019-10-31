
/********************************************************************************
  update the clock on the status bar
********************************************************************************/
//https://github.com/esp8266/Arduino/issues/4749
void UpdateDisplayTime() {
  time_t now = time(nullptr);   //get current time

  if (now > 1500000000) {       //this is a check to see if NTP time has been synced. (this is a time equal to approximatly the current time)
    struct tm * timeinfo;
    time(&now);
    timeinfo = localtime(&now);

    if ((timeinfo->tm_min) != prevDisplayMinute) { //update the display only if time has changed (updates every minute)
      prevDisplayMinute = timeinfo->tm_min;
      //   if (now != prevDisplayTime) { //update the display only if time has changed (updates every second)
      //     prevDisplayTime = now;

      Serial.print("time is: ");
      Serial.println(now);
      //      Serial.println(ctime(&now));

      char formattedTime [30];
      strftime (formattedTime, 30, "%R", timeinfo); // http://www.cplusplus.com/reference/ctime/strftime/
      Serial.println(formattedTime);

      tft.setTextColor(WHITE);
      //      tft.fillRect(0, 60 - 18, 65, 20, BLACK); //clear the previous time
      //      tft.setCursor(0, 60);
      tft.fillRect(70 + 13, 283 - 17, 65, 18, BLACK); //clear the previous time
      tft.setCursor(70 + 13, 283);

      //tft.print(ctime(&now));      //dislay the current time
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
  if (client.isWifiConnected()) {
    long rssi = WiFi.RSSI();
    tft.fillRect(195, 283 - 18, 40, 20, BLACK);   //clear the last voltage reading
    tft.setCursor(195, 283);
    tft.print(rssi);

    Serial.print("RSSI:");
    Serial.println(rssi);
  }
}

/********************************************************************************
  read the battery voltage and update status bar
  the ESP32 ADC should really be calibrated so these readings are good for relative measurements.
********************************************************************************/
void UpdateBatteryStatus() {
  // batteryFloat = battery / 620.6; // battery(12 bit reading) / 4096 * 3.3V * 2(there is a resistor divider)
  //  batteryFloat = battery / 560; // battery(12 bit reading) / 4096 * 3.3V * 2(there is a resistor divider)  adjust with fudge factor
  //4.1 real v was reading 3.7V

  battery = analogRead(BAT_PIN);
  Serial.println(battery);
  batteryFloat = battery / 559.5; //we needed to add fudge factor to calibrate readings. There Must not be a 50% voltage divider on the input.
  //    battery = battery / 4096;   //battery(12 bit reading) / 4096 * 3.3V * 2(there is a resistor divider)
  //    battery = battery /620.60606060606;
  Serial.println(batteryFloat);

  tft.fillRect(190, 301 - 18, 40, 20, BLACK);   //clear the last voltage reading
  tft.setCursor(190, 301);
  tft.print(batteryFloat);
  tft.print("V");
}


/********************************************************************************
  Update status bar with WifI and MQTT connection status.
********************************************************************************/
void UpdateWiFiMQTTConnectionStatus() {
  if (client.isConnected()) {
    if (!WiFi_status) {
      tft.fillCircle(50, 301 - 6, 6, GREEN); //x,y,radius,color     //WiFi Status
      tft.fillCircle(130, 301 - 6, 6, GREEN); //x,y,radius,color    //MQTT Status
      WiFi_status = true;
    }
  }
  else {
    if (WiFi_status) {
      tft.fillCircle(50, 301 - 6, 6, RED); //x,y,radius,color     //WiFi Status
      tft.fillCircle(130, 301 - 6, 6, RED); //x,y,radius,color    //MQTT Status
      WiFi_status = false;
    }
  }
}



/********************************************************************************
  Update status bar with WifI connection status
  Display only updates on connection status change
********************************************************************************/
void UpdateWiFiConnectionStatus() {
  if (client.isWifiConnected()) {
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
  if (client.isMqttConnected()) {
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
      tft.setCursor(190, 319);
      tft.print(GPS.satellites);

      tft.fillRect(0, 283 - 17, 35, 18, BLACK);   //clear the last speed reading
      tft.setCursor(0, 283);
      float speedkmh = GPS.speed * 1.852;
      tft.print(speedkmh,1);

      GPS_status = true;
    }
  }
  else {
    if (GPS_status) {
      tft.fillCircle(50, 319 - 6, 6, RED); //x,y,radius,color     //GPS Status
      tft.fillRect(190, 319 - 17, 40, 18, BLACK);  //clear the last # of satellite reading
      tft.setCursor(190, 319);
      tft.print('0');

      tft.fillRect(0, 283 - 17, 35, 18, BLACK);   //clear the last speed reading
      tft.setCursor(0, 283);
      tft.print('0');

      GPS_status = false;
    }
  }
}



/*
   Return the quality (Received Signal Strength Indicator)
   of the WiFi network.
   Returns a number between 0 and 100 if WiFi is connected.
   Returns -1 if WiFi is disconnected.
  https://github.com/tttapa/Projects/blob/master/ESP8266/WiFi/RSSI-WiFi-Quality/RSSI-WiFi-Quality.ino
*/
int getQuality() {
  if (WiFi.status() != WL_CONNECTED)
    return -1;
  int dBm = WiFi.RSSI();
  if (dBm <= -100)
    return 0;
  if (dBm >= -50)
    return 100;
  return 2 * (dBm + 100);
}
