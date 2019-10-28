/********************************************************************************
  This file contains functions used for generating QRcodes
********************************************************************************/

/********************************************************************************
  This routine will display a large QRcode on a 240x320 TFT display
 ********************************************************************************/

void displayQRcode(char *const QRcodeText) {

  //--------------------------------------------
  // Allocate memory to store the QR code.
  // memory size depends on version number
  uint8_t qrcodeData[qrcode_getBufferSize(QRcode_Version)];

  //qrcode_initText(&qrcode, qrcodeData, QRcode_Version, QRcode_ECC, "ark:AUjnVRstxXV4qP3wgKvBgv1yiApvbmcHhx?amount=0.3");
  qrcode_initText(&qrcode, qrcodeData, QRcode_Version, QRcode_ECC, QRcodeText);

  tft.fillRoundRect(27, 77, 186, 186, 4, WHITE);   //white background with a few pixels of guard around the code

  //this will put the QRcode on the top left corner
  uint8_t x0 = 35;
  uint8_t y0 =  85;   //
  //--------------------------------------------
  //display QRcode
  for (uint8_t y = 0; y < qrcode.size; y++) {
    for (uint8_t x = 0; x < qrcode.size; x++) {

      if (qrcode_getModule(&qrcode, x, y) == 0) {     //change to == 1 to make QR code with black background

        //  we started by setting all background pixels to white so we don't need to clear them again.
        //     tft.drawPixel(x0 + 3 * x,     y0 + 3 * y, TFT_WHITE);
        //     tft.drawPixel(x0 + 3 * x + 1, y0 + 3 * y, TFT_WHITE);
        //     tft.drawPixel(x0 + 3 * x + 2, y0 + 3 * y, TFT_WHITE);

        //     tft.drawPixel(x0 + 3 * x,     y0 + 3 * y + 1, TFT_WHITE);
        //     tft.drawPixel(x0 + 3 * x + 1, y0 + 3 * y + 1, TFT_WHITE);
        //     tft.drawPixel(x0 + 3 * x + 2, y0 + 3 * y + 1, TFT_WHITE);

        //     tft.drawPixel(x0 + 3 * x,     y0 + 3 * y + 2, TFT_WHITE);
        //     tft.drawPixel(x0 + 3 * x + 1, y0 + 3 * y + 2, TFT_WHITE);
        //     tft.drawPixel(x0 + 3 * x + 2, y0 + 3 * y + 2, TFT_WHITE);

      } else {

        //uncomment to double the QRcode. Comment to display normal code size
        tft.drawPixel(x0 + 3 * x,     y0 + 3 * y, QRcodeDarkPixelColor);
        tft.drawPixel(x0 + 3 * x + 1, y0 + 3 * y, QRcodeDarkPixelColor);
        tft.drawPixel(x0 + 3 * x + 2, y0 + 3 * y, QRcodeDarkPixelColor);

        tft.drawPixel(x0 + 3 * x,     y0 + 3 * y + 1, QRcodeDarkPixelColor);
        tft.drawPixel(x0 + 3 * x + 1, y0 + 3 * y + 1, QRcodeDarkPixelColor);
        tft.drawPixel(x0 + 3 * x + 2, y0 + 3 * y + 1, QRcodeDarkPixelColor);

        tft.drawPixel(x0 + 3 * x,     y0 + 3 * y + 2, QRcodeDarkPixelColor);
        tft.drawPixel(x0 + 3 * x + 1, y0 + 3 * y + 2, QRcodeDarkPixelColor);
        tft.drawPixel(x0 + 3 * x + 2, y0 + 3 * y + 2, QRcodeDarkPixelColor);

      }
    }
  }


}
