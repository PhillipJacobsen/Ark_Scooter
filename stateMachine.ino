


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

          //QRcodeText = "ark:AUjnVRstxXV4qP3wgKvBgv1yiApvbmcHhx?amount=0.3";

          //this is pseudorandom when the wifi or bluetooth does not have a connection. It can be considered "random" when the radios have a connection
          //arduino random function is overloaded on to esp_random();
          int esprandom = (random(16384, 16777216));    //generate random number with a lower and upper bound

          //char* QRcodeText;               // QRcode Version = 10 with ECC=2 gives 211 Alphanumeric characters or 151 bytes(any characters)
          char QRcodeText[256];
          strcpy(QRcodeText, "rad:");
          strcat(QRcodeText, ArkAddress);
          strcat(QRcodeText, "?hash=");

          char esprandom_char[10];
          itoa(esprandom, esprandom_char, 10);    //convert int to string(base 10 representation).
          QRcodeHash = esprandom_char;            //store the hash in a global to be used later.
          strcat(QRcodeText, esprandom_char);
          strcat(QRcodeText, "&rate=370000000");

          //QRcodeText = "rad:TRXA2NUACckkYwWnS9JRkATQA453ukAcD1?hash=4897212321343433&rate=370000000";  //Public Key, Latitude, Longitude, Rate
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
//use difftime
          //http://www.cplusplus.com/reference/ctime/difftime/
          state = STATE_6;
         // proceed to next state
        }
        else {
          state = STATE_5;
        }

        break;
      }

  }
}



int search_RentalStartTx() {
  if (millis() - previousUpdateTime_RentalStartSearch > UpdateInterval_RentalStartSearch)  {    //poll Ark node every 8 seconds for a new transaction
    previousUpdateTime_RentalStartSearch += UpdateInterval_RentalStartSearch;

    //check to see if new new transaction has been received in wallet
    searchRXpage = lastRXpage + 1;
    if ( GetReceivedTransaction(ArkAddress, searchRXpage, id, amount, senderAddress, senderPublicKey, vendorField) ) {
      lastRXpage++;

      //check to see if vendorField of new transaction matches the field in QRcode that we displayed
      if  (strcmp(vendorField, QRcodeHash) == 0) {
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
