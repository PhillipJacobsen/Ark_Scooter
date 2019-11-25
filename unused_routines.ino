
/********************************************************************************
  This will routine send GPS coordinates to MQTT broker if Fix is achieved.
  GPS SAT # and Speed are also updated on the TFT display

 ********************************************************************************/
void GPStoMQTT() {
  if (client.isWifiConnected()) {


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
    //   if (GPS.fix) {
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

    //   Serial.print("Location Converted2: ");
    //   Serial.print(convertedLat, 8);
    //   Serial.print(", ");
    //   Serial.println(convertedLon, 8);

    //    Serial.print("Speed (knots): "); Serial.println(GPS.speed);
    float indicatedSpeed = GPS.speed * 1.852;
    //    Serial.print("Speed (km/h): "); Serial.println(indicatedSpeed);


    //      Serial.print("Angle: "); Serial.println(GPS.angle);

    //  Serial.print("Altitude: "); Serial.println(GPS.altitude);
    //  Serial.print("Satellites: "); Serial.println((int)GPS.satellites);

    //      msg = "{\"name\":\"where PJ should be fishing\",\"Fix\":false,\"lat\":53.53583908,\"lon\":-113.27674103,\"alt\":0,\"speed\":0,\"sat\":0}";
    //      Serial.print("Publish message: ");
    //     Serial.println(msg);
    //    client.publish("test/GPS2", msg);

    // https://arduino.stackexchange.com/questions/20911/how-to-append-float-value-of-into-a-string

    /*
       https://www.tutorialspoint.com/arduino/arduino_strings.htm
      char query[50];
      strcpy(query, "?page=");
      char page_char[8];
      itoa(page, page_char, 10);    //convert int to string
      strcat(query, page_char);
      strcat(query, "&limit=1&orderBy=timestamp:asc");

      char buf[256];
      strcpy(buf, "{\"name\":\"where PJ should be fishing\",\"Fix\":true,\"lat\":");
    */

    String  buf;
    buf += F("{\"name\":\"PJ should be fishing\",\"Fix\":true,\"lat\":");
    buf += String(convertedLat + 0.0032, 8);    //add noise to gps signal
    buf += F(",\"lon\":");
    buf += String(convertedLon + 0.00221, 8);
    //    buf += F(",\"alt\":0,\"speed\":0,\"sat\":0}");
    buf += F(",\"alt\":0,\"speed\":");
    buf += String(indicatedSpeed);
    buf += F(",\"sat\":");
    buf += String(GPS.satellites);
    buf += F(",\"bal\":");

    //String temp = buf +balance;
    //Serial.print("temp balance: ");
    //Serial.println(balanceCopied);
    Serial.print("Global temp balance: ");
    Serial.println(walletBalance);


    //buf += String("12323123123");
    buf += String(walletBalance);


    //buf += String(balanceUINT, 10);


    // buf += balance_temp.c_str();
    //buf += strstr(balance_STRING);
    //int bal = *balanceUINT;
    //buf += String(bal, 10);
    buf += F(",\"bat\":");
    buf += String(batteryPercent);
    buf += F("}");

    //char a = 47;
    //int b = (int) a;

    //    Serial.print("debug balance string ");
    //    Serial.println(balanceCopied);
    //    Serial.print("debug balance ");
    //   Serial.println(bal,10);

    Serial.print("Publish message: ");
    Serial.println(buf);

    //     client.publish("test/GPS", buf.c_str());
    client.publish("scooter/TRXA2NUACckkYwWnS9JRkATQA453ukAcD1/data", buf.c_str());

  }
}




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
