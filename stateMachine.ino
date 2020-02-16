


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
          int esprandom = (random(16384, 16777216));    //generate random number with a lower and upper bound
          char QRcodeText[256 + 1];       // QRcode Version = 10 with ECC=2 gives 211 Alphanumeric characters or 151 bytes(any characters)
         //NOTE!  I wonder if sprintf() is better to use here
      //sprintf, strcpy, strcat (and also strlen function) are all considered dangerous - the all use pointer to buffers -
//there are no checks to see if the destination buffer is large enough to hold the resulting string -
//so can easily lead to buffer overflow.


          
          strcpy(QRcodeText, "rad:");
          strcat(QRcodeText, ArkAddress);
          strcat(QRcodeText, "?hash=");

          char esprandom_char[10];
          itoa(esprandom, esprandom_char, 10);    //convert int to string(base 10 representation).
          //QRcodeHash is 16 characters
          strcpy(QRcodeHash, esprandom_char);      //copy into global character array
          Serial.print("QRcodeHash ");
          Serial.println(QRcodeHash);
          strcat(QRcodeText, esprandom_char);
          strcat(QRcodeText, "&rate=");
          strcat(QRcodeText, RENTAL_RATE_STR);
          //Example: QRcodeText = "rad:TRXA2NUACckkYwWnS9JRkATQA453ukAcD1?hash=4897212321343433&rate=370000000";  //Scooter Address, Hash, Rental Rate
          displayQRcode(QRcodeText);

          previousUpdateTime_RentalStartSearch = millis();    //reset transaction search counter

          state = STATE_4;
          Serial.print("State: ");
          Serial.println(state);
        }
        else {
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
          if (search_RentalStartTx()) {
            Serial.println("Start Ride Timer");
            rideTime_start_ms = millis();
            rideTime_length_ms = 10000;
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

          scooterRental.endLatitude = convertDegMinToDecDeg(GPS.latitude);
          if ( GPS.lat == 'S') {
            scooterRental.endLatitude = (0 - scooterRental.endLatitude);
          }
          scooterRental.endLongitude = convertDegMinToDecDeg(GPS.longitude);
          if ( GPS.lon == 'W') {
            scooterRental.endLongitude = (0 - scooterRental.endLongitude);
          }
          getWallet();                  // Retrieve Wallet Nonce before you send a transaction
          sendBridgechainTransaction();

          Serial.println("");
          Serial.println("=================================");
          Serial.println("Rental Structure: ");
          Serial.println(scooterRental.senderAddress);
          Serial.println(scooterRental.payment);
          Serial.printf("%" PRIu64 "\n", scooterRental.payment_Uint64);   //PRIx64 to print in hexadecimal
          Serial.println(scooterRental.rentalRate);
          Serial.println(scooterRental.startLatitude);      //this prints out only 2 decimal places.  It has 7 decimals
          Serial.println(scooterRental.startLongitude);
          Serial.println(scooterRental.endLatitude);
          Serial.println(scooterRental.endLongitude);    
          Serial.println(scooterRental.vendorField);    
          Serial.println("=================================");
          Serial.println("");
                
          state = STATE_6;
          Serial.print("State: ");
          Serial.println(state);
          // proceed to next state
        }
        else {
          //timer has not expired
          updateSpeedometer();
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

    if ( GetReceivedTransaction(ArkAddress, searchRXpage, id, amount, senderAddress, senderPublicKey, vendorField) ) {
      lastRXpage++;
      Serial.print("Received vendorField: ");
      Serial.println(vendorField);
      Serial.print("QRcodeHash: ");
      Serial.println(QRcodeHash);

      //check to see if vendorField of new transaction matches the field in QRcode that we displayed
      if  (strcmp(vendorField, QRcodeHash) == 0) {

        strcpy(scooterRental.senderAddress, senderAddress);           //copy into global character array
        strcpy(scooterRental.payment, amount);                        //copy into global character array
        scooterRental.payment_Uint64 = strtoull(amount, NULL, 10);    //convert string to unsigned long long global
        strcpy(scooterRental.vendorField, vendorField);               //copy into global character array

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
