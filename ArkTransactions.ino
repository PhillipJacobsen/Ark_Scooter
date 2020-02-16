/********************************************************************************
  This file contains functions that interact with Ark client C++ API
********************************************************************************/

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



////////////////////////////////////////////////////////////////////////////////
// Sign a Message using a 12-word Passphrase and Verify it.
//
// Given the text "Hello World",
// and the passphrase "this is a top secret passphrase",
// the computed 'Signature" is:
// - "304402200fb4adddd1f1d652b544ea6ab62828a0a65b712ed447e2538db0caebfa68929e02205ecb2e1c63b29879c2ecf1255db506d671c8b3fa6017f67cfd1bf07e6edd1cc8".
//
// ---
void signMessage() {
  Message message;
  message.sign(MessageText, PASSPHRASE);

  const auto signatureString = BytesToHex(message.signature);
  printf("\n\nSignature from Signed Message: %s\n", signatureString.c_str());

  const bool isValid = message.verify();
  printf("\nMessage Signature is valid: %s\n\n", isValid ? "true" : "false");
}



////////////////////////////////////////////////////////////////////////////////
// Send a BridgeChain transaction, tailored for a custom network.
void sendBridgechainTransaction() {
  // Use the Transaction Builder to make a transaction.
  walletNonce_Uint64 = walletNonce_Uint64 + 1;

  char tempVendorField[80];
  strcpy(tempVendorField, "Ride End: ");
  strcat(tempVendorField, QRcodeHash);

  auto bridgechainTransaction = builder::Transfer()
                                .type(TYPE_0_TYPE)
                                .senderPublicKey(identities::Keys::fromPassphrase(PASSPHRASE).publicKey.data())
                                //.recipientId("TLdYHTKRSD3rG66zsytqpAgJDX75qbcvgT")        //genesis_2
                                .recipientId(scooterRental.senderAddress)        //genesis_2
                                .vendorField(tempVendorField)
                                .fee(TYPE_0_FEE)
                                .sign(PASSPHRASE)
                                .nonce(walletNonce_Uint64)
                                .amount(10000ULL)
                                .expiration(0UL)
                                //  .secondSign(SecondPassphrase)
                                .build(cfg);

  const auto transactionJson = bridgechainTransaction.toJson();
  printf("\n\nBridgechain Transaction: %s\n\n", transactionJson.c_str());

  bridgechainTransaction.sign(PASSPHRASE);

  char transactionsBuffer[600];
  snprintf(&transactionsBuffer[0], 600, "{\"transactions\":[%s]}", bridgechainTransaction.toJson().c_str());
  std::string jsonStr = transactionsBuffer;
  std::string sendResponse = connection.api.transactions.send(jsonStr);
  Serial.println(sendResponse.c_str());


}




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
  This routine retrieves the current nonce and the balance for the wallet

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

  function writes directly to global variables
  char walletBalance[64];
  uint64_t walletNonce_Uint64 = 1ULL;
  char walletNonce[64];
  uint64_t walletBalance_Uint64 = 0ULL;

********************************************************************************/
//void getWallet(const char* &nonce, const char* &balance) {
void getWallet() {
  //std::string walletGetResponse = connection.api.wallets.get(ArkAddress);
  const auto walletGetResponse = connection.api.wallets.get(ArkAddress);

  const size_t capacity = JSON_OBJECT_SIZE(1) + JSON_OBJECT_SIZE(6) + 200;
  DynamicJsonDocument doc(capacity);

  //const char* json = "{\"data\":{\"address\":\"TKneFA9Rm6GrX9zVXhn6iGprnW2fEauouE\",\"publicKey\":\"039ae554142f4df0a22c5c25b182896e9b3a1c785c6a0b8d1581cade5936608452\",\"nonce\":\"2\",\"balance\":\"2099999480773504\",\"isDelegate\":false,\"isResigned\":false}}";

  deserializeJson(doc, walletGetResponse.c_str());
  JsonObject data = doc["data"];
  strcpy(walletBalance, data["balance"]);      //copy into global character array
  walletBalance_Uint64 = strtoull(data["balance"], NULL, 10);   //string to unsigned long long

  strcpy(walletNonce, data["nonce"]);          //copy into global character array
  walletNonce_Uint64 = strtoull(data["nonce"], NULL, 10);   //string to unsigned long long

  Serial.print("\nGet Wallet ");
  Serial.println(walletGetResponse.c_str()); // The response is a 'std::string', to Print on Arduino, we need the c_string type.
  Serial.print("Nonce: ");
  Serial.println(walletNonce);
  Serial.printf("%" PRIu64 "\n", walletNonce_Uint64);   //PRIx64 to print in hexadecimal
  Serial.print("Balance: ");
  Serial.println(walletBalance);

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

  Serial.print("Page: ");
  Serial.println(page);

  char query[50];
  strcpy(query, "?page=");
  char page_char[8];
  itoa(page, page_char, 10);    //convert int to string
  strcat(query, page_char);
  strcat(query, "&limit=1&orderBy=timestamp:asc");

  //--------------------------------------------
  //peform the API
  //sort by oldest transactions first.  For simplicity set limit = 1 so we only get 1 transaction returned
  const auto walletGetResponse = connection.api.wallets.transactionsReceived(address, query);

  const size_t capacity = JSON_ARRAY_SIZE(1) + JSON_OBJECT_SIZE(2) + JSON_OBJECT_SIZE(3) + JSON_OBJECT_SIZE(9) + JSON_OBJECT_SIZE(14) + 1240 + 250; //add an extra 250 to be safe
  DynamicJsonDocument doc(capacity);
  //const char* json = "{\"meta\":{\"totalCountIsEstimate\":true,\"count\":1,\"pageCount\":2,\"totalCount\":2,\"next\":null,\"previous\":\"/api/wallets/TRXA2NUACckkYwWnS9JRkATQA453ukAcD1/transactions/received?page=1&limit=1&orderBy=timestamp%3Aasc&transform=true\",\"self\":\"/api/wallets/TRXA2NUACckkYwWnS9JRkATQA453ukAcD1/transactions/received?page=2&limit=1&orderBy=timestamp%3Aasc&transform=true\",\"first\":\"/api/wallets/TRXA2NUACckkYwWnS9JRkATQA453ukAcD1/transactions/received?page=1&limit=1&orderBy=timestamp%3Aasc&transform=true\",\"last\":\"/api/wallets/TRXA2NUACckkYwWnS9JRkATQA453ukAcD1/transactions/received?page=2&limit=1&orderBy=timestamp%3Aasc&transform=true\"},\"data\":[{\"id\":\"a59b33f8e708d14fb726a7f5bd2c3bb35c35b6389553e6be6869a6699cdc69d5\",\"blockId\":\"14320034153575802056\",\"version\":2,\"type\":0,\"typeGroup\":1,\"amount\":\"100000000\",\"fee\":\"9806624\",\"sender\":\"TEf7p5jf1LReywuits5orBsmpkMe8fLTkk\",\"senderPublicKey\":\"02b7cca8003dbce7394f87d3a7127f6fab5a8ebace83e5633baaae38c58f3eee7a\",\"recipient\":\"TRXA2NUACckkYwWnS9JRkATQA453ukAcD1\",\"signature\":\"36772f190c7c11134f6c00db0cb03d3ac5ac7e972abc7ddef076afe4a4362e29afd6e55ef3a0f0fa76466d2f4bdd1afbf488e836fd2f83a195e58561ce7c7244\",\"confirmations\":16883,\"timestamp\":{\"epoch\":2191712,\"unix\":1574186052,\"human\":\"2019-11-19T17:54:12.856Z\"},\"nonce\":\"3\"}]}";
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
  //  const char* data_0_blockId = data_0["blockId"]; // "4937253598533919154"
  //  int data_0_version = data_0["version"]; // 2
  //  int data_0_type = data_0["type"]; // 0
  //  int data_0_typeGroup = data_0["typeGroup"]; // 1
  const char* data_0_amount = data_0["amount"]; // "100000000000"
  //  const char* data_0_fee = data_0["fee"]; // "9613248"
  const char* data_0_sender = data_0["sender"]; // "TEf7p5jf1LReywuits5orBsmpkMe8fLTkk"
  const char* data_0_senderPublicKey = data_0["senderPublicKey"]; // "02b7cca8003dbce7394f87d3a7127f6fab5a8ebace83e5633baaae38c58f3eee7a"
  //  const char* data_0_recipient = data_0["recipient"]; // "TRXA2NUACckkYwWnS9JRkATQA453ukAcD1"
  //  const char* data_0_signature = data_0["signature"]; // "57d78bc151d6b41d013e528966aee161c7fbc6f4d598774f33ac30f796c4b1ab7e2b2ce5f96612aebfe120a2956ce482515f99c73b3f52d7486a29ed8391295b"
  const char* data_0_vendorField = data_0["vendorField"];
  //  long data_0_confirmations = data_0["confirmations"]; // 125462

  /*
    JsonObject data_0_timestamp = data_0["timestamp"];
    long data_0_timestamp_epoch = data_0_timestamp["epoch"]; // 374000
    long data_0_timestamp_unix = data_0_timestamp["unix"]; // 1572368340
    const char* data_0_timestamp_human = data_0_timestamp["human"]; // "2019-10-29T16:59:00.856Z"
    const char* data_0_nonce = data_0["nonce"]; // "2"
  */


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
    Serial.println("No Transaction. data_0_id is null");
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
  int page = 1;
  const char* id;               //transaction ID
  const char* amount;           //transactions amount
  const char* senderAddress;    //transaction address of sender
  const char* senderPublicKey;  //transaction address of sender
  const char* vendorField;      //vendor field

  while ( GetReceivedTransaction(ArkAddress, page, id, amount, senderAddress, senderPublicKey, vendorField ) ) {
    Serial.print("Page: ");
    Serial.println(page);
    Serial.print("Vendor Field: ");
    Serial.println(vendorField);
    page++;
  };

  Serial.print("No more Transactions ");
  Serial.print("\nThe most recent transaction was page #: ");
  Serial.println(page - 1);
  return page - 1;
}
