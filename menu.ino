void DisplayStatusPanel() {

  tft.setCursor(60-6, 283);
  tft.print("km/h");

  tft.setCursor(0, 301);
  tft.print("WiFI");
  tft.setCursor(70, 301);
  tft.print("MQTT");
  tft.setCursor(0, 319);
  tft.print("GPS");
  tft.setCursor(70, 319);
  tft.print("ARK");

  tft.setCursor(150, 301);
  tft.print("BAT");
  tft.setCursor(150, 319);
  tft.print("SAT");

  tft.fillCircle(50, 301 - 6, 6, RED); //x,y,radius,color     //WiFi Status
  tft.fillCircle(130, 301 - 6, 6, RED); //x,y,radius,color    //MQTT Status
  tft.fillCircle(50, 319 - 6, 6, RED); //x,y,radius,color     //GPS Status
  tft.fillCircle(130, 319 - 6, 6, RED); //x,y,radius,color    //ARK Status
}


void UpdateBatteryStatus() {
  // batteryFloat = battery / 620.6; // battery(12 bit reading) / 4096 * 3.3V * 2(there is a resistor divider)
  //  batteryFloat = battery / 560; // battery(12 bit reading) / 4096 * 3.3V * 2(there is a resistor divider)  adjust with fudge factor
  //4.1 real v was reading 3.7V

  battery = analogRead(35);
  Serial.println(battery);
  batteryFloat = battery / 559.5; //we needed to add fudge factor to calibrate readings. There Must not be a 50% voltage divider on the input.
  //    battery = battery / 4096;   //battery(12 bit reading) / 4096 * 3.3V * 2(there is a resistor divider)
  //    battery = battery /620.60606060606;
  Serial.println(batteryFloat);
  
  tft.fillRect(190, 301 - 20, 40, 22, BLACK);   //clear the last voltage reading
  tft.setCursor(190, 301);
  tft.print(batteryFloat);
  tft.print("V");
}

void UpdateWiFiConnectionStatus() {
  if (client.isConnected()) {
    if (!WiFI_status) {
      tft.fillCircle(50, 301 - 6, 6, GREEN); //x,y,radius,color     //WiFi Status
      tft.fillCircle(130, 301 - 6, 6, GREEN); //x,y,radius,color    //MQTT Status
      WiFI_status = true;
    }
  }
  else {
    if (WiFI_status) {
      tft.fillCircle(50, 301 - 6, 6, RED); //x,y,radius,color     //WiFi Status
      tft.fillCircle(130, 301 - 6, 6, RED); //x,y,radius,color    //MQTT Status
      WiFI_status = false;
    }
  }
}

/********************************************************************************
  This routine will update the GPS Network Connection Icon on the TFT display
  Screen updates only occur when there is a change in connection status
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

      tft.fillRect(0, 283 - 17, 50, 18, BLACK);   //clear the last speed reading
      tft.setCursor(0, 283);
      float speedkmh = GPS.speed*1.852;
      tft.print(speedkmh);

      GPS_status = true;
    }
  }
  else {
    if (GPS_status) {
      tft.fillCircle(50, 319 - 6, 6, RED); //x,y,radius,color     //GPS Status
      tft.fillRect(190, 319 - 17, 40, 18, BLACK);  //clear the last # of satellite reading
      tft.setCursor(190, 319);
      tft.print('0');

      tft.fillRect(0, 283 - 17, 50, 18, BLACK);   //clear the last speed reading
      tft.setCursor(0, 283);      
//      tft.print('0');
      
      GPS_status = false;
    }
  }
}
