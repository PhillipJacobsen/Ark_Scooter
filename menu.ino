
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

  if (millis() - previousUpdateTime_RSSI > UpdateInterval_RSSI)  {
    previousUpdateTime_RSSI += UpdateInterval_RSSI;
    if (client.isWifiConnected()) {
      long rssi = WiFi.RSSI();
      tft.fillRect(195, 283 - 18, 40, 20, BLACK);   //clear the last voltage reading
      tft.setCursor(195, 283);
      tft.print(rssi);

      Serial.print("RSSI:");
      Serial.println(rssi);
    }
  }
}

/********************************************************************************
  read the battery voltage and update status bar
  the ESP32 ADC should really be calibrated so these readings are good for relative measurements.
********************************************************************************/
void UpdateBatteryStatus() {
  if (millis() - previousUpdateTime_Battery > UpdateInterval_Battery)  {
    previousUpdateTime_Battery += UpdateInterval_Battery;
    // batteryFloat = battery / 620.6; // battery(12 bit reading) / 4096 * 3.3V * 2(there is a resistor divider)
    //  batteryFloat = battery / 560; // battery(12 bit reading) / 4096 * 3.3V * 2(there is a resistor divider)  adjust with fudge factor
    //4.1 real v was reading 3.7V

    //full power = approximate range: 1950 -> 2355
    //

    battery = analogRead(BAT_PIN);

    batteryPercent = map(battery, 1950, 2360, 0, 100);
    batteryPercent = constrain(batteryPercent, 0, 100);
    Serial.print("battery: ");
    Serial.println(battery);
    Serial.println(batteryPercent);
    batteryFloat = battery / 559.5; //we needed to add fudge factor to calibrate readings. There Must not be a 50% voltage divider on the input.
    //    battery = battery / 4096;   //battery(12 bit reading) / 4096 * 3.3V * 2(there is a resistor divider)
    //    battery = battery /620.60606060606;
    Serial.println(batteryFloat);

    tft.fillRect(190, 301 - 18, 40, 20, BLACK);   //clear the last voltage reading
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
  if GPS fix then update display with GPS Speed and GPS Sat
********************************************************************************/
void UpdateGPSDataStatus() {
  if (GPS.fix) {
    if (millis() - previousUpdateTime_GPS > UpdateInterval_GPS)  {
      previousUpdateTime_GPS += UpdateInterval_GPS;
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
      tft.setCursor(190, 319);
      tft.print('0');

      tft.fillRect(0, 283 - 17, 35, 18, BLACK);   //clear the last speed reading
      tft.setCursor(0, 283);
      tft.print('0');

      GPS_status = false;
    }
  }
}



void DisplayArkBitmap() {
  clearMainScreen();
  tft.drawBitmap(56, 100, ArkBitmap, 128, 128, ArkRed);   // Display Ark bitmap on middle portion of screen
}
