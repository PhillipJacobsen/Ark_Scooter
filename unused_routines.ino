
//just some temp MQTT stuff that is not being used.  put in onConnectionEstablished
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







//MACaddress.toCharArray(MQTT_CLIENT_NAME_MAC_ADDRESS_TEST2, 6);
//start new metho9d
//  uint8_t baseMac[6];
//  // Get MAC address for WiFi station
// esp_read_mac(baseMac, ESP_MAC_WIFI_STA);
//  char baseMacChr[18] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
//  sprintf(baseMacChr, "%02X:%02X:%02X:%02X:%02X:%02X", baseMac[0], baseMac[1], baseMac[2], baseMac[3], baseMac[4], baseMac[5]);
  //return String(baseMacChr);

//end new method




    
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




//display received message to Serial
void test2Func (const String & payload) {
  Serial.print("Received MQTT message: ");
  Serial.println(payload);
}







//NOTES!!!!!!!!!!!!!!
//const char* is a pointer to memory that hopefully contains a null-terminated string.
//A char* points to the memory location of a sequence of multiple chars.
//char sz[] = {'t', 'e', 's', 't', 0};    //C-string
//const char *psz = "test";

//https://accu.org/index.php/journals/1445  char* x is the same as char *x
    
    
    
    //https://forum.arduino.cc/index.php?topic=334771.0
    //  std::string balanceCopy = std::string(balance);
    //  balanceCopied = balanceCopy.c_str(); 
    
    
    // nonceUINT = strtol(nonce);
    // balanceUINT = strtol(balance, NULL, 10);    //returns the maximum value of a 32-bit number. Actually, we need a 64 bit number here.
    // nonceUINT = atol(*nonce);
    // balanceUINT = atol(*balance);


////////////////////////////////////////////////////////////////////////////////
// Sign a Message using a 12-word Passphrase and Verify it.
//
// Given the text "Hello World",
// and the passphrase "this is a top secret passphrase",
// the computed 'Signature" is:
// - "304402200fb4adddd1f1d652b544ea6ab62828a0a65b712ed447e2538db0caebfa68929e02205ecb2e1c63b29879c2ecf1255db506d671c8b3fa6017f67cfd1bf07e6edd1cc8".
//
// ---
static const auto MessageText       = "Hello World";

void signMessage() {
  Message message;
  message.sign(MessageText, PASSPHRASE);

  const auto signatureString = BytesToHex(message.signature);
  printf("\n\nSignature from Signed Message: %s\n", signatureString.c_str());

  const bool isValid = message.verify();
  printf("\nMessage Signature is valid: %s\n\n", isValid ? "true" : "false");
}





//  http://www.fileformat.info/tool/hash.htm
/*  
  void encode_sha256() {

  //int esprandom = (random(16384, 16777216));    //generate random number with a lower and upper bound


  //char *payload = "Hello SHA 256!";
  char *payload = "9299610";

  byte shaResult[32];

  const size_t payloadLength = strlen(payload);       //holds length of payload

  mbedtls_md_init(&ctx);
  mbedtls_md_setup(&ctx, mbedtls_md_info_from_type(md_type), 0);
  mbedtls_md_starts(&ctx);
  mbedtls_md_update(&ctx, (const unsigned char *) payload, payloadLength);
  mbedtls_md_finish(&ctx, shaResult);
  mbedtls_md_free(&ctx);

  Serial.print("Hash: ");

  for (int i = 0; i < sizeof(shaResult); i++) {
    char str[3];

    sprintf(str, "%02x", (int)shaResult[i]);
    Serial.print(str);
  }
  }

*/





//I think a structure here for transaction details would be better form
//I need to do some work here to make things less hacky
//struct transactionDetails {
//   const char*  id;
//   int amount;
//   const char* senderAddress;
//   const char* vendorField;
//};

//--------------------------------------------
// these are used to store the received transation details returned from wallet search
// https://www.geeksforgeeks.org/difference-const-char-p-char-const-p-const-char-const-p/
//pointers to constant charaters. you cannot change the value but you can change the pointer
//const char* id;              //transaction ID
//const char* amount;           //transactions amount
//const char* senderAddress;    //transaction address of sender
//const char* senderPublicKey;  //transaction address of sender
//const char* vendorField;      //vendor field




/********************************************************************************
  This will routine send GPS coordinates to MQTT broker if Fix is achieved.
  GPS SAT # and Speed are also updated on the TFT display

 ********************************************************************************/
void GPStoMQTT() {
  if (WiFiMQTTclient.isWifiConnected()) {


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
    //    WiFiMQTTclient.publish("test/GPS2", msg);

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

    //     WiFiMQTTclient.publish("test/GPS", buf.c_str());
    WiFiMQTTclient.publish("scooter/TRXA2NUACckkYwWnS9JRkATQA453ukAcD1/data", buf.c_str());

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
