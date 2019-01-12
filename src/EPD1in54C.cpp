/**
 *  @filename   :   EPD1in54C.cpp
 *  @brief      :   Implements for e-paper library
 *  @author     :   Yehui from Waveshare
 *
 *  Copyright (C) Waveshare     August 10 2017
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
#include "EPD1in54C.h"

const unsigned char lutVcom0[] =
  {
    0x0E, 0x14, 0x01, 0x0A, 0x06, 0x04, 0x0A, 0x0A,
    0x0F, 0x03, 0x03, 0x0C, 0x06, 0x0A, 0x00
  };

const unsigned char lutW[] =
  {
    0x0E, 0x14, 0x01, 0x0A, 0x46, 0x04, 0x8A, 0x4A,
    0x0F, 0x83, 0x43, 0x0C, 0x86, 0x0A, 0x04
  };

const unsigned char lutB[] =
  {
    0x0E, 0x14, 0x01, 0x8A, 0x06, 0x04, 0x8A, 0x4A,
    0x0F, 0x83, 0x43, 0x0C, 0x06, 0x4A, 0x04
  };

const unsigned char lutG1[] =
  {
    0x8E, 0x94, 0x01, 0x8A, 0x06, 0x04, 0x8A, 0x4A,
    0x0F, 0x83, 0x43, 0x0C, 0x06, 0x0A, 0x04
  };

const unsigned char lutG2[] =
  {
    0x8E, 0x94, 0x01, 0x8A, 0x06, 0x04, 0x8A, 0x4A,
    0x0F, 0x83, 0x43, 0x0C, 0x06, 0x0A, 0x04
  };

const unsigned char lutVcom1[] =
  {
    0x03, 0x1D, 0x01, 0x01, 0x08, 0x23, 0x37, 0x37,
    0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
  };

const unsigned char lutRed0[] =
  {
    0x83, 0x5D, 0x01, 0x81, 0x48, 0x23, 0x77, 0x77,
    0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
  };

const unsigned char lutRed1[] =
  {
    0x03, 0x1D, 0x01, 0x01, 0x08, 0x23, 0x37, 0x37,
    0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
  };

EPD1in54C::~EPD1in54C() {
};

EPD1in54C::EPD1in54C(unsigned int reset, unsigned int dc, unsigned int cs, unsigned int busy)
  : EPDIF(reset, dc, cs, busy) {
  width = EPD_WIDTH;
  height = EPD_HEIGHT;
};

int EPD1in54C::init(void) {
  /* this calls the peripheral hardware interface, see epdif */
  if (ifInit() != 0) {
    return -1;
  }
  /* EPD hardware init start */
  reset();
  sendCommand(POWER_SETTING);
  sendData(0x07);
  sendData(0x00);
  sendData(0x08);
  sendData(0x00);
  sendCommand(BOOSTER_SOFT_START);
  sendData(0x17);
  sendData(0x17);
  sendData(0x17);
  sendCommand(POWER_ON);

  waitUntilIdle();

  sendCommand(PANEL_SETTING);
  sendData(0x0f);
  sendData(0x0d);
  sendCommand(VCOM_AND_DATA_INTERVAL_SETTING);
  sendData(0xF7);
  //    SendCommand(PLL_CONTROL);
  //    SendData(0x39);
  sendCommand(TCON_RESOLUTION);
  sendData(0x98);
  sendData(0x00);
  sendData(0x98);
  sendCommand(VCM_DC_SETTING_REGISTER);
  sendData(0xf7);

  setLutBw();
  setLutRed();
  /* EPD hardware init end */

  return 0;
}

/**
 *  @brief: basic function for sending commands
 */
void EPD1in54C::sendCommand(unsigned char command) {
  digitalWrite(dcPin, LOW);
  spiTransfer(command);
}

/**
 *  @brief: basic function for sending data
 */
void EPD1in54C::sendData(unsigned char data) {
  digitalWrite(dcPin, HIGH);
  spiTransfer(data);
}

/**
 *  @brief: Wait until the busyPin goes HIGH
 */
void EPD1in54C::waitUntilIdle(void) {
  while(digitalRead(busyPin) == 0) {      //0: busy, 1: idle
    delayMs(100);
  }
}

/**
 *  @brief: module reset.
 *          often used to awaken the module in deep sleep,
 *          see EPD1in54C::sleep();
 */
void EPD1in54C::reset(void) {
  digitalWrite(resetPin, LOW);                //module reset
  delayMs(200);
  digitalWrite(resetPin, HIGH);
  delayMs(200);
}

/**
 *  @brief: set the look-up tables
 */
void EPD1in54C::setLutBw(void) {
  unsigned int count;
  sendCommand(0x20);         //g vcom
  for(count = 0; count < 15; count++) {
    sendData(lutVcom0[count]);
  }
  sendCommand(0x21);        //g ww --
  for(count = 0; count < 15; count++) {
    sendData(lutW[count]);
  }
  sendCommand(0x22);         //g bw r
  for(count = 0; count < 15; count++) {
    sendData(lutB[count]);
  }
  sendCommand(0x23);         //g wb w
  for(count = 0; count < 15; count++) {
    sendData(lutG1[count]);
  }
  sendCommand(0x24);         //g bb b
  for(count = 0; count < 15; count++) {
    sendData(lutG2[count]);
  }
}

void EPD1in54C::setLutRed(void) {
  unsigned int count;
  sendCommand(0x25);
  for(count = 0; count < 15; count++) {
    sendData(lutVcom1[count]);
  }
  sendCommand(0x26);
  for(count = 0; count < 15; count++) {
    sendData(lutRed0[count]);
  }
  sendCommand(0x27);
  for(count = 0; count < 15; count++) {
    sendData(lutRed1[count]);
  }
}

void EPD1in54C::displayFrame(const unsigned char* frame_buffer_black, const unsigned char* frame_buffer_red) {
  if (frame_buffer_black != NULL) {
    sendCommand(DATA_START_TRANSMISSION_1);
    delayMs(2);
    for (int i = 0; i < this->width * this->height / 8; i++) {
      sendData(pgm_read_byte(&frame_buffer_black[i]));
    }
    delayMs(2);
  }
  if (frame_buffer_red != NULL) {
    sendCommand(DATA_START_TRANSMISSION_2);
    delayMs(2);
    for (int i = 0; i < this->width * this->height / 8; i++) {
      sendData(pgm_read_byte(&frame_buffer_red[i]));
    }
    delayMs(2);
  }
  sendCommand(DISPLAY_REFRESH);
  waitUntilIdle();
}

/**
 *  @brief: After this command is transmitted, the chip would enter the
 *          deep-sleep mode to save power.
 *          The deep sleep mode would return to standby by hardware reset.
 *          The only one parameter is a check code, the command would be
 *          executed if check code = 0xA5.
 *          You can use EPD1in54C::init() to awaken
 */
void EPD1in54C::sleep() {
  sendCommand(VCOM_AND_DATA_INTERVAL_SETTING);
  sendData(0x17);
  sendCommand(VCM_DC_SETTING_REGISTER);         //to solve Vcom drop
  sendData(0x00);
  sendCommand(POWER_SETTING);         //power setting
  sendData(0x02);        //gate switch to external
  sendData(0x00);
  sendData(0x00);
  sendData(0x00);
  waitUntilIdle();
  sendCommand(POWER_OFF);         //power off
}

/* END OF FILE */
