/********************************************************************************
  This file contains functions that interact with Ark client C++ API
  code here is a hack right now. Just learning the API and working on basic program flow and function
********************************************************************************/


/********************************************************************************
  This routine checks to see if Ark node is syncronized to the chain.
  This is a maybe a good way to see if node communication is working correctly.
  This might be a good routine to run periodically
  Returns True if node is synced

     The following method can be used to get the Status of a Node.
     This is equivalant to calling '167.114.29.49:4003/api/v2/node/status'
     json-formatted object:
  {
   "data":{
      "synced":true,
      "now":4047140,
      "blocksCount":-4047140,
      "timestamp":82303508
   }
  }

    https://arduinojson.org/v6/api/jsondocument/
    https://arduinojson.org/v6/assistant/
********************************************************************************/
bool checkArkNodeStatus() {
  const auto nodeStatus = connection.api.node.status();   //get status of Ark node
  Serial.print("\nNode Status: ");
  Serial.println(nodeStatus.c_str()); // The response is a 'std::string', to Print on Arduino, we need the c_string type.

  const size_t capacity = JSON_OBJECT_SIZE(1) + JSON_OBJECT_SIZE(4) + 50;
  DynamicJsonDocument doc(capacity);

  //  const char* json = "{\"data\":{\"synced\":true,\"now\":4047140,\"blocksCount\":-4047140,\"timestamp\":82303508}}";

  deserializeJson(doc, nodeStatus.c_str());

  JsonObject data = doc["data"];
  bool data_synced = data["synced"]; // true
  // long data_now = data["now"]; // 4047140
  // long data_blocksCount = data["blocksCount"]; // -4047140
  // long data_timestamp = data["timestamp"]; // 82303508
  return data_synced;
}

/*
  bool checkArkNodeStatus() {


  const auto nodeStatus = connection.api.node.status();

  Serial.print("\nNode Status: ");
  Serial.println(nodeStatus.c_str()); // The response is a 'std::string', to Print on Arduino, we need the c_string type.

  const size_t capacity = JSON_OBJECT_SIZE(1) + JSON_OBJECT_SIZE(3) + 50;
  DynamicJsonBuffer jsonBuffer(capacity);

  JsonObject& root = jsonBuffer.parseObject(nodeStatus.c_str());

  JsonObject& data = root["data"];
  bool data_synced = data["synced"]; // true
  //long data_now = data["now"]; // 1178395
  //int data_blocksCount = data["blocksCount"]; // 0

  return data_synced;

*/





/********************************************************************************
  This routine will search through all the received transactions of ArkAddress wallet starting from the oldest.
  "searching wallet + page#" will be displayed. text will toggle between red/white every received transaction
  The page number of the last transaction in the search will be displayed.
  This is the page to the most newest receive transaction on the chain.
  The final page number is also equal to the total number of received transactions in the wallet.

  The routine returns the page number of the most recent transaction.
  Empty wallet will return '0' (NOT YET TESTED)

********************************************************************************/
int getMostRecentReceivedTransaction() {
  Serial.print("\n\n\nHere are all the transactions in a wallet\n");

  int CursorXtemp;
  int CursorYtemp;

  int page = 1;
  while ( searchReceivedTransaction(ArkAddress, page, id, amount, senderAddress, vendorField) ) {

    //    timeNow = millis() - timeAPIfinish;  //get current time
    //    Serial.print("API read time:");
    //    Serial.println(timeNow);

    Serial.print("Page: ");
    Serial.println(page);
    //   Serial.print("Transaction id: ");
    //    Serial.println(id);
    //    Serial.print("Amount(Arktoshi): ");
    //    Serial.println(amount);
    //    Serial.print("Amount(Ark): ");
    //   Serial.println(float(amount) / 100000000, 8);
    //    Serial.print("Sender address: ");
    //   Serial.println(senderAddress);
    Serial.print("Vendor Field: ");
    Serial.println(vendorField);

    tft.setCursor(CursorX, CursorY);
    if ( (page & 0x01) == 0) {
      tft.setTextColor(ILI9341_WHITE);
      tft.print("searching wallet: ");
      CursorXtemp = tft.getCursorX();
      CursorYtemp = tft.getCursorY();
      tft.setTextColor(ILI9341_BLACK);
      tft.print(page - 1);
      tft.setCursor(CursorXtemp, CursorYtemp);
      tft.setTextColor(ILI9341_WHITE);
      tft.println(page);


    }
    else {
      tft.setTextColor(ILI9341_RED);
      tft.print("searching wallet: ");
      CursorXtemp = tft.getCursorX();
      CursorYtemp = tft.getCursorY();
      tft.setTextColor(ILI9341_BLACK);
      tft.print(page - 1);
      tft.setCursor(CursorXtemp, CursorYtemp);
      tft.setTextColor(ILI9341_RED);
      tft.println(page);
      //We need to clear the pixels around the page number every time we refresh.
    }
    page++;
    yield();

    //    timeAPIfinish = millis();  //get time that API read finished

  };
  tft.setCursor(CursorXtemp, CursorYtemp);
  tft.setTextColor(ILI9341_BLACK);
  tft.println(page - 1);

  Serial.print("No more Transactions ");
  Serial.print("\nThe most recent transaction was page #: ");
  Serial.println(page - 1);

  return page - 1;
}








//quick test routine.
void searchTransaction() {
  //const std::map<std::string, std::string>& body_parameters, int limit = 5,
  std::string vendorFieldHexString;
  vendorFieldHexString = "6964647955";
  //std::string transactionSearchResponse = connection.api.transactions.search( {{"vendorFieldHex", vendorFieldHexString}, {"orderBy", "timestamp:asc"} },1,1);
  std::string transactionSearchResponse = connection.api.transactions.search( {{"recipientId", ArkAddress}, {"orderBy", "timestamp:asc"} }, "?limit=1&page=1");

  Serial.print("\nSearch Result Transactions: ");
  Serial.println(transactionSearchResponse.c_str()); // The response is a 'std::string', to Print on Arduino, we need the c_string type.
}
