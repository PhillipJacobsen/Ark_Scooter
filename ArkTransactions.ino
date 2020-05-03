/********************************************************************************
  This file contains functions that interact with Ark client C++ API
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
  Serial.println("\n=================================");
  Serial.println("Check status of Radians Relay Node");

  const auto nodeStatus = connection.api.node.status();   //get status of Ark node
  Serial.print("\nNode Status: ");
  Serial.println(nodeStatus.c_str()); // The response is a 'std::string', to Print on Arduino, we need the c_string type.

  const size_t capacity = JSON_OBJECT_SIZE(1) + JSON_OBJECT_SIZE(4) + 50;
  DynamicJsonDocument doc(capacity);
  deserializeJson(doc, nodeStatus.c_str());
  JsonObject data = doc["data"];
  return data["synced"];
}


/********************************************************************************
  This routine retrieves the current nonce and the balance for the wallet

     This is equivalant to calling http://37.34.60.90:4040/api/v2/wallets/TRXA2NUACckkYwWnS9JRkATQA453ukAcD1

     json-formatted object:
  {
     "data":{
        "address":"TRXA2NUACckkYwWnS9JRkATQA453ukAcD1",
        "publicKey":"03e063f436ccfa3dfa9e9e6ee5e08a65a82a5ce2b2daf58a9be235753a971411e2",
        "nonce":"140",
        "balance":"94968174556",
        "isDelegate":false,
        "isResigned":false
     }
  }

  this function writes directly to these global variables:
  struct wallet {
  char walletBalance[64 + 1];
  uint64_t walletNonce_Uint64 = 1ULL;
  char walletNonce[64 + 1];
  uint64_t walletBalance_Uint64 = 0ULL;
  }

********************************************************************************/
void getWallet() {
  const auto walletGetResponse = connection.api.wallets.get(ArkAddress);

  const size_t capacity = JSON_OBJECT_SIZE(1) + JSON_OBJECT_SIZE(6) + 200;
  DynamicJsonDocument doc(capacity);

  deserializeJson(doc, walletGetResponse.c_str());
  JsonObject data = doc["data"];
  strcpy(bridgechainWallet.walletBalance, data["balance"]);                     //copy into global character array
  bridgechainWallet.walletBalance_Uint64 = strtoull(data["balance"], NULL, 10); //convert string to unsigned long long

  strcpy(bridgechainWallet.walletNonce, data["nonce"]);                         //copy into global character array
  bridgechainWallet.walletNonce_Uint64 = strtoull(data["nonce"], NULL, 10);     //convert string to unsigned long long

  Serial.println("\n=================================");
  Serial.println("Retrieving wallet Nonce & Balance");
  Serial.print("Get Wallet Response");
  Serial.println(walletGetResponse.c_str());                                    // The response is a 'std::string', to Print on Arduino, we need the c_string type.
  Serial.print("Nonce: ");
  //Serial.println(bridgechainWallet.walletNonce);                              // serial.print does not have support for Uint64 
  Serial.printf("%" PRIu64 "\n", bridgechainWallet.walletNonce_Uint64);         // PRIx64 to print in hexadecimal
  Serial.print("Balance: ");
  Serial.println(bridgechainWallet.walletBalance);
}



/********************************************************************************
  This routine retrieves 1 received transaction in wallet if available
  Returns '0' if no transaction exist
  returns parameters in
  id -> transaction ID
  amount -> amount of Arktoshi
  senderAddress -> transaction sender address
  vendorfield -> 255(or 256??? check this) Byte vendor field

  This is equivalant to calling:
    https://radians.nl/api/v2/wallets/TRXA2NUACckkYwWnS9JRkATQA453ukAcD1/transactions/received?page=1&limit=1&orderBy=timestamp:asc
    query = "?page=1&limit=1&orderBy=timestamp:asc"

  json-formatted object:
  {
     "meta":{
        "totalCountIsEstimate":true,
        "count":1,
        "pageCount":133,
        "totalCount":133,
        "next":"/api/wallets/TRXA2NUACckkYwWnS9JRkATQA453ukAcD1/transactions/received?page=2&limit=1&orderBy=timestamp%3Aasc&transform=true",
        "previous":null,
        "self":"/api/wallets/TRXA2NUACckkYwWnS9JRkATQA453ukAcD1/transactions/received?page=1&limit=1&orderBy=timestamp%3Aasc&transform=true",
        "first":"/api/wallets/TRXA2NUACckkYwWnS9JRkATQA453ukAcD1/transactions/received?page=1&limit=1&orderBy=timestamp%3Aasc&transform=true",
        "last":"/api/wallets/TRXA2NUACckkYwWnS9JRkATQA453ukAcD1/transactions/received?page=133&limit=1&orderBy=timestamp%3Aasc&transform=true"
     },
     "data":[
        {
           "id":"c45656ae40a6de17dea7694826f2bbb00d115130fbcaba257feaa820886acac3",
           "blockId":"4937253598533919154",
           "version":2,
           "type":0,
           "typeGroup":1,
           "amount":"100000000000",
           "fee":"9613248",
           "sender":"TEf7p5jf1LReywuits5orBsmpkMe8fLTkk",
           "senderPublicKey":"02b7cca8003dbce7394f87d3a7127f6fab5a8ebace83e5633baaae38c58f3eee7a",
           "recipient":"TRXA2NUACckkYwWnS9JRkATQA453ukAcD1",
           "signature":"57d78bc151d6b41d013e528966aee161c7fbc6f4d598774f33ac30f796c4b1ab7e2b2ce5f96612aebfe120a2956ce482515f99c73b3f52d7486a29ed8391295b",
           "confirmations":1366750,
           "timestamp":{
              "epoch":374000,
              "unix":1572368340,
              "human":"2019-10-29T16:59:00.856Z"
           },
           "nonce":"2"
        }
     ]
  }
********************************************************************************/
int GetReceivedTransaction(const char *const address, int page, const char* &id, const char* &amount, const char* &senderAddress, const char* &senderPublicKey, const char* &vendorField ) {

  //--------------------------------------------
  // assemble query string where the page number is a function parameter
  // query = "?page=1&limit=1&orderBy=timestamp:asc"
  char query[50];
  strcpy(query, "?page=");
  char page_char[8];
  itoa(page, page_char, 10);    //convert int to string in base 10
  strcat(query, page_char);
  strcat(query, "&limit=1&orderBy=timestamp:asc");

  //--------------------------------------------
  //peform the API call
  //sort by oldest transactions first.  For simplicity set limit = 1 so we only get 1 transaction returned
  const auto walletGetResponse = connection.api.wallets.transactionsReceived(address, query);

  const size_t capacity = JSON_ARRAY_SIZE(1) + JSON_OBJECT_SIZE(2) + JSON_OBJECT_SIZE(3) + JSON_OBJECT_SIZE(9) + JSON_OBJECT_SIZE(14) + 1240 + 250; //add an extra 250 to be safe
  DynamicJsonDocument doc(capacity);
  deserializeJson(doc, walletGetResponse.c_str());

  JsonObject data_0 = doc["data"][0];
  id = data_0["id"];
  amount = data_0["amount"];
  senderAddress = data_0["sender"];
  senderPublicKey = data_0["senderPublicKey"];
  vendorField = data_0["vendorField"];

  //--------------------------------------------
  //  Print the received transaction response
  Serial.print("\nGet Received Transaction Response");
  Serial.println(walletGetResponse.c_str());                    // The response is a 'std::string', to Print on Arduino, we need the c_string type.

  Serial.print("Page: ");
  Serial.println(page);

  //--------------------------------------------
  //  the data_0_id parameter will be used to determine if a valid transaction was found.
  if (id == nullptr) {
    Serial.println("No Transaction found. data_0_id is null");
    return 0;           //no transaction found
  }
  else {
    Serial.println("Transaction was received");
    Serial.print("Transactions ID: ");
    Serial.println(id);

    return 1;           //transaction found
  }
}



/********************************************************************************
  This routine will search through all the received transactions of ArkAddress wallet starting from the oldest.
  The routine returns the page number of the most recent transaction.
  The final page number is also equal to the total number of received transactions in the wallet.
  Empty wallet will return '0' (NOT YET TESTED)

********************************************************************************/
int getMostRecentReceivedTransaction(int page = 1) {
  Serial.println("\n=================================");
  Serial.println("Scanning all the received transactions in the wallet looking for the the newest one...");
  const char* id;               //transaction ID
  const char* amount;           //transactions amount
  const char* senderAddress;    //transaction address of sender
  const char* senderPublicKey;  //transaction address of sender
  const char* vendorField;      //vendor field

  while ( GetReceivedTransaction(ArkAddress, page, id, amount, senderAddress, senderPublicKey, vendorField ) ) {
    page++;
  };

  Serial.print("No more Transactions ");
  Serial.print("\nThe most recent transaction was page #: ");
  Serial.println(page - 1);
  return page - 1;
}


/********************************************************************************
  This routine will poll the Ark node API searching for the RentalStart custom transaction
  It polls once every 8 seconds (defined by UpdateInterval_RentalStartSearch)
  When polling the API we set the limit = 1 so we only retrieve 1 transaction at a time.
********************************************************************************/
int search_RentalStartTx() {
  if (millis() - previousUpdateTime_RentalStartSearch > UpdateInterval_RentalStartSearch)  {    //poll Ark node every 8 seconds for a new transaction
    previousUpdateTime_RentalStartSearch += UpdateInterval_RentalStartSearch;

    Serial.println("\n=================================");
    Serial.println("Polling Radians network to see if Rental Start transaction has been received. ");

    //  check to see if new new transaction has been received in wallet
    // lastRXpage is the page# of the last received transaction
    int searchRXpage = bridgechainWallet.lastRXpage + 1;
    const char* id;               //transaction ID
    const char* amount;           //transactions amount
    const char* senderAddress;    //transaction address of sender
    const char* senderPublicKey;  //transaction address of sender
    const char* vendorField;      //vendor field

    const char* asset_gps_latitude;
    const char* asset_gps_longitude;
    const char* asset_sessionId;
    const char* asset_rate;

    if ( GetTransaction_RentalStart(ArkAddress, searchRXpage, id, amount, senderAddress, senderPublicKey, vendorField, asset_gps_latitude, asset_gps_longitude, asset_sessionId, asset_rate) ) {
      Serial.println("\n=================================");
      Serial.println("Rental Start transaction was received");

      bridgechainWallet.lastRXpage++;             //increment received counter if rental start was received.
      saveEEPROM(bridgechainWallet.lastRXpage);   //store the page in the Flash
      
      Serial.print("Received SessionID: ");
      Serial.println(asset_sessionId);
      Serial.print("QR code SessionID: ");
      Serial.println(scooterRental.sessionID_QRcode);

      //check to see if sessionID of new transaction matches the Hash embedded in QRcode that was displayed
      if  (strcmp(asset_sessionId, scooterRental.sessionID_QRcode) == 0) {
        strcpy(scooterRental.senderAddress, senderAddress);           //copy into global character array
        strcpy(scooterRental.payment, amount);                        //copy into global character array
        scooterRental.payment_Uint64 = strtoull(amount, NULL, 10);    //convert string to unsigned long long global
        strcpy(scooterRental.sessionID_RentalStart, asset_sessionId);             //copy into global character array

        Serial.println("Received SessionID matched QR code SessionID");
        return 1;
      }

      else {        //we received a transaction that did not match. We should issue refund.
        Serial.print("SessionID did not match hash embedded in QRcode");
        // issueRefund();  TODO!!!!!!!!!!!!!!!!!!!!!!!
        return 0;
      }
    }
    else {    //we did not receive a transaction
      return 0;
    }
  }
  else {      //it was not time to poll Ark network for a new transaction
    return 0;
  }
}


/********************************************************************************
  This routine retrieves 1 RentalStart transaction if available in wallet
  Returns '0' if no transaction exists or if other transaction type exists.
  Pass wallet address and page to function

  //this is equivalent to called:  https://radians.nl/api/v2/wallets/TRXA2NUACckkYwWnS9JRkATQA453ukAcD1/transactions/received?page=41&limit=1&orderBy=timestamp:asc
  //query = "?page=1&limit=1&orderBy=timestamp:asc"

  json-formatted object:
  {
   "meta":{
      "totalCountIsEstimate":true,
      "count":1,
      "pageCount":133,
      "totalCount":133,
      "next":"/api/wallets/TRXA2NUACckkYwWnS9JRkATQA453ukAcD1/transactions/received?page=42&limit=1&orderBy=timestamp%3Aasc&transform=true",
      "previous":"/api/wallets/TRXA2NUACckkYwWnS9JRkATQA453ukAcD1/transactions/received?page=40&limit=1&orderBy=timestamp%3Aasc&transform=true",
      "self":"/api/wallets/TRXA2NUACckkYwWnS9JRkATQA453ukAcD1/transactions/received?page=41&limit=1&orderBy=timestamp%3Aasc&transform=true",
      "first":"/api/wallets/TRXA2NUACckkYwWnS9JRkATQA453ukAcD1/transactions/received?page=1&limit=1&orderBy=timestamp%3Aasc&transform=true",
      "last":"/api/wallets/TRXA2NUACckkYwWnS9JRkATQA453ukAcD1/transactions/received?page=133&limit=1&orderBy=timestamp%3Aasc&transform=true"
   },
   "data":[
      {
         "id":"2238687a95688eb434953ac6548ade4648a3963a8158c036b65ee8e434e17230",
         "blockId":"10300106383332556177",
         "version":2,
         "type":500,
         "typeGroup":4000,
         "amount":"1",
         "fee":"10000000",
         "sender":"TLdYHTKRSD3rG66zsytqpAgJDX75qbcvgT",
         "senderPublicKey":"02cbe4667ab08693cbb3c248b96635f84b5412a99b49237f059a724f2cfe2b733f",
         "recipient":"TRXA2NUACckkYwWnS9JRkATQA453ukAcD1",
         "signature":"cc5b22000e267dad4ac52a319120fe3dd022ba6fcb102f635ffe66fc2ec1f6ae6b2491c53eb88352aadddab68d18dc6ce6ef4cba12bd84e53be1c28364350566",
         "asset":{
            "gps":{
               "timestamp":1583125216,
               "latitude":"1.111111",
               "longitude":"-180.222222",
               "human":"2020-03-02T05:00:16.000Z"
            },
            "sessionId":"2cf24dba5fb0a30e26e83b2ac5b9e29e1b161e5c1fa7425e73043362938b9824",
            "rate":"5",
            "gpsCount":1
         },
         "confirmations":658462,
         "timestamp":{
            "epoch":11130880,
            "unix":1583125220,
            "human":"2020-03-02T05:00:20.856Z"
         },
         "nonce":"40"
      }
   ]
  }

  returns parameters:
  id -> transaction ID
  amount -> amount of Arktoshi
  senderAddress -> transaction sender address
  senderPublicKey -> transaction sender public key
  vendorField -> 255 Byte vendor field
  asset_gps_latitude
  asset_gps_longitude
  asset_sessionId
  asset_rate

********************************************************************************/
int GetTransaction_RentalStart(const char *const address, int page, const char* &id, const char* &amount, const char* &senderAddress, const char* &senderPublicKey, const char* &vendorField, const char* &asset_gps_latitude, const char* &asset_gps_longitude, const char* &asset_sessionId, const char* &asset_rate ) {

  //--------------------------------------------
  // assemble query string where the page number is a function parameter
  // query = "?page=1&limit=1&orderBy=timestamp:asc"
  char query[50];
  strcpy(query, "?page=");
  char page_char[8];
  itoa(page, page_char, 10);    //convert int to string in base 10
  strcat(query, page_char);
  strcat(query, "&limit=1&orderBy=timestamp:asc");

  //--------------------------------------------
  //peform the API call
  //sort by oldest transactions first.  For simplicity set limit = 1 so we only get 1 transaction returned
  const auto walletGetResponse = connection.api.wallets.transactionsReceived(address, query);

  const size_t capacity = JSON_ARRAY_SIZE(1) + JSON_OBJECT_SIZE(2) + JSON_OBJECT_SIZE(3) + 2 * JSON_OBJECT_SIZE(4) + JSON_OBJECT_SIZE(9) + JSON_OBJECT_SIZE(15) + 1580;
  DynamicJsonDocument doc(capacity);
  deserializeJson(doc, walletGetResponse.c_str());

  //--------------------------------------------
  //  Print the entire returned response string
  Serial.print("Get Wallet Received Transaction: ");
  Serial.println(walletGetResponse.c_str()); // The response is a 'std::string', to Print on Arduino, we need the c_string type.

  JsonObject data_0 = doc["data"][0];
  //--------------------------------------------
  //  the data_0_id parameter will be used to determine if a valid transaction was found.
  if (data_0["id"] == nullptr) {
    Serial.println("No Transaction received");
    return 0;           //no transaction found
  }
  else {
    //--------------------------------------------
    //  check for valid Rental Start transaction type.
    if (!((data_0["type"] == 500) && (data_0["typeGroup"] == 4000))) {

      Serial.println("\nTransaction I don't care about was received");
      bridgechainWallet.lastRXpage++;              //increment global receive transaction counter.
      saveEEPROM(bridgechainWallet.lastRXpage);   //store the page in the Flash
      return 0;
    }
    //--------------------------------------------
    //  Rental Start transaction was received
    JsonObject data_0_asset = data_0["asset"];
    JsonObject data_0_asset_gps = data_0_asset["gps"];

    asset_gps_latitude = data_0_asset_gps["latitude"];
    asset_gps_longitude = data_0_asset_gps["longitude"];
    asset_sessionId = data_0_asset["sessionId"];
    asset_rate = data_0_asset["rate"];

    id = data_0["id"];
    amount = data_0["amount"];
    senderAddress = data_0["sender"];
    senderPublicKey = data_0["senderPublicKey"];
    vendorField = data_0["vendorField"];
    return 1;           //transaction found
  }
}





/********************************************************************************
  // Send a Rental Finish Custom BridgeChain transaction

  view rental finish transaction in explorer.
  https://radians.nl/api/v2/transactions/61ebc45edcc87ca34a50b5e4590e5881dd4148c905bcf8208ad0afd2e7076348

  {
   "data":{
      "id":"61ebc45edcc87ca34a50b5e4590e5881dd4148c905bcf8208ad0afd2e7076348",
      "blockId":"5533205050820353537",
      "version":2,
      "type":600,
      "typeGroup":4000,
      "amount":"1",
      "fee":"10000000",
      "sender":"TRXA2NUACckkYwWnS9JRkATQA453ukAcD1",
      "senderPublicKey":"03e063f436ccfa3dfa9e9e6ee5e08a65a82a5ce2b2daf58a9be235753a971411e2",
      "recipient":"TLdYHTKRSD3rG66zsytqpAgJDX75qbcvgT",
      "signature":"30440220432bf3354b4e6394dd49ae3193ec591778ed12052b16ac85bd30aa341e5f724d022024e495b752469d8648ef010658a848c0acfc5a10cdb0b328749807869c7764f7",
      "asset":{
         "gps":[
            {
               "timestamp":1583475939,
               "latitude":"53.535352",
               "longitude":"-113.277912",
               "human":"2020-03-06T06:25:39.000Z"
            },
            {
               "timestamp":1583475999,
               "latitude":"53.535400",
               "longitude":"-113.277944",
               "human":"2020-03-06T06:26:39.000Z"
            }
         ],
         "sessionId":"dd2691b9d5990f7ec03349fee853154f15ee7416af6784d31a66f5c7224d6061",
         "containsRefund":true,
         "gpsCount":2,
         "rideDuration":60
      },
      "confirmations":614893,
      "timestamp":{
         "epoch":11481664,
         "unix":1583476004,
         "human":"2020-03-06T06:26:44.856Z"
      },
      "nonce":"109"
   }
  }
********************************************************************************/
void SendTransaction_RentalFinish() {

  //--------------------------------------------
  // Retrieve Wallet Nonce from blockchain before sending transaction
  getWallet();

  //--------------------------------------------
  // increment the current nonce.
  // note: If the send fails then we need to unwind this increment.
  bridgechainWallet.walletNonce_Uint64 = bridgechainWallet.walletNonce_Uint64 + 1;

  //--------------------------------------------
  // convert the floating point representation to 64-bit integers
  uint64_t endlat = (uint64_t) (scooterRental.endLatitude * 1000000);
  uint64_t endlon = (uint64_t) (scooterRental.endLongitude * 1000000);

  uint64_t startlat = (uint64_t) (scooterRental.startLatitude * 1000000);
  uint64_t startlon = (uint64_t) (scooterRental.startLongitude * 1000000);

  //--------------------------------------------
  // Use the Transaction Builder to make a transaction.
  auto bridgechainTransaction = builder::radians::ScooterRentalFinish(cfg)
                                .recipientId(scooterRental.senderAddress)
                                .timestamp(scooterRental.startTime, 0)  //uint32_t
                                .latitude(startlat, 0)                  //uint64_t
                                .longitude(startlon, 0)                 //uint64_t
                                .timestamp(scooterRental.endTime, 1)    //uint32_t
                                .latitude(endlat, 1)                    //uint64_t
                                .longitude(endlon, 1)                   //uint64_t
                                .sessionId(scooterRental.sessionID_QRcode_byte)   //array of uint8_t 
                                .containsRefund(false)
                                .fee(10000000)
                                .nonce(bridgechainWallet.walletNonce_Uint64)
                                .amount(1)
                                .sign(PASSPHRASE)
                                .build();

  Serial.println("\n=================================");
  Serial.println("Ride is Finished. Locking scooter.");
  Serial.println("\nSending Rental Finish Transaction");

  //--------------------------------------------
  // Create and Print the Json representation of the Transaction.
  const auto transactionJson = bridgechainTransaction.toJson();
  printf("Bridgechain Transaction: %s\n", transactionJson.c_str());

  //--------------------------------------------
  // Sign the transaction
  bridgechainTransaction.sign(PASSPHRASE);

  //--------------------------------------------
  // Create the JSON bufffer
  char transactionsBuffer[1500];
  snprintf(&transactionsBuffer[0], 1500, "{\"transactions\":[%s]}", bridgechainTransaction.toJson().c_str());
  std::string jsonStr = transactionsBuffer;

  //--------------------------------------------
  // Send the transaction and display the response
  std::string sendResponse = connection.api.transactions.send(jsonStr);
  Serial.println(sendResponse.c_str());

  //--------------------------------------------
  // NOTE: there should be some error handling here in case the transaction failed to send
}




/********************************************************************************
  Send a standard BridgeChain transaction, tailored for a custom network.

  NOTE: This project does not send any standard transactions so the following is not used.

********************************************************************************/
/*

void sendBridgechainTransaction() {
  // Use the Transaction Builder to make a transaction.
  bridgechainWallet.walletNonce_Uint64 = bridgechainWallet.walletNonce_Uint64 + 1;

  char tempVendorField[80];
  strcpy(tempVendorField, "Ride End: ");
  strcat(tempVendorField, scooterRental.sessionID_QRcode);

  auto bridgechainTransaction = builder::Transfer(cfg)
                                // .type(TYPE_0_TYPE)
                                // .senderPublicKey(identities::Keys::fromPassphrase(PASSPHRASE).publicKey.data())
                                //.recipientId("TLdYHTKRSD3rG66zsytqpAgJDX75qbcvgT")        //genesis_2
                                .recipientId(scooterRental.senderAddress)        //genesis_2
                                .vendorField(tempVendorField)
                                .fee(TYPE_0_FEE)
                                .sign(PASSPHRASE)
                                .nonce(bridgechainWallet.walletNonce_Uint64)
                                .amount(10000ULL)
                                .expiration(0UL)
                                //  .secondSign(SecondPassphrase)
                                .build();

  const auto transactionJson = bridgechainTransaction.toJson();
  printf("\n\nBridgechain Transaction: %s\n\n", transactionJson.c_str());

  bridgechainTransaction.sign(PASSPHRASE);

  char transactionsBuffer[600];
  snprintf(&transactionsBuffer[0], 600, "{\"transactions\":[%s]}", bridgechainTransaction.toJson().c_str());
  std::string jsonStr = transactionsBuffer;
  std::string sendResponse = connection.api.transactions.send(jsonStr);
  Serial.println(sendResponse.c_str());
}
*/
