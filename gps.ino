
/********************************************************************************
  converts lat/long from Adafruit degree-minute format to decimal-degrees
  http://arduinodev.woofex.net/2013/02/06/adafruit_gps_forma/
********************************************************************************/
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
  Fill in the data structure to be sent via MQTT

  const char* status;
  int battery;
  int fix;
  int satellites;
  float latitude;
  float longitude;
  float speedKPH;
  char walletBalance[64];

 ********************************************************************************/
void build_MQTTpacket() {
  NodeRedMQTTpacket.battery = batteryPercent;
  strcpy( NodeRedMQTTpacket.walletBalance, walletBalance);

  NodeRedMQTTpacket.fix = int(GPS.fix);
  if (NodeRedMQTTpacket.fix) {
    NodeRedMQTTpacket.status = "Available";
    NodeRedMQTTpacket.satellites = GPS.satellites;              //number of satellites
    NodeRedMQTTpacket.speedKPH = GPS.speed * 1.852;  //convert knots to kph
    //we need to do some fomatting of the GPS signal so it is suitable for mapping software on Thingsboard
    NodeRedMQTTpacket.latitude = convertDegMinToDecDeg(GPS.latitude);
    if (( GPS.lat == 'S') | ( GPS.lat == 'W')) {
      NodeRedMQTTpacket.latitude = (0 - NodeRedMQTTpacket.latitude);
    }
    NodeRedMQTTpacket.longitude = convertDegMinToDecDeg(GPS.longitude);
    if (( GPS.lon == 'S') | ( GPS.lon == 'W')) {
      NodeRedMQTTpacket.longitude = (0 - NodeRedMQTTpacket.longitude);
    }
  }
  else {        //we do not have a GPS fix. What should the GPS location be?
    NodeRedMQTTpacket.status = "Broken";
    NodeRedMQTTpacket.latitude = 53.53583908;
    NodeRedMQTTpacket.longitude = -113.27674103;
  }
}

/********************************************************************************
  send structure to NodeRed MQTT broker
********************************************************************************/
void send_MQTTpacket() {

  if (millis() - previousUpdateTime_MQTT_Publish > UpdateInterval_MQTT_Publish)  {
    previousUpdateTime_MQTT_Publish += UpdateInterval_MQTT_Publish;

    if (client.isWifiConnected()) {

      build_MQTTpacket();

      // example: {"status":"Rented","fix":1,"lat":53.53849358,"lon":-113.27589669,"speed":0.74,"sat":5,"bal":99990386752,"bat":96}
      String  buf;
      buf += F("{");
      buf += F("\"status\":");
      buf += String(NodeRedMQTTpacket.status);
      buf += F(",\"fix\":");
      buf += String(NodeRedMQTTpacket.fix);
      buf += F(",\"lat\":");
      buf += String(NodeRedMQTTpacket.latitude + 0.0032, 8);    //add noise to gps signal
      buf += F(",\"lon\":");
      buf += String(NodeRedMQTTpacket.longitude + 0.00221, 8);
      buf += F(",\"speed\":");
      buf += String(NodeRedMQTTpacket.speedKPH);
      buf += F(",\"sat\":");
      buf += String(NodeRedMQTTpacket.satellites);
      buf += F(",\"bal\":");
      buf += String(NodeRedMQTTpacket.walletBalance);
      buf += F(",\"bat\":");
      buf += String(NodeRedMQTTpacket.battery);
      buf += F("}");

      Serial.print("send_MQTTpacket: ");
      Serial.println(buf);
      client.publish("scooter/TRXA2NUACckkYwWnS9JRkATQA453ukAcD1/data", buf.c_str());
    }
  }
}
