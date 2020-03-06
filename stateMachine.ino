


//to do: Putting the transitional code in the state switch case makes your switch statement hard to alter (have to change transitional code in multiple spots).
//  https://www.reddit.com/r/programming/comments/1vrhdq/tutorial_implementing_state_machines/
//  https://pastebin.com/22s5khze

// https://www.embeddedrelated.com/showarticle/723.php

//Finite State Machine
void StateMachine() {
  switch (state) {
    case STATE_0: {
        if (WiFi_status) {
          state = STATE_1;
          Serial.print("State: ");
          Serial.println(state);
        }
        else {
          rentalStatus = "Broken";
          state = STATE_0;
        }
        break;
      }

    case STATE_1: {
        if (!WiFi_status) {     //check for WiFi disconnect
          state = STATE_0;
        }
        else if (MQTT_status) {  //wait for MQTT connect
          state = STATE_2;
          Serial.print("State: ");
          Serial.println(state);
        }
        else {
          rentalStatus = "Broken";
          state = STATE_1;
        }
        break;
      }


    case STATE_2: {
        if (!WiFi_status) {     //check for WiFi disconnect
          state = STATE_0;
        }
        else if (!MQTT_status) {  //check for MQTT disconnect
          state = STATE_1;
        }
        else if (ARK_status) {  //wait for ARK network connect
          state = STATE_3;
          Serial.print("State: ");
          Serial.println(state);
        }
        else {
          rentalStatus = "Broken";
          state = STATE_2;
        }
        break;
      }

    case STATE_3: {
        if (!WiFi_status) {     //check for WiFi disconnect
          state = STATE_0;
        }
        else if (!MQTT_status) {  //check for MQTT disconnect
          state = STATE_1;
        }
        else if (!ARK_status) {  //check for ARK network disconnect
          state = STATE_2;
        }
        else if (GPS_status) {  //wait for GPS fix


          //this is pseudorandom when the wifi or bluetooth does not have a connection. It can be considered "random" when the radios have a connection
          //arduino random function is overloaded on to esp_random();
          // https://docs.espressif.com/projects/esp-idf/en/latest/api-reference/system/system.html

          uint32_t esprandom = (esp_random());    //generate 32 bit random number with a lower and upper bound using ESP32 RNG.
          //int esprandom = (random(16384, 16777216));    //This uses Arduino PRNG that is overloaded. Provide it with a lower and upper bound

          char QRcodeText[256 + 1];       // QRcode Version = 10 with ECC=2 gives 211 Alphanumeric characters or 151 bytes(any characters)
          //NOTE!  I wonder if sprintf() is better to use here
          //sprintf, strcpy, strcat (and also strlen function) are all considered dangerous - the all use pointer to buffers -
          //there are no checks to see if the destination buffer is large enough to hold the resulting string -
          //so can easily lead to buffer overflow.

          strcpy(QRcodeText, "rad:");
          strcat(QRcodeText, ArkAddress);

          //   start sha256
          // use this to check result of SHA256 https://passwordsgenerator.net/sha256-hash-generator/
          // http://www.fileformat.info/tool/hash.htm

          //byte shaResult[32];
          char SHApayload[10 + 1]; //max number is 4294967295
          //itoa(esprandom, SHApayload, 10);      //this will interpret numbers as signed
          utoa(esprandom, SHApayload, 10);        //use this instead for unsigned conversion
          const size_t payloadLength = strlen(SHApayload);       //holds length of payload
          mbedtls_md_init(&ctx);
          mbedtls_md_setup(&ctx, mbedtls_md_info_from_type(md_type), 0);
          mbedtls_md_starts(&ctx);
          mbedtls_md_update(&ctx, (const unsigned char *) SHApayload, payloadLength);
          mbedtls_md_finish(&ctx, shaResult);
          mbedtls_md_free(&ctx);

          Serial.print("value to be Hashed: ");
          Serial.println(SHApayload);

          char shaResult_char[64 + 1];
          shaResult_char[0] = '\0';

          for (int i = 0; i < sizeof(shaResult); i++) {
            char str[3];
            sprintf(str, "%02x", (int)shaResult[i]);
            //           Serial.print(str);
            strcat(shaResult_char, str);
          }
          Serial.print("QRcode SHA256: ");
          Serial.println(shaResult_char);
          //end sha256

          strcat(QRcodeText, "?hash=");
          //hardcode Hash for testing
          strcat(QRcodeText, "1234300000000000000000000000000000000000000000000000000000000000");     //append hash
          strcpy(QRcodeHash, "1234300000000000000000000000000000000000000000000000000000000000");    //stash hash away for use later in rental start transaction
          //QRcodeHash_Byte = shaResult;      //stash away so we can send in Rental Finish Transaction

          // strcat(QRcodeText, shaResult_char);     //append hash
          //  strcpy(QRcodeHash, shaResult_char);    //stash hash away for use later in rental start transaction

          //strcat(QRcodeText, SHApayload);  //use this if you want to use random number instead of SHA256

          strcat(QRcodeText, "&rate=");
          strcat(QRcodeText, RENTAL_RATE_STR);


          scooterRental.QRLatitude = convertDegMinToDecDeg(GPS.latitude);
          if ( GPS.lat == 'S') {
            scooterRental.QRLatitude = (0 - scooterRental.QRLatitude);
          }

          scooterRental.QRLongitude = convertDegMinToDecDeg(GPS.longitude);
          if ( GPS.lon == 'W') {
            scooterRental.QRLongitude = (0 - scooterRental.QRLongitude);
          }

          //buf += String(scooterRental.QRLatitude, 4);    // use 6 decimal point precision. alternate method
          char QRLatitude[13];
          snprintf(&QRLatitude[0], 13, "%.6f", scooterRental.QRLatitude);             //create string with 6 decimal point.

          char QRLongitude[13];
          snprintf(&QRLongitude[0], 13, "%.6f", scooterRental.QRLongitude);             //create string with 6 decimal point.

          strcat(QRcodeText, "&lat=");
          strcat(QRcodeText, QRLatitude);

          strcat(QRcodeText, "&lon=");
          strcat(QRcodeText, QRLongitude);

          Serial.print("QR text: ");
          Serial.println(QRcodeText);

          //Example: QRcodeText = "rad:TRXA2NUACckkYwWnS9JRkATQA453ukAcD1?hash=4897212321343433&rate=370000000";  //Scooter Address, Hash, Rental Rate
          //Example "rad:TRXA2NUACckkYwWnS9JRkATQA453ukAcD1?hash=1234300000000000000000000000000000000000000000000000000000000000&rate=370000000&lat=-180.222222&lon=1.111111"

          displayQRcode(QRcodeText);

          previousUpdateTime_RentalStartSearch = millis();    //reset transaction search counter

          rentalStatus = "Available";

          state = STATE_4;
          Serial.print("State: ");
          Serial.println(state);
        }
        else {
          rentalStatus = "Broken";
          state = STATE_3;
        }
        break;

      }

    case STATE_4: {
        if (!WiFi_status) {     //check for WiFi disconnect
          DisplayArkBitmap();
          state = STATE_0;
        }
        else if (!MQTT_status) {  //check for MQTT disconnect
          DisplayArkBitmap();
          state = STATE_1;
        }
        else if (!ARK_status) {  //check for ARK network disconnect
          DisplayArkBitmap();
          state = STATE_2;
        }
        else if (!GPS_status) {  //check for GPS network disconnect
          DisplayArkBitmap();
          state = STATE_3;
        }
        else {                  //we are looking for a Rental Start Tx
          if (search_RentalStartTx()) {       //
            Serial.println("Start Ride Timer");
            rideTime_start_ms = millis();
            
            scooterRental.startTime = time(nullptr);

            //rideTime_length_ms
            uint64_t rideTime_length_min = scooterRental.payment_Uint64 / RENTAL_RATE_UINT64;     //# of minutes    rate = .037RAD per minute
            rideTime_length_ms = rideTime_length_min * 60000;
            Serial.print("ride time length: ");
            Serial.println(rideTime_length_ms);

            //uint32_t rideTime_length_ms

            //rideTime_length_ms = 10000;
            remainingRentalTime_previous = rideTime_length_ms;

            clearMainScreen();
            tft.setFont(&Lato_Medium_36);
            tft.setTextColor(SpeedGreen);     // http://www.barth-dev.de/online/rgb565-color-picker/
            tft.setCursor(75, 150);
            tft.print("km/h");

            previousSpeed = 0;
            updateSpeedometer();
            //we need to update the initial display here
            //remainingRentalTime_previous = 0;

            //startRideTimer();
            //unlockScooter();

            rentalStatus = "Rented";
            state = STATE_5;
            Serial.print("State: ");
            Serial.println(state);
            break;
          }
          else {
            state = STATE_4;
            break;
          }
        }
        break;
      }

    case STATE_5: {   // rider is using scooter
        //wait for timer to expire and then lock scooter and send rental finish and go back to beginning
        if (millis() - rideTime_start_ms > rideTime_length_ms)  {
          //timer has expired
          //use difftime
          //http://www.cplusplus.com/reference/ctime/difftime/

          scooterRental.endTime = time(nullptr);

          scooterRental.endLatitude = convertDegMinToDecDeg(GPS.latitude);
          if ( GPS.lat == 'S') {
            scooterRental.endLatitude = (0 - scooterRental.endLatitude);
          }
          scooterRental.endLongitude = convertDegMinToDecDeg(GPS.longitude);
          if ( GPS.lon == 'W') {
            scooterRental.endLongitude = (0 - scooterRental.endLongitude);
          }


          getWallet();                  // Retrieve Wallet Nonce before you send a transaction
          //sendBridgechainTransaction();    //this sends a standard transaction
          SendTransaction_RentalFinish();



          Serial.println("");
          Serial.println("=================================");
          Serial.println("Rental Structure: ");
          Serial.println(scooterRental.senderAddress);
          Serial.println(scooterRental.payment);
          Serial.printf("%" PRIu64 "\n", scooterRental.payment_Uint64);   //PRIx64 to print in hexadecimal
          Serial.println(scooterRental.rentalRate);
          Serial.println(scooterRental.startLatitude, 6);     //this prints out only 6 decimal places.  It has 8 decimals
          Serial.println(scooterRental.startLongitude, 6);
          Serial.println(scooterRental.endLatitude, 6);
          Serial.println(scooterRental.endLongitude, 6);
          Serial.println(scooterRental.vendorField);
          Serial.println("=================================");
          Serial.println("");

          rentalStatus = "Available";
          state = STATE_6;
          Serial.print("State: ");
          Serial.println(state);
          // proceed to next state
        }
        else {
          //timer has not expired
          updateSpeedometer();      // no speed is shown initially if there is no change.
          updateCountdownTimer();
          state = STATE_5;
        }
        break;
      }



    case STATE_6: {
        state = STATE_3;
        Serial.print("State: ");
        Serial.println(state);
        break;
      }

  }
}



int search_RentalStartTx() {
  if (millis() - previousUpdateTime_RentalStartSearch > UpdateInterval_RentalStartSearch)  {    //poll Ark node every 8 seconds for a new transaction
    previousUpdateTime_RentalStartSearch += UpdateInterval_RentalStartSearch;

    //  check to see if new new transaction has been received in wallet
    // lastRXpage is the page# of the last received transaction
    int searchRXpage = lastRXpage + 1;
    const char* id;              //transaction ID
    const char* amount;           //transactions amount
    const char* senderAddress;    //transaction address of sender
    const char* senderPublicKey;  //transaction address of sender
    const char* vendorField;      //vendor field

    const char* asset_gps_latitude;
    const char* asset_gps_longitude;
    const char* asset_sessionId;
    const char* asset_rate;

    //if ( GetReceivedTransaction(ArkAddress, searchRXpage, id, amount, senderAddress, senderPublicKey, vendorField) ) {
    if ( GetTransaction_RentalStart(ArkAddress, searchRXpage, id, amount, senderAddress, senderPublicKey, vendorField, asset_gps_latitude, asset_gps_longitude, asset_sessionId, asset_rate) ) {
      lastRXpage++;   //increment received counter if rental start was received.
      lastRXpage_eeprom = lastRXpage;
      saveCredentials();
      Serial.print("Received sessionId: ");
      Serial.println(asset_sessionId);
      Serial.print("QRcodeHash: ");
      //Serial.println(QRcodeHash);shaResult_char
      Serial.println(QRcodeHash);

      //check to see if vendorField of new transaction matches the field in QRcode that we displayed
      //if  (strcmp(vendorField, QRcodeHash) == 0) {

      if  (strcmp(asset_sessionId, QRcodeHash) == 0) {

        strcpy(scooterRental.senderAddress, senderAddress);           //copy into global character array
        strcpy(scooterRental.payment, amount);                        //copy into global character array
        scooterRental.payment_Uint64 = strtoull(amount, NULL, 10);    //convert string to unsigned long long global
        //   strcpy(scooterRental.vendorField, vendorField);               //copy into global character array
        strcpy(scooterRental.sessionID, asset_sessionId);               //copy into global character array

        scooterRental.startLatitude = convertDegMinToDecDeg(GPS.latitude);
        if ( GPS.lat == 'S') {
          scooterRental.startLatitude = (0 - scooterRental.startLatitude);
        }
        scooterRental.startLongitude = convertDegMinToDecDeg(GPS.longitude);
        if ( GPS.lon == 'W') {
          scooterRental.startLongitude = (0 - scooterRental.startLongitude);
        }

        return 1;
      }
      else {        //we received a transaction that did not match. We should issue refund.
        Serial.print("session id did not match hash embedded in QRcode");
        // issueRefund();
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
