


//to do: Putting the transitional code in the state switch case makes your switch statement hard to alter (have to change transitional code in multiple spots).
//  https://www.reddit.com/r/programming/comments/1vrhdq/tutorial_implementing_state_machines/
//  https://pastebin.com/22s5khze

// https://www.embeddedrelated.com/showarticle/723.php

//Finite Mealy State Machine
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
          clearMainScreen();
          //QRcodeText = "ark:AUjnVRstxXV4qP3wgKvBgv1yiApvbmcHhx?amount=0.3";
          QRcodeText = "03e063f436ccfa3dfa9e9e6ee5e08a65a82a5ce2b2daf58a9be235753a971411e2,48.972,-114.868,3.7";  //Public Key, Latitude, Longitude, Rate
          displayQRcode(QRcodeText);

          tft.setFont(&FreeSansBold18pt7b);
          tft.setTextColor(ArkRed);
          tft.setCursor(12, 30);        //12,45
          tft.print("Scan to Ride");
          tft.setFont(&FreeSans9pt7b);
          tft.setTextColor(WHITE);

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
        state = STATE_5;
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
