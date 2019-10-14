// Color definitions
//#define ILI9341_BLACK       0x0000  ///<   0,   0,   0
//#define ILI9341_NAVY        0x000F  ///<   0,   0, 123
//#define ILI9341_GREEN   0x03E0  ///<   0, 125,   0
//#define ILI9341_DARKCYAN    0x03EF  ///<   0, 125, 123
//#define ILI9341_MAROON      0x7800  ///< 123,   0,   0
//#define ILI9341_PURPLE      0x780F  ///< 123,   0, 123
//#define ILI9341_OLIVE       0x7BE0  ///< 123, 125,   0
//#define ILI9341_LIGHTGREY   0xC618  ///< 198, 195, 198
//#define ILI9341_DARKGREY    0x7BEF  ///< 123, 125, 123
//#define ILI9341_BLUE        0x001F  ///<   0,   0, 255
//#define ILI9341_GREEN       0x07E0  ///<   0, 255,   0
//#define ILI9341_CYAN        0x07FF  ///<   0, 255, 255
//#define ILI9341_RED         0xF800  ///< 255,   0,   0
//#define ILI9341_MAGENTA     0xF81F  ///< 255,   0, 255
//#define ILI9341_YELLOW      0xFFE0  ///< 255, 255,   0
//#define ILI9341_WHITE       0xFFFF  ///< 255, 255, 255
//#define ILI9341_ORANGE      0xFD20  ///< 255, 165,   0
//#define ILI9341_GREENYELLOW 0xAFE5  ///< 173, 255,  41
//#define ILI9341_PINK        0xFC18  ///< 255, 130, 198


//screen is 240 x 320
//graphics primitives documentation  https://learn.adafruit.com/adafruit-gfx-graphics-library/graphics-primitives

//top left corner is (0,0)
//void drawRect(uint16_t x0, uint16_t y0, uint16_t w, uint16_t h, uint16_t color);
//void fillRect(uint16_t x0, uint16_t y0, uint16_t w, uint16_t h, uint16_t color);
//void drawRoundRect(uint16_t x0, uint16_t y0, uint16_t w, uint16_t h, uint16_t radius, uint16_t color);
//void fillRoundRect(uint16_t x0, uint16_t y0, uint16_t w, uint16_t h, uint16_t radius, uint16_t color);
//X, Y pair for the top-left corner of the rectangle, a width and height (in pixels), and a color


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


  tft.fillCircle(50, 301 - 6, 6, ILI9341_RED); //x,y,radius,color     //WiFi Status
  tft.fillCircle(130, 301 - 6, 6, ILI9341_RED); //x,y,radius,color    //MQTT Status
  tft.fillCircle(50, 319 - 6, 6, ILI9341_RED); //x,y,radius,color     //GPS Status
  tft.fillCircle(130, 319 - 6, 6, ILI9341_RED); //x,y,radius,color    //ARK Status
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
  
  tft.fillRect(190, 301 - 20, 40, 22, ILI9341_BLACK);   //clear the last voltage reading
  tft.setCursor(190, 301);
  tft.print(batteryFloat);
  tft.print("V");
}

void UpdateWiFiConnectionStatus() {
  if (client.isConnected()) {
    if (!WiFI_status) {
      tft.fillCircle(50, 301 - 6, 6, ILI9341_GREEN); //x,y,radius,color     //WiFi Status
      tft.fillCircle(130, 301 - 6, 6, ILI9341_GREEN); //x,y,radius,color    //MQTT Status
      WiFI_status = true;
    }
  }
  else {
    if (WiFI_status) {
      tft.fillCircle(50, 301 - 6, 6, ILI9341_RED); //x,y,radius,color     //WiFi Status
      tft.fillCircle(130, 301 - 6, 6, ILI9341_RED); //x,y,radius,color    //MQTT Status
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
      tft.fillCircle(50, 319 - 6, 6, ILI9341_GREEN); //x,y,radius,color     //GPS Status

      tft.fillRect(190, 319 - 17, 40, 18, ILI9341_BLACK);   //clear the last # of satellite reading
      tft.setCursor(190, 319);
      tft.print(GPS.satellites);

      tft.fillRect(0, 283 - 17, 50, 18, ILI9341_BLACK);   //clear the last speed reading
      tft.setCursor(0, 283);
      float speedkmh = GPS.speed*1.852;
      tft.print(speedkmh);

      GPS_status = true;
    }
  }
  else {
    if (GPS_status) {
      tft.fillCircle(50, 319 - 6, 6, ILI9341_RED); //x,y,radius,color     //GPS Status
      tft.fillRect(190, 319 - 17, 40, 18, ILI9341_BLACK);  //clear the last # of satellite reading
      tft.setCursor(190, 319);
      tft.print('0');

      tft.fillRect(0, 283 - 17, 50, 18, ILI9341_BLACK);   //clear the last speed reading
      tft.setCursor(0, 283);      
//      tft.print('0');
      
      GPS_status = false;
    }
  }
}
