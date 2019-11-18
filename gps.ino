
float conv_coords(float in_coords)
{
  //Initialize the location.
  float f = in_coords;
  // Get the first two digits by turning f into an integer, then doing an integer divide by 100;
  // firsttowdigits should be 77 at this point.
  int firsttwodigits = ((int)f) / 100; //This assumes that f < 10000.
  float nexttwodigits = f - (float)(firsttwodigits * 100);
  float theFinalAnswer = (float)(firsttwodigits + nexttwodigits / 60.0);
  return theFinalAnswer;
}


// converts lat/long from Adafruit
// degree-minute format to decimal-degrees
double convertDegMinToDecDeg (float degMin) {
  double min = 0.0;
  double decDeg = 0.0;

  //get the minutes, fmod() requires double
  min = fmod((double)degMin, 100.0);

  //rebuild coordinates in decimal degrees
  degMin = (int) ( degMin / 100 );
  decDeg = degMin + ( min / 60 );

  return decDeg;
}


/********************************************************************************
  This will routine send GPS coordinates to MQTT broker if Fix is achieved.
  GPS SAT # and Speed are also updated on the TFT display

 ********************************************************************************/
void GPStoMQTT() {
  Serial.print("\nTime: ");
  if (GPS.hour < 10) {
    Serial.print('0');
  }
  Serial.print(GPS.hour, DEC); Serial.print(':');
  if (GPS.minute < 10) {
    Serial.print('0');
  }
  Serial.print(GPS.minute, DEC); Serial.print(':');
  if (GPS.seconds < 10) {
    Serial.print('0');
  }
  Serial.print(GPS.seconds, DEC); Serial.print('.');
  if (GPS.milliseconds < 10) {
    Serial.print("00");
  } else if (GPS.milliseconds > 9 && GPS.milliseconds < 100) {
    Serial.print("0");
  }
  Serial.println(GPS.milliseconds);
  Serial.print("Date: ");
  Serial.print(GPS.day, DEC); Serial.print('/');
  Serial.print(GPS.month, DEC); Serial.print("/20");
  Serial.println(GPS.year, DEC);
  Serial.print("Fix: "); Serial.print((int)GPS.fix);
  Serial.print(" quality: "); Serial.println((int)GPS.fixquality);
  if (GPS.fix) {
    Serial.print("Location: ");
    Serial.print(GPS.latitude, 4); Serial.print(GPS.lat);
    Serial.print(", ");
    Serial.print(GPS.longitude, 4); Serial.println(GPS.lon);

    //     Serial.print("Location Converted: ");
    //     Serial.print(conv_coords(GPS.latitude), 8); Serial.print(GPS.lat);
    //     Serial.print(", ");
    //     Serial.print(conv_coords(GPS.longitude), 8); Serial.println(GPS.lon);

    float convertedLat = convertDegMinToDecDeg(GPS.latitude);
    if (( GPS.lat == 'S') | ( GPS.lat == 'W')) {
      convertedLat = (0 - convertedLat);
    }
    float convertedLon = convertDegMinToDecDeg(GPS.longitude);
    if (( GPS.lon == 'S') | ( GPS.lon == 'W')) {
      convertedLon = (0 - convertedLon);
    }

    Serial.print("Location Converted2: ");
    Serial.print(convertedLat, 8);
    Serial.print(", ");
    Serial.println(convertedLon, 8);

    Serial.print("Speed (knots): "); Serial.println(GPS.speed);
    float indicatedSpeed = GPS.speed * 1.852;
    Serial.print("Speed (km/h): "); Serial.println(indicatedSpeed);


    //      Serial.print("Angle: "); Serial.println(GPS.angle);
    Serial.print("Altitude: "); Serial.println(GPS.altitude);
    Serial.print("Satellites: "); Serial.println((int)GPS.satellites);

    //      msg = "{\"name\":\"where PJ should be fishing\",\"Fix\":false,\"lat\":53.53583908,\"lon\":-113.27674103,\"alt\":0,\"speed\":0,\"sat\":0}";
    //      Serial.print("Publish message: ");
    //     Serial.println(msg);
    //    client.publish("test/GPS2", msg);

    // https://arduino.stackexchange.com/questions/20911/how-to-append-float-value-of-into-a-string

    //const char* buf;
    String  buf;
    buf += F("{\"name\":\"where PJ should be fishing\",\"Fix\":true,\"lat\":");
    buf += String(convertedLat+0.0032, 8);      //add noise to gps signal
    buf += F(",\"lon\":");
    buf += String(convertedLon+0.00221, 8);
    //    buf += F(",\"alt\":0,\"speed\":0,\"sat\":0}");
    buf += F(",\"alt\":0,\"speed\":");
    buf += String(indicatedSpeed);
    buf += F(",\"sat\":");
    buf += String(GPS.satellites);
     buf += F(",\"bat\":");

    // val = map(batteryFloat, 3.6, 4.23, 0, 100);
    buf += String(batteryPercent);   
    buf += F("}");

    Serial.println(buf);
    Serial.print("Publish message: ");
    //     client.publish("test/GPS", buf.c_str());
    client.publish("scooter/TRXA2NUACckkYwWnS9JRkATQA453ukAcD1/data", buf.c_str());


    tft.fillRect(190, 319 - 17, 40, 18, ILI9341_BLACK); //clear the last voltage reading
    tft.setCursor(190, 319);
    tft.print(GPS.satellites);

    tft.fillRect(0, 283 - 17, 35, 18, ILI9341_BLACK);   //clear the last speed reading
    tft.setCursor(0, 283);
    float speedkmh = GPS.speed * 1.852;
    tft.print(speedkmh, 1);
  }
}
