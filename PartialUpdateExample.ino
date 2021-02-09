/*
    LilyGo Ink Screen Series Test
        - Created by Lewis he
*/

// According to the board, cancel the corresponding macro definition
#define LILYGO_T5_V213
// #define LILYGO_T5_V22
// #define LILYGO_T5_V24
// #define LILYGO_T5_V28


#include <boards.h>
#include <GxEPD.h>
#include <SD.h>
#include <FS.h>

// #include <GxGDEH0154D67/GxGDEH0154D67.h>  // 1.54" b/w
// #include <GxDEPG0150BN/GxDEPG0150BN.h>    // 1.54" b/w newer panel

// #include <GxGDEW027W3/GxGDEW027W3.h>      // 2.7" b/w

// #include <GxGDEH0213B72/GxGDEH0213B72.h>  // 2.13" b/w new panel
#include <GxGDEH0213B73/GxGDEH0213B73.h>  // 2.13" b/w newer panel
// #include <GxDEPG0213BN/GxDEPG0213BN.h>    // 2.13" b/w newer panel

// #include <GxGDEH029A1/GxGDEH029A1.h>      // 2.9" b/w
// #include <GxQYEG0290BN/GxQYEG0290BN.h>    // 2.9" b/w new panel
// #include <GxDEPG0290B/GxDEPG0290B.h>      // 2.9" b/w new panel


// The following screens are not supported
// #include <GxGDEW0213Z16/GxGDEW0213Z16.h>  // 2.13" b/w/r
// #include <GxGDEW0154Z04/GxGDEW0154Z04.h>  // 1.54" b/w/r 200x200
// #include <GxGDEW0154Z17/GxGDEW0154Z17.h>  // 1.54" b/w/r 152x152
// #include <GxGDEW027C44/GxGDEW027C44.h>    // 2.7" b/w/r
// #include <GxGDEW029Z10/GxGDEW029Z10.h>    // 2.9" b/w/r



#include GxEPD_BitmapExamples

// FreeFonts from Adafruit_GFX
#include <Fonts/FreeMonoBold9pt7b.h>
#include <Fonts/FreeMonoBold12pt7b.h>
#include <Fonts/FreeMonoBold18pt7b.h>
#include <Fonts/FreeMonoBold24pt7b.h>
#include <GxIO/GxIO_SPI/GxIO_SPI.h>
#include <GxIO/GxIO.h>
#include <WiFi.h>

GxIO_Class io(SPI,  EPD_CS, EPD_DC,  EPD_RSET);
GxEPD_Class display(io, EPD_RSET, EPD_BUSY);


void setup(void)
{
    Serial.begin(115200);
    Serial.println();
    Serial.println("setup");
    display.init(); // enable diagnostic output on Serial
    Serial.println("setup done");
}


void loop()
{
// use asymmetric values for test
    uint16_t box_x = 20;
    uint16_t box_y = 25;
    uint16_t box_w = 60;
    uint16_t box_h = 20;
    uint16_t cursor_y = GxEPD_WIDTH/2;
    float value = 13.95;
    display.setRotation(1);
    display.setFont(&FreeMonoBold24pt7b);
    display.setTextColor(GxEPD_BLACK);

    // partial update to full screen to preset for partial update of box window
    // (this avoids strange background effects)
//    display.fillRect(0, 0, GxEPD_HEIGHT, GxEPD_WIDTH, GxEPD_WHITE);
//    display.updateWindow(0, 0, GxEPD_HEIGHT, GxEPD_WIDTH, true);

    for (uint16_t i = 1; i <= 10; i++) {
        Serial.printf("i=%d, x=%d, y=%d\n",i, box_x, cursor_y);
        display.fillRect(0, 0, GxEPD_HEIGHT, GxEPD_WIDTH, GxEPD_WHITE);
//        display.eraseDisplay(true); // not partial
        display.setCursor(box_x, cursor_y);
        display.print(value * i, 2);
        display.updateWindow(0, 0, GxEPD_WIDTH, GxEPD_HEIGHT, false);
//            display.updateWindow(box_x, box_y, box_w, box_h, true);
        display.powerDown();
        // FIXME: need to follow instructions and powerdown, but the display starts to fade
        delay(2000);
    }

    delay(10000);


  /* time_now_s = (millis() - time_start_ms) / 1000; */
  /* char time_string[] = {'0', '0', ':', '0', '0', '\0'}; */
  /* time_string[0] = time_now_s / 60 / 10 + '0'; */
  /* time_string[1] = time_now_s / 60 % 10 + '0'; */
  /* time_string[3] = time_now_s % 60 / 10 + '0'; */
  /* time_string[4] = time_now_s % 60 % 10 + '0'; */

  /* paint.SetWidth(32); */
  /* paint.SetHeight(96); */
  /* paint.SetRotate(ROTATE_90); */

  /* paint.Clear(UNCOLORED); */
  /* paint.DrawStringAt(0, 4, time_string, &Font24, COLORED); */
  /* epd.SetFrameMemory(paint.GetImage(), 80, 72, paint.GetWidth(), paint.GetHeight()); */
  /* epd.DisplayFrame(); */

  /* delay(500); */

}
