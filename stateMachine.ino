





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
          QRcodeText = "ark:AUjnVRstxXV4qP3wgKvBgv1yiApvbmcHhx?amount=0.3";
          displayQRcode(QRcodeText);

          tft.setFont(&FreeSansBold18pt7b);
          tft.setTextColor(ArkRed);
          tft.setCursor(12, 45);
          tft.print("Scan to Ride");
          tft.setFont(&FreeSans9pt7b);
          tft.setTextColor(WHITE);

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
          clearMainScreen();
          tft.drawBitmap(56, 120, myBitmap, 128, 128, ArkRed);   // Display Ark bitmap on middle portion of screen
          state = STATE_0;
        }
        else if (!MQTT_status) {  //check for MQTT disconnect
          clearMainScreen();
          tft.drawBitmap(56, 120, myBitmap, 128, 128, ArkRed);   // Display Ark bitmap on middle portion of screen
          state = STATE_1;
        }
        else if (!ARK_status) {  //check for ARK network disconnect
          clearMainScreen();
          tft.drawBitmap(56, 120, myBitmap, 128, 128, ArkRed);   // Display Ark bitmap on middle portion of screen
          state = STATE_2;
        }
        else if (!GPS_status) {  //check for GPS network disconnect
          clearMainScreen();
          tft.drawBitmap(56, 120, myBitmap, 128, 128, ArkRed);   // Display Ark bitmap on middle portion of screen
          state = STATE_3;
        }
        else {
          state = STATE_4;
        }
        break;

      }
  }

}


bool checkPaymentTimer() {
  timeNow = millis();  //get current time
  if ((timeNow > payment_Timeout)) {

    return true;
  }
  Serial.print("timeout: ");
  Serial.println ((payment_Timeout - timeNow) / 1000);
  tft.fillRect(150, 75, 80, 18, BLACK);     //delete the previous time
  tft.setCursor(150, 90);
  tft.setTextColor(RED);
  tft.println((payment_Timeout - timeNow) / 1000);

  return false;
}

bool checkCancelButton() {
}

bool search_newRX() {
}



void ArkVendingMachine() {         //The Vending state machine
  // Serial.print("vmState: ");
  //  Serial.println(vmState);


  switch (vmState) {              //Depending on the state


    //--------------------------------------------
    case DRAW_HOME: {
        //    drawHomeScreen();
        ARKscan_lasttime = millis();
        vmState = WAIT_FOR_USER;

        break;
      }


    case WAIT_FOR_USER: {

        if (millis() > ARKscan_lasttime + ARK_mtbs)  {

          //check to see if new new transaction has been received in wallet
          searchRXpage = lastRXpage + 1;
          if ( searchReceivedTransaction(ArkAddress, searchRXpage, id, amount, senderAddress, vendorField) ) {
            //a new unknown transaction has been received.
            lastRXpage++;
            Serial.print("Page: ");
            Serial.println(searchRXpage);
            Serial.print("Transaction id: ");
            Serial.println(id);
            Serial.print("Vendor Field: ");
            Serial.println(vendorField);

            tft.fillRect(2, 240, 227, 65, BLACK);     //clear the bottom portion of the screen
            tft.setCursor(4, 250);
            tft.setTextColor(RED);
            tft.println("New Transaction in Wallet");
            tft.setCursor(4, 270);
            tft.print("Page: ");
            tft.println(searchRXpage);
            tft.setCursor(4, 290);
            tft.print("Vendor Field: ");
            tft.println(vendorField);

            ARKscan_lasttime = millis();
            vmState = WAIT_FOR_USER;     //stay in the same state
            break;
          }
          ARKscan_lasttime = millis();
        }



        vmState = WAIT_FOR_USER;     //default state
        break;
      }


    case WAIT_FOR_PAY: {


        vmState = WAIT_FOR_PAY;     //stay in the same state
        break;
      }


    case VEND_ITEM: {
        Serial.println("turn on servo");
        //        servo1.write(180);
        delay(5000);
        //        servo1.write(90);

        Serial.println("turn off servo");

        timeAPIstart = millis();  //get time that API read started


        timeNow = millis() - timeAPIstart;  //get current time
        Serial.print(" send message time:");
        Serial.println(timeNow);


        delay(3000);
        vmState = DRAW_HOME;             //
        break;                          //Get out of switch
      }


  }
}
