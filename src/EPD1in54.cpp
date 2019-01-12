/**
 *  @filename   :   EPD1in54.cpp
 *  @brief      :   Implements for e-paper library
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

#include <stdlib.h>
#include "EPD1in54.h"

EPD1in54::~EPD1in54() {
};

EPD1in54::EPD1in54(unsigned int reset, unsigned int dc, unsigned int cs, unsigned int busy)
  : EPDIF(reset, dc, cs, busy) {
  width = EPD_WIDTH;
  height = EPD_HEIGHT;
};

int EPD1in54::init(const unsigned char* lut) {
  /* this calls the peripheral hardware interface, see epdif */
  if (ifInit() != 0) {
    return -1;
  }
  /* EPD hardware init start */
  this->lut = lut;
  reset();
  sendCommand(DRIVER_OUTPUT_CONTROL);
  sendData((EPD_HEIGHT - 1) & 0xFF);
  sendData(((EPD_HEIGHT - 1) >> 8) & 0xFF);
  sendData(0x00);                     // GD = 0; SM = 0; TB = 0;
  sendCommand(BOOSTER_SOFT_START_CONTROL);
  sendData(0xD7);
  sendData(0xD6);
  sendData(0x9D);
  sendCommand(WRITE_VCOM_REGISTER);
  sendData(0xA8);                     // VCOM 7C
  sendCommand(SET_DUMMY_LINE_PERIOD);
  sendData(0x1A);                     // 4 dummy lines per gate
  sendCommand(SET_GATE_TIME);
  sendData(0x08);                     // 2us per line
  sendCommand(DATA_ENTRY_MODE_SETTING);
  sendData(0x03);                     // X increment; Y increment
  setLut(this->lut);
  /* EPD hardware init end */
  return 0;
}

/**
 *  @brief: basic function for sending commands
 */
void EPD1in54::sendCommand(unsigned char command) {
  digitalWrite(dcPin, LOW);
  spiTransfer(command);
}

/**
 *  @brief: basic function for sending data
 */
void EPD1in54::sendData(unsigned char data) {
  digitalWrite(dcPin, HIGH);
  spiTransfer(data);
}

/**
 *  @brief: Wait until the busyPin goes LOW
 */
void EPD1in54::waitUntilIdle(void) {
  while(digitalRead(busyPin) == HIGH) {      //LOW: idle, HIGH: busy
    delayMs(100);
  }
}

/**
 *  @brief: module reset.
 *          often used to awaken the module in deep sleep,
 *          see EPD1in54::sleep();
 */
void EPD1in54::reset(void) {
  digitalWrite(resetPin, LOW);                //module reset
  delayMs(200);
  digitalWrite(resetPin, HIGH);
  delayMs(200);
}

/**
 *  @brief: set the look-up table register
 */
void EPD1in54::setLut(const unsigned char* lut) {
  this->lut = lut;
  sendCommand(WRITE_LUT_REGISTER);
  /* the length of look-up table is 30 bytes */
  for (int i = 0; i < 30; i++) {
    sendData(this->lut[i]);
  }
}

/**
 *  @brief: put an image buffer to the frame memory.
 *          this won't update the display.
 */
void EPD1in54::setFrameMemory(
                              const unsigned char* image_buffer,
                              int x,
                              int y,
                              int image_width,
                              int image_height
                              ) {
  int x_end;
  int y_end;

  if (
      image_buffer == NULL ||
      x < 0 || image_width < 0 ||
      y < 0 || image_height < 0
      ) {
    return;
  }
  /* x point must be the multiple of 8 or the last 3 bits will be ignored */
  x &= 0xF8;
  image_width &= 0xF8;
  if (x + image_width >= this->width) {
    x_end = this->width - 1;
  } else {
    x_end = x + image_width - 1;
  }
  if (y + image_height >= this->height) {
    y_end = this->height - 1;
  } else {
    y_end = y + image_height - 1;
  }
  setMemoryArea(x, y, x_end, y_end);
  setMemoryPointer(x, y);
  sendCommand(WRITE_RAM);
  /* send the image data */
  for (int j = 0; j < y_end - y + 1; j++) {
    for (int i = 0; i < (x_end - x + 1) / 8; i++) {
      sendData(image_buffer[i + j * (image_width / 8)]);
    }
  }
}

/**
 *  @brief: put an image buffer to the frame memory.
 *          this won't update the display.
 *
 *          Question: When do you use this function instead of
 *          void setFrameMemory(
 *              const unsigned char* image_buffer,
 *              int x,
 *              int y,
 *              int image_width,
 *              int image_height
 *          );
 *          Answer: setFrameMemory with parameters only reads image data
 *          from the RAM but not from the flash in AVR chips (for AVR chips,
 *          you have to use the function pgm_read_byte to read buffers
 *          from the flash).
 */
void EPD1in54::setFrameMemory(const unsigned char* image_buffer) {
  setMemoryArea(0, 0, this->width - 1, this->height - 1);
  setMemoryPointer(0, 0);
  sendCommand(WRITE_RAM);
  /* send the image data */
  for (int i = 0; i < this->width / 8 * this->height; i++) {
    sendData(pgm_read_byte(&image_buffer[i]));
  }
}

/**
 *  @brief: clear the frame memory with the specified color.
 *          this won't update the display.
 */
void EPD1in54::clearFrameMemory(unsigned char color) {
  setMemoryArea(0, 0, this->width - 1, this->height - 1);
  setMemoryPointer(0, 0);
  sendCommand(WRITE_RAM);
  /* send the color data */
  for (int i = 0; i < this->width / 8 * this->height; i++) {
    sendData(color);
  }
}

/**
 *  @brief: update the display
 *          there are 2 memory areas embedded in the e-paper display
 *          but once this function is called,
 *          the the next action of setFrameMemory or clearFrame will
 *          set the other memory area.
 */
void EPD1in54::displayFrame(void) {
  sendCommand(DISPLAY_UPDATE_CONTROL_2);
  sendData(0xC4);
  sendCommand(MASTER_ACTIVATION);
  sendCommand(TERMINATE_FRAME_READ_WRITE);
  waitUntilIdle();
}

/**
 *  @brief: private function to specify the memory area for data R/W
 */
void EPD1in54::setMemoryArea(int x_start, int y_start, int x_end, int y_end) {
  sendCommand(SET_RAM_X_ADDRESS_START_END_POSITION);
  /* x point must be the multiple of 8 or the last 3 bits will be ignored */
  sendData((x_start >> 3) & 0xFF);
  sendData((x_end >> 3) & 0xFF);
  sendCommand(SET_RAM_Y_ADDRESS_START_END_POSITION);
  sendData(y_start & 0xFF);
  sendData((y_start >> 8) & 0xFF);
  sendData(y_end & 0xFF);
  sendData((y_end >> 8) & 0xFF);
}

/**
 *  @brief: private function to specify the start point for data R/W
 */
void EPD1in54::setMemoryPointer(int x, int y) {
  sendCommand(SET_RAM_X_ADDRESS_COUNTER);
  /* x point must be the multiple of 8 or the last 3 bits will be ignored */
  sendData((x >> 3) & 0xFF);
  sendCommand(SET_RAM_Y_ADDRESS_COUNTER);
  sendData(y & 0xFF);
  sendData((y >> 8) & 0xFF);
  waitUntilIdle();
}

/**
 *  @brief: After this command is transmitted, the chip would enter the
 *          deep-sleep mode to save power.
 *          The deep sleep mode would return to standby by hardware reset.
 *          You can use EPD1in54::init() to awaken
 */
void EPD1in54::sleep() {
  sendCommand(DEEP_SLEEP_MODE);
  waitUntilIdle();
}

const unsigned char lutFullUpdate[] =
  {
    0x02, 0x02, 0x01, 0x11, 0x12, 0x12, 0x22, 0x22,
    0x66, 0x69, 0x69, 0x59, 0x58, 0x99, 0x99, 0x88,
    0x00, 0x00, 0x00, 0x00, 0xF8, 0xB4, 0x13, 0x51,
    0x35, 0x51, 0x51, 0x19, 0x01, 0x00
  };

const unsigned char lutPartialUpdate[] =
  {
    0x10, 0x18, 0x18, 0x08, 0x18, 0x18, 0x08, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x13, 0x14, 0x44, 0x12,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00
  };

/* END OF FILE */
