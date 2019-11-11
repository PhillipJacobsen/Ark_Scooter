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


/********************************************************************************
  This routine retrieves the nonce for the last transaction on the wallet

     This is equivalant to calling http://37.34.60.90:4040/api/v2/wallets/TRXA2NUACckkYwWnS9JRkATQA453ukAcD1
     json-formatted object:
  {
  "data":{
  "address":"TKneFA9Rm6GrX9zVXhn6iGprnW2fEauouE",
  "publicKey":"039ae554142f4df0a22c5c25b182896e9b3a1c785c6a0b8d1581cade5936608452",
  "nonce":"2",
  "balance":"2099999480773504",
  "isDelegate":false,
  "isResigned":false
  }
  }

  virtual std::string get(const char *const identifier) = 0;
********************************************************************************/
void getWallet(uint32_t &nonce, uint32_t &balance) {
  //std::string walletGetResponse = connection.api.wallets.get(ArkAddress);
  const auto walletGetResponse = connection.api.wallets.get(ArkAddress);

  const size_t capacity = JSON_OBJECT_SIZE(1) + JSON_OBJECT_SIZE(6) + 200;
  DynamicJsonDocument doc(capacity);

  //const char* json = "{\"data\":{\"address\":\"TKneFA9Rm6GrX9zVXhn6iGprnW2fEauouE\",\"publicKey\":\"039ae554142f4df0a22c5c25b182896e9b3a1c785c6a0b8d1581cade5936608452\",\"nonce\":\"2\",\"balance\":\"2099999480773504\",\"isDelegate\":false,\"isResigned\":false}}";

  deserializeJson(doc, walletGetResponse.c_str());

  JsonObject data = doc["data"];
  //const char* data_address = data["address"]; // "TKneFA9Rm6GrX9zVXhn6iGprnW2fEauouE"
  //const char* data_publicKey = data["publicKey"]; // "039ae554142f4df0a22c5c25b182896e9b3a1c785c6a0b8d1581cade5936608452"
  const char* data_nonce = data["nonce"]; // "2"
  const char* data_balance = data["balance"]; // "2099999480773504"
  //bool data_isDelegate = data["isDelegate"]; // false
  //bool data_isResigned = data["isResigned"]; // false

  Serial.print("\n Get Wallet ");
  Serial.println(walletGetResponse.c_str()); // The response is a 'std::string', to Print on Arduino, we need the c_string type.
  Serial.print("Nonce: ");
  Serial.println(data_nonce);
  Serial.print("Balance: ");
  Serial.println(data_balance);
  nonce = atol(data_nonce);
  balance = atol(data_balance);
}



/********************************************************************************
  This routine retrieves 1 received transaction in wallet if available
  Returns '0' if no transaction exist
  returns parameters in
  id -> transaction ID
  amount -> amount of Arktoshi
  senderAddress -> transaction sender address
  vendorfield -> 255(or 256??? check this) Byte vendor field


********************************************************************************/
int GetReceivedTransaction(const char *const address, int page, const char* &id, const char* &amount, const char* &senderAddress, const char* &senderPublicKey, const char* &vendorField ) {

  //this is what we need to assemble:  https://radians.nl/api/v2/wallets/TRXA2NUACckkYwWnS9JRkATQA453ukAcD1/transactions/received?page=1&limit=1&orderBy=timestamp:asc
  //query = "?page=1&limit=1&orderBy=timestamp:asc"

  char query[50];
  strcpy(query, "?page=");
  char page_char[8];
  itoa(page, page_char, 10);    //convert int to string
  strcat(query, page_char);
  strcat(query, "&limit=1&orderBy=timestamp:asc");

//  Serial.print("query: ");
//  Serial.println(query);

  //--------------------------------------------
  //peform the API
  //sort by oldest transactions first.  For simplicity set limit = 1 so we only get 1 transaction returned
  timeAPIstart = millis();  //get time that API read started
  const auto walletGetResponse = connection.api.wallets.transactionsReceived(address, query);
  timeNow = millis() - timeAPIstart;  //get elapsed time
  Serial.print("Ark API read time: ");
  Serial.println(timeNow);


  //Serial.print("\nSearch Received Address: ");
  //Serial.println(address);
  //Serial.print("\nSearch page: ");
  //Serial.println(page );

  const size_t capacity = JSON_ARRAY_SIZE(1) + JSON_OBJECT_SIZE(2) + JSON_OBJECT_SIZE(3) + JSON_OBJECT_SIZE(9) + JSON_OBJECT_SIZE(14) + 1040;
  DynamicJsonDocument doc(capacity);

  //const char* json = "{\"meta\":{\"totalCountIsEstimate\":false,\"count\":1,\"pageCount\":1,\"totalCount\":1,\"next\":null,\"previous\":null,\"self\":\"/api/wallets/TRXA2NUACckkYwWnS9JRkATQA453ukAcD1/transactions/received?transform=true&page=1&limit=100\",\"first\":\"/api/wallets/TRXA2NUACckkYwWnS9JRkATQA453ukAcD1/transactions/received?transform=true&page=1&limit=100\",\"last\":\"/api/wallets/TRXA2NUACckkYwWnS9JRkATQA453ukAcD1/transactions/received?transform=true&page=1&limit=100\"},\"data\":[{\"id\":\"c45656ae40a6de17dea7694826f2bbb00d115130fbcaba257feaa820886acac3\",\"blockId\":\"4937253598533919154\",\"version\":2,\"type\":0,\"typeGroup\":1,\"amount\":\"100000000000\",\"fee\":\"9613248\",\"sender\":\"TEf7p5jf1LReywuits5orBsmpkMe8fLTkk\",\"senderPublicKey\":\"02b7cca8003dbce7394f87d3a7127f6fab5a8ebace83e5633baaae38c58f3eee7a\",\"recipient\":\"TRXA2NUACckkYwWnS9JRkATQA453ukAcD1\",\"signature\":\"57d78bc151d6b41d013e528966aee161c7fbc6f4d598774f33ac30f796c4b1ab7e2b2ce5f96612aebfe120a2956ce482515f99c73b3f52d7486a29ed8391295b\",\"confirmations\":125462,\"timestamp\":{\"epoch\":374000,\"unix\":1572368340,\"human\":\"2019-10-29T16:59:00.856Z\"},\"nonce\":\"2\"}]}";
  deserializeJson(doc, walletGetResponse.c_str());

  /*
    JsonObject meta = doc["meta"];
    bool meta_totalCountIsEstimate = meta["totalCountIsEstimate"]; // false
    int meta_count = meta["count"]; // 1
    int meta_pageCount = meta["pageCount"]; // 1
    int meta_totalCount = meta["totalCount"]; // 1
    const char* meta_self = meta["self"]; // "/api/wallets/TRXA2NUACckkYwWnS9JRkATQA453ukAcD1/transactions/received?transform=true&page=1&limit=100"
    const char* meta_first = meta["first"]; // "/api/wallets/TRXA2NUACckkYwWnS9JRkATQA453ukAcD1/transactions/received?transform=true&page=1&limit=100"
    const char* meta_last = meta["last"]; // "/api/wallets/TRXA2NUACckkYwWnS9JRkATQA453ukAcD1/transactions/received?transform=true&page=1&limit=100"
  */

  JsonObject data_0 = doc["data"][0];
  const char* data_0_id = data_0["id"]; // "c45656ae40a6de17dea7694826f2bbb00d115130fbcaba257feaa820886acac3"
  const char* data_0_blockId = data_0["blockId"]; // "4937253598533919154"
  int data_0_version = data_0["version"]; // 2
  int data_0_type = data_0["type"]; // 0
  int data_0_typeGroup = data_0["typeGroup"]; // 1
  const char* data_0_amount = data_0["amount"]; // "100000000000"
  const char* data_0_fee = data_0["fee"]; // "9613248"
  const char* data_0_sender = data_0["sender"]; // "TEf7p5jf1LReywuits5orBsmpkMe8fLTkk"
  const char* data_0_senderPublicKey = data_0["senderPublicKey"]; // "02b7cca8003dbce7394f87d3a7127f6fab5a8ebace83e5633baaae38c58f3eee7a"
  const char* data_0_recipient = data_0["recipient"]; // "TRXA2NUACckkYwWnS9JRkATQA453ukAcD1"
  const char* data_0_signature = data_0["signature"]; // "57d78bc151d6b41d013e528966aee161c7fbc6f4d598774f33ac30f796c4b1ab7e2b2ce5f96612aebfe120a2956ce482515f99c73b3f52d7486a29ed8391295b"
  const char* data_0_vendorField = data_0["vendorField"];
  long data_0_confirmations = data_0["confirmations"]; // 125462

  /*
    JsonObject data_0_timestamp = data_0["timestamp"];
    long data_0_timestamp_epoch = data_0_timestamp["epoch"]; // 374000
    long data_0_timestamp_unix = data_0_timestamp["unix"]; // 1572368340
    const char* data_0_timestamp_human = data_0_timestamp["human"]; // "2019-10-29T16:59:00.856Z"
  */
  const char* data_0_nonce = data_0["nonce"]; // "2"

  //--------------------------------------------
  //  Print the entire returned response string
  Serial.print("Get Wallet Received Transaction: ");
  Serial.println(walletGetResponse.c_str()); // The response is a 'std::string', to Print on Arduino, we need the c_string type.

  //--------------------------------------------
  //  The meta parameters that are returned are currently not reliable and are "estimates". Apparently this is due to lower performance nodes
  //  For this reason I will not use any of the meta parameters

  //--------------------------------------------
  //  the data_0_id parameter will be used to determine if a valid transaction was found.
  if (data_0_id == nullptr) {
    Serial.println("data_0_id is null");
    Serial.print(".");
    return 0;           //no transaction found
  }
  else {
    Serial.println("transaction was received");
    id = data_0_id;
    amount = data_0_amount;
    senderAddress = data_0_sender;
    senderPublicKey = data_0_senderPublicKey;
    vendorField = data_0_vendorField;
  }
  return 1;           //transaction found

}





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
  Serial.println("\n\nHere are all the transactions in a wallet");

  int CursorXtemp;
  int CursorYtemp;

  int page = 1;

  {

    while ( GetReceivedTransaction(ArkAddress, page, id, amount, senderAddress, senderPublicKey, vendorField ) ) {

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

      /*    tft.setCursor(CursorX, CursorY);
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

      */
      page++;

      //    timeAPIfinish = millis();  //get time that API read finished

    };
    //  tft.setCursor(CursorXtemp, CursorYtemp);
    //  tft.setTextColor(ILI9341_BLACK);
    //  tft.println(page - 1);

    Serial.print("No more Transactions ");
    Serial.print("\nThe most recent transaction was page #: ");
    Serial.println(page - 1);

    return page - 1;
  }

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
