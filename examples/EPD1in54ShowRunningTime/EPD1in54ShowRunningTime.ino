/**
 *  @filename   :   EPD1in54ShowRunningTime.ino
 *  @brief      :   1.54inch e-paper display demo
 *  @author     :   Yehui from Waveshare
 *
 *  Copyright (C) Waveshare     September 5 2017
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documnetation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to  whom the Software is
 * furished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS OR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <SPI.h>
#include <EPD1in54.h>
#include <EPDPaint.h>
#include "imagedata.h"

#define COLORED     0
#define UNCOLORED   1

/**
  * Due to RAM not enough in Arduino UNO, a frame buffer is not allowed.
  * In this case, a smaller image buffer is allocated and you have to
  * update a partial display several times.
  * 1 byte = 8 pixels, therefore you have to set 8*N pixels at a time.
  */
unsigned char image[1024];
EPDPaint paint(image, 0, 0);    // width should be the multiple of 8
EPD1in54 epd; // default reset: 8, dc: 9, cs: 10, busy: 7
// EPD1in54 epd(33, 25, 26, 27); // reset, dc, cs, busy
unsigned long timeStartMs;
unsigned long timeNowS;
unsigned long timeShowedS = 1000;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);

  if (epd.init(lutFullUpdate) != 0) {
    Serial.print("e-Paper init failed");
    return;
  }

  /**
   *  there are 2 memory areas embedded in the e-paper display
   *  and once the display is refreshed, the memory area will be auto-toggled,
   *  i.e. the next action of SetFrameMemory will set the other memory area
   *  therefore you have to clear the frame memory twice.
   */
  epd.clearFrameMemory(0xFF);   // bit set = white, bit reset = black
  epd.displayFrame();
  epd.clearFrameMemory(0xFF);   // bit set = white, bit reset = black
  epd.displayFrame();

  paint.setRotate(ROTATE_0);
  paint.setWidth(200);
  paint.setHeight(24);

  /* For simplicity, the arguments are explicit numerical coordinates */
  paint.clear(COLORED);
  paint.drawStringAt(30, 4, "Hello world!", &Font16, UNCOLORED);
  epd.setFrameMemory(paint.getImage(), 0, 10, paint.getWidth(), paint.getHeight());

  paint.clear(UNCOLORED);
  paint.drawStringAt(30, 4, "e-Paper Demo", &Font16, COLORED);
  epd.setFrameMemory(paint.getImage(), 0, 30, paint.getWidth(), paint.getHeight());

  paint.setWidth(64);
  paint.setHeight(64);

  paint.clear(UNCOLORED);
  paint.drawRectangle(0, 0, 40, 50, COLORED);
  paint.drawLine(0, 0, 40, 50, COLORED);
  paint.drawLine(40, 0, 0, 50, COLORED);
  epd.setFrameMemory(paint.getImage(), 16, 60, paint.getWidth(), paint.getHeight());

  paint.clear(UNCOLORED);
  paint.drawCircle(32, 32, 30, COLORED);
  epd.setFrameMemory(paint.getImage(), 120, 60, paint.getWidth(), paint.getHeight());

  paint.clear(UNCOLORED);
  paint.drawFilledRectangle(0, 0, 40, 50, COLORED);
  epd.setFrameMemory(paint.getImage(), 16, 130, paint.getWidth(), paint.getHeight());

  paint.clear(UNCOLORED);
  paint.drawFilledCircle(32, 32, 30, COLORED);
  epd.setFrameMemory(paint.getImage(), 120, 130, paint.getWidth(), paint.getHeight());
  epd.displayFrame();

  delay(2000);

  if (epd.init(lutPartialUpdate) != 0) {
    Serial.print("e-Paper init failed");
    return;
  }

  /**
   *  there are 2 memory areas embedded in the e-paper display
   *  and once the display is refreshed, the memory area will be auto-toggled,
   *  i.e. the next action of SetFrameMemory will set the other memory area
   *  therefore you have to set the frame memory and refresh the display twice.
   */
  epd.setFrameMemory(IMAGE_DATA);
  epd.displayFrame();
  epd.setFrameMemory(IMAGE_DATA);
  epd.displayFrame();

  timeStartMs = millis();
}

void loop() {
  // put your main code here, to run repeatedly:
  timeNowS = (millis() - timeStartMs) / 1000;
  if (timeNowS != timeShowedS) {
    timeShowedS = timeNowS;
    updateTime(timeShowedS);
  }
  delay(20);
}

void updateTime(unsigned long seconds) {
  char time_string[] = {'0', '0', ':', '0', '0', '\0'};
  time_string[0] = seconds / 60 / 10 + '0';
  time_string[1] = seconds / 60 % 10 + '0';
  time_string[3] = seconds % 60 / 10 + '0';
  time_string[4] = seconds % 60 % 10 + '0';

  paint.setWidth(32);
  paint.setHeight(96);
  paint.setRotate(ROTATE_270);

  paint.clear(UNCOLORED);
  paint.drawStringAt(0, 4, time_string, &Font24, COLORED);
  epd.setFrameMemory(paint.getImage(), 80, 72, paint.getWidth(), paint.getHeight());
  epd.displayFrame();
}
