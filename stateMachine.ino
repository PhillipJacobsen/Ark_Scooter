/********************************************************************************
  This file contains the main Finite State Machine logic for controlling the rental session

********************************************************************************/
//--------------------------------------------
// Mealy Finite State Machine
// The state machine logic is executed once each cycle of the Arduino "main" loop.
//--------------------------------------------
void StateMachine() {
  switch (state) {

    //--------------------------------------------
    // State 0
    // Initial state after microcontroller powers up and initializes the various peripherals
    // Transitions to State 1 once WiFi is connected
    case STATE_0: {
        if (WiFi_status) {          //wait for WiFi to connect
          state = STATE_1;
          Serial.print("State: ");
          Serial.println(state);
        }
        else {
          scooterRental.rentalStatus = "Broken";
          state = STATE_0;
        }
        break;
      }

    //--------------------------------------------
    // State 1
    // Transistions to state 2 once connected to MQTT broker
    // Return to state 0 if WiFi disconnects
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
          scooterRental.rentalStatus = "Broken";
          state = STATE_1;
        }
        break;
      }

    //--------------------------------------------
    // State 2
    // Transistions to state 3 once connected to Ark Node
    // Return to state 0 if WiFi disconnects
    // Returns to state 1 if MQTT disconnects
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
          scooterRental.rentalStatus = "Broken";
          state = STATE_2;
        }
        break;
      }


    //--------------------------------------------
    // State 3
    // Transistions to state 4 once GPS gets a satellite lock.
    // Transition Actions:
    //  -rentalStatus = "Available"
    //  -generate and display QR code
    //
    // Return to state 0 if WiFi disconnects
    // Returns to state 1 if MQTT disconnects
    // Returns to state 2 if Ark network disconnects
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



          GenerateDisplay_QRcode();

          previousUpdateTime_RentalStartSearch = millis();    //reset transaction search counter

          scooterRental.rentalStatus = "Available";
          state = STATE_4;
          Serial.print("State: ");
          Serial.println(state);
        }
        else {
          scooterRental.rentalStatus = "Broken";
          state = STATE_3;
        }
        break;

      }



    //--------------------------------------------
    // State 4
    // Transistions to state 5 once valid RentalStart blockchain transaction is received
    // Transition Actions:
    //  -unlock scooter
    //  -rentalStatus = "Rented"
    //  -Start Ride Timer
    //  -Initialize display with speedometer and ride timer
    //
    // Return to state 0 if WiFi disconnects
    // Returns to state 1 if MQTT disconnects
    // Returns to state 2 if Ark network disconnects
    // Returns to state 3 if GPS loses lock
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
            Serial.println("\nUnlocking Scooter & Starting Ride Timer");
            scooterRental.startTime = time(nullptr);      //record Unix timestamp of the Rental start
            rideTime_start_ms = millis();                 //We are using the ms timer for the ride timer. This id probably redundant. We could use the previous unix timer

            //record current GPS coordinates at the start of the Rental
            scooterRental.startLatitude = convertDegMinToDecDeg_lat(GPS.latitude);
            scooterRental.startLongitude = convertDegMinToDecDeg_lon(GPS.longitude);

            //calculate the ride length = received payment / Rental rate(RAD/seconds)
            uint64_t rideTime_length_sec = scooterRental.payment_Uint64 / RENTAL_RATE_UINT64;

// NOTE, println does not support uint64_t           
//            Serial.print("Ride time length(seconds): ");
//            Serial.println(rideTime_length_sec);
            
            rideTime_length_ms = rideTime_length_sec * 1000;      //convert to ms
            Serial.print("Ride time length(ms): ");
            Serial.println(rideTime_length_ms); 
            
            remainingRentalTime_previous_s = rideTime_length_sec;    //this is used by the countdown timer to refresh the display only once each second.
            // remainingRentalTime_previous_s = 0;   //Might need to use this to ensure that timer display shows the initial timer value.  Otherwise it might not show anything until the first second has elapsed.

            //erase QRcode from display and show speedometer and ride timer
            clearMainScreen();
            tft.setFont(&Lato_Medium_36);
            tft.setTextColor(SpeedGreen);     // http://www.barth-dev.de/online/rgb565-color-picker/
            tft.setCursor(75, 150);
            tft.print("km/h");

            previousSpeed = 0;
            updateSpeedometer();

            unlockScooter();                  //put control logic to unlock scoote here

            scooterRental.rentalStatus = "Rented";
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


    //--------------------------------------------
    // State 5
    // Transistions to state 6 once ride timer expires.  Speedometer and Ride timer are regularly updated on the display
    // Transition Actions:
    //  -lock scooter
    //  -rentalStatus = "Available"
    //  -Send RentalFinish blockchan transaction (code currently does not check to make sure a WiFi connection is available prior to sending).
    //  -Initialize display with speedometer and ride timer
    //
    // In this state the WiFi, MQTT, Ark, and GPS connections are ignored.
    case STATE_5: {   // rider is using scooter
        //wait for timer to expire and then lock scooter and send rental finish and go back to beginning
        if (millis() - rideTime_start_ms > rideTime_length_ms)  {
          //timer has expired
          //use difftime
          //http://www.cplusplus.com/reference/ctime/difftime/

          scooterRental.endTime = time(nullptr);  //record Unix timestamp of the Rental Finish

          //record GPS coordinates of the Rental Finish
          scooterRental.endLatitude = convertDegMinToDecDeg_lat(GPS.latitude);
          scooterRental.endLongitude = convertDegMinToDecDeg_lon(GPS.longitude);
         
          SendTransaction_RentalFinish(); // send Rental Finish transaction

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

//  Serial.print("endlat ");
//  Serial.printf("%" PRIu64 "\n", endlat);   //PRIx64 to print in hexadecimal
//  Serial.println("");
//  Serial.print("endlon ");
//  Serial.printf("%" PRIu64 "\n", endlon);   //PRIx64 to print in hexadecimal
//  Serial.println("");


          scooterRental.rentalStatus = "Available";
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


    //--------------------------------------------
    // State 6
    // Currently nothing happens in this state.
    // Go immediately back to state 3.
    case STATE_6: {
        state = STATE_3;
        Serial.print("State: ");
        Serial.println(state);
        break;
      }

  }
}




/********************************************************************************
  updates the ride countdown timer Displayed on the screen.
  It only refreshes the screen once per second
********************************************************************************/
void updateCountdownTimer() {

  uint32_t remainingRentalTime_s = rideTime_length_ms - (millis() - rideTime_start_ms);   //calculate remaining ride time in ms
  //need to check for time < 0
  //  if (remainingRentalTime_s < 0) {  This won't work because of the unsigned type
  if (remainingRentalTime_s > rideTime_length_ms) {  //checks for wrap of unsigned type
    remainingRentalTime_s = 0;
  }
  else {
    remainingRentalTime_s = remainingRentalTime_s / 1000;   //# of seconds

  }


  if (remainingRentalTime_s != remainingRentalTime_previous_s) {
    //update display every second

    //create the string this is currently displayed on the screen.  We are going to use this to calculate how much of the screen to erase
    char previousTimer_char[10];
    snprintf(&previousTimer_char[0], 10, "%u", remainingRentalTime_previous_s);        //create string from unsigned int

    //create the string that we want to write to the display.
    char currentTimer_char[10];
    snprintf(&currentTimer_char[0], 10, "%u", remainingRentalTime_s);             //create string from unsigned int

    remainingRentalTime_previous_s = remainingRentalTime_s;               //update previous timer
    //update countdown timer display
    int16_t  x1, y1;
    uint16_t w, h;
    tft.setFont(&Lato_Semibold_48);
    tft.setTextColor(OffWhite);
    tft.getTextBounds(previousTimer_char, 70, 230, &x1, &y1, &w, &h);   //get bounds of the previous speed text
    tft.fillRect(x1, y1, w, h, BLACK);                                  //erase the last speed reading

    //display the updated countdowntimer on the display
    tft.setCursor(70, 230);
    tft.print(remainingRentalTime_s);
  }
}


/********************************************************************************
  unlock the scooter
********************************************************************************/
void unlockScooter() {

}


/********************************************************************************
  updates the speedometer displayed on the screen.
  It only refreshes the screen if the speed changes
********************************************************************************/
void updateSpeedometer() {

  char previousSpeed_char[6];
  snprintf(&previousSpeed_char[0], 6, "%.1f", previousSpeed);   //create string of prevoius speed with 1 decimal point.

  float speedkmh = GPS.speed * 1.852;                           //get current speed with full precision
  char currentSpeed_char[6];
  snprintf(&currentSpeed_char[0], 6, "%.1f", speedkmh);         //create string with 1 decimal point.

  if  (strcmp(currentSpeed_char, previousSpeed_char) == 0) {
    return;
  }

  int16_t  x1, y1;
  uint16_t w, h;
  tft.setFont(&Lato_Black_96);
  tft.getTextBounds(previousSpeed_char, 30, 105, &x1, &y1, &w, &h);   //get bounds of the previous speed text
  tft.fillRect(x1, y1, w, h, BLACK);                                  //clear the last speed reading

  //display updated speed
  previousSpeed = speedkmh;               //update previous speed
  //tft.setFont(&Lato_Black_96);
  tft.setTextColor(SpeedGreenDarker);
  tft.setCursor(30, 105);
  tft.print(speedkmh, 1);
}


/********************************************************************************
  Generates the QRcode and displays on TFT display
********************************************************************************/
void GenerateDisplay_QRcode () {

  uint32_t esprandom = (esp_random());            //generate 32 bit random number with a lower and upper bound using ESP32 RNG.
  // this is pseudorandom when the wifi or bluetooth does not have a connection. It can be considered "random" when the radios have a connection
  // arduino random function is overloaded on to esp_random();
  // https://docs.espressif.com/projects/esp-idf/en/latest/api-reference/system/system.html


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

  char SHApayload[10 + 1]; //max number is 4294967295
  //itoa(esprandom, SHApayload, 10);      //this will interpret numbers as signed
  utoa(esprandom, SHApayload, 10);        //use this instead for unsigned conversion
  const size_t payloadLength = strlen(SHApayload);       //holds length of payload
  mbedtls_md_init(&ctx);
  mbedtls_md_setup(&ctx, mbedtls_md_info_from_type(md_type), 0);
  mbedtls_md_starts(&ctx);
  mbedtls_md_update(&ctx, (const unsigned char *) SHApayload, payloadLength);
  mbedtls_md_finish(&ctx, shaResult);   //shaResult is global variable of type bytes
  mbedtls_md_free(&ctx);

  //display the value to be hashed
  Serial.print("value to be Hashed: ");
  Serial.println(SHApayload);

  //convert the SHAresult which is an array of bytes into an array of characters so we can send to terminal display
  char shaResult_char[64 + 1];
  shaResult_char[0] = '\0';
  for (int i = 0; i < sizeof(shaResult); i++) {
    char str[3];
    sprintf(str, "%02x", (int)shaResult[i]);
    //           Serial.print(str);
    strcat(shaResult_char, str);
  }
  //display the resulting SHA256
  Serial.print("QRcode SHA256: ");
  Serial.println(shaResult_char);
  //end sha256

  strcat(QRcodeText, "?hash=");
  //hardcode Hash for testing
  //strcat(QRcodeText, "1234300000000000000000000000000000000000000000000000000000000000");     //append hash
  //strcpy(QRcodeHash, "1234300000000000000000000000000000000000000000000000000000000000");    //stash hash away for use later in rental start transaction

  strcat(QRcodeText, shaResult_char);     //append hash to QRcode string
  strcpy(QRcodeHash, shaResult_char);    //stash hash away for use later in rental start transaction handler

  strcat(QRcodeText, "&rate=");
  strcat(QRcodeText, RENTAL_RATE_STR);


  scooterRental.QRLatitude = convertDegMinToDecDeg_lat(GPS.latitude);
  scooterRental.QRLongitude = convertDegMinToDecDeg_lon(GPS.longitude);


  //buf += String(scooterRental.QRLatitude, 4);    // use 6 decimal point precision. alternate method
  // http://joequery.me/code/snprintf-c/
  char QRLatitude[13];
  snprintf(&QRLatitude[0], 13, "%.6f", scooterRental.QRLatitude);             //create string with 6 decimal point.

  char QRLongitude[13];
  snprintf(&QRLongitude[0], 13, "%.6f", scooterRental.QRLongitude);           //create string with 6 decimal point.

  strcat(QRcodeText, "&lat=");
  strcat(QRcodeText, QRLatitude);

  strcat(QRcodeText, "&lon=");
  strcat(QRcodeText, QRLongitude);

  Serial.print("QR text: ");
  Serial.println(QRcodeText);

  //Example QRcodeText = "rad:TRXA2NUACckkYwWnS9JRkATQA453ukAcD1?hash=1234300000000000000000000000000000000000000000000000000000000000&rate=370000000&lat=-180.222222&lon=1.111111"
  displayQRcode(QRcodeText);    //display on the screen
}
