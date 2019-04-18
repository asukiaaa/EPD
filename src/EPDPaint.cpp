/**
 *  @filename   :   EPDPaint.cpp
 *  @brief      :   EPDPaint tools
 *  @author     :   Yehui from Waveshare
 *
 *  Copyright (C) Waveshare     September 9 2017
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

#if defined(__AVR__) || defined(ARDUINO_ARCH_SAMD)
#include <avr/pgmspace.h>
#elif defined(ESP8266) || defined(ESP32)
#include <pgmspace.h>
#endif
#include "EPDPaint.h"

EPDPaint::EPDPaint(unsigned char* image, int width, int height) {
  this->rotate = ROTATE_0;
  this->image = image;
  /* 1 byte = 8 pixels, so the width should be the multiple of 8 */
  this->width = width % 8 ? width + 8 - (width % 8) : width;
  this->height = height;
}

EPDPaint::~EPDPaint() {
}

/**
 *  @brief: clear the image
 */
void EPDPaint::clear(int colored) {
  for (int x = 0; x < this->width; x++) {
    for (int y = 0; y < this->height; y++) {
      drawAbsolutePixel(x, y, colored);
    }
  }
}

/**
 *  @brief: this draws a pixel by absolute coordinates.
 *          this function won't be affected by the rotate parameter.
 */
void EPDPaint::drawAbsolutePixel(int x, int y, int colored) {
  if (x < 0 || x >= this->width || y < 0 || y >= this->height) {
    return;
  }
  if (IF_INVERT_COLOR) {
    if (colored) {
      image[(x + y * this->width) / 8] |= 0x80 >> (x % 8);
    } else {
      image[(x + y * this->width) / 8] &= ~(0x80 >> (x % 8));
    }
  } else {
    if (colored) {
      image[(x + y * this->width) / 8] &= ~(0x80 >> (x % 8));
    } else {
      image[(x + y * this->width) / 8] |= 0x80 >> (x % 8);
    }
  }
}

/**
 *  @brief: Getters and Setters
 */
unsigned char* EPDPaint::getImage(void) {
  return this->image;
}

int EPDPaint::getWidth(void) {
  return this->width;
}

void EPDPaint::setWidth(int width) {
  this->width = width % 8 ? width + 8 - (width % 8) : width;
}

int EPDPaint::getHeight(void) {
  return this->height;
}

void EPDPaint::setHeight(int height) {
  this->height = height;
}

int EPDPaint::getRotate(void) {
  return this->rotate;
}

void EPDPaint::setRotate(int rotate){
  this->rotate = rotate;
}

/**
 *  @brief: this draws a pixel by the coordinates
 */
void EPDPaint::drawPixel(int x, int y, int colored) {
  int point_temp;
  if (this->rotate == ROTATE_0) {
    if(x < 0 || x >= this->width || y < 0 || y >= this->height) {
      return;
    }
    drawAbsolutePixel(x, y, colored);
  } else if (this->rotate == ROTATE_90) {
    if(x < 0 || x >= this->height || y < 0 || y >= this->width) {
      return;
    }
    point_temp = x;
    x = this->width - y;
    y = point_temp;
    drawAbsolutePixel(x, y, colored);
  } else if (this->rotate == ROTATE_180) {
    if(x < 0 || x >= this->width || y < 0 || y >= this->height) {
      return;
    }
    x = this->width - x;
    y = this->height - y;
    drawAbsolutePixel(x, y, colored);
  } else if (this->rotate == ROTATE_270) {
    if(x < 0 || x >= this->height || y < 0 || y >= this->width) {
      return;
    }
    point_temp = x;
    x = y;
    y = this->height - point_temp;
    drawAbsolutePixel(x, y, colored);
  }
}

/**
 *  @brief: this draws a charactor on the frame buffer but not refresh
 */
void EPDPaint::drawCharAt(int x, int y, char ascii_char, sFONT* font, int colored) {
  int i, j;
  unsigned int char_offset = (ascii_char - ' ') * font->Height * (font->Width / 8 + (font->Width % 8 ? 1 : 0));
  const unsigned char* ptr = &font->table[char_offset];

  for (j = 0; j < font->Height; j++) {
    for (i = 0; i < font->Width; i++) {
      if (pgm_read_byte(ptr) & (0x80 >> (i % 8))) {
        drawPixel(x + i, y + j, colored);
      }
      if (i % 8 == 7) {
        ptr++;
      }
    }
    if (font->Width % 8 != 0) {
      ptr++;
    }
  }
}

/**
 *  @brief: this displays a string on the frame buffer but not refresh
 */
void EPDPaint::drawStringAt(int x, int y, const char* text, sFONT* font, int colored) {
  const char* p_text = text;
  unsigned int counter = 0;
  int refcolumn = x;

  /* Send the string character by character on EPD */
  while (*p_text != 0) {
    /* Display one character on EPD */
    drawCharAt(refcolumn, y, *p_text, font, colored);
    /* Decrement the column position by 16 */
    refcolumn += font->Width;
    /* Point on the next character */
    p_text++;
    counter++;
  }
}

/**
 *  @brief: this draws a line on the frame buffer
 */
void EPDPaint::drawLine(int x0, int y0, int x1, int y1, int colored) {
  /* Bresenham algorithm */
  int dx = x1 - x0 >= 0 ? x1 - x0 : x0 - x1;
  int sx = x0 < x1 ? 1 : -1;
  int dy = y1 - y0 <= 0 ? y1 - y0 : y0 - y1;
  int sy = y0 < y1 ? 1 : -1;
  int err = dx + dy;

  while((x0 != x1) && (y0 != y1)) {
    drawPixel(x0, y0 , colored);
    if (2 * err >= dy) {
      err += dy;
      x0 += sx;
    }
    if (2 * err <= dx) {
      err += dx;
      y0 += sy;
    }
  }
}

/**
 *  @brief: this draws a horizontal line on the frame buffer
 */
void EPDPaint::drawHorizontalLine(int x, int y, int line_width, int colored) {
  int i;
  for (i = x; i < x + line_width; i++) {
    drawPixel(i, y, colored);
  }
}

/**
 *  @brief: this draws a vertical line on the frame buffer
 */
void EPDPaint::drawVerticalLine(int x, int y, int line_height, int colored) {
  int i;
  for (i = y; i < y + line_height; i++) {
    drawPixel(x, i, colored);
  }
}

/**
 *  @brief: this draws a rectangle
 */
void EPDPaint::drawRectangle(int x0, int y0, int x1, int y1, int colored) {
  int min_x, min_y, max_x, max_y;
  min_x = x1 > x0 ? x0 : x1;
  max_x = x1 > x0 ? x1 : x0;
  min_y = y1 > y0 ? y0 : y1;
  max_y = y1 > y0 ? y1 : y0;

  drawHorizontalLine(min_x, min_y, max_x - min_x + 1, colored);
  drawHorizontalLine(min_x, max_y, max_x - min_x + 1, colored);
  drawVerticalLine(min_x, min_y, max_y - min_y + 1, colored);
  drawVerticalLine(max_x, min_y, max_y - min_y + 1, colored);
}

/**
 *  @brief: this draws a filled rectangle
 */
void EPDPaint::drawFilledRectangle(int x0, int y0, int x1, int y1, int colored) {
  int min_x, min_y, max_x, max_y;
  int i;
  min_x = x1 > x0 ? x0 : x1;
  max_x = x1 > x0 ? x1 : x0;
  min_y = y1 > y0 ? y0 : y1;
  max_y = y1 > y0 ? y1 : y0;

  for (i = min_x; i <= max_x; i++) {
    drawVerticalLine(i, min_y, max_y - min_y + 1, colored);
  }
}

/**
 *  @brief: this draws a circle
 */
void EPDPaint::drawCircle(int x, int y, int radius, int colored) {
  /* Bresenham algorithm */
  int x_pos = -radius;
  int y_pos = 0;
  int err = 2 - 2 * radius;
  int e2;

  do {
    drawPixel(x - x_pos, y + y_pos, colored);
    drawPixel(x + x_pos, y + y_pos, colored);
    drawPixel(x + x_pos, y - y_pos, colored);
    drawPixel(x - x_pos, y - y_pos, colored);
    e2 = err;
    if (e2 <= y_pos) {
      err += ++y_pos * 2 + 1;
      if(-x_pos == y_pos && e2 <= x_pos) {
        e2 = 0;
      }
    }
    if (e2 > x_pos) {
      err += ++x_pos * 2 + 1;
    }
  } while (x_pos <= 0);
}

/**
 *  @brief: this draws a filled circle
 */
void EPDPaint::drawFilledCircle(int x, int y, int radius, int colored) {
  /* Bresenham algorithm */
  int x_pos = -radius;
  int y_pos = 0;
  int err = 2 - 2 * radius;
  int e2;

  do {
    drawPixel(x - x_pos, y + y_pos, colored);
    drawPixel(x + x_pos, y + y_pos, colored);
    drawPixel(x + x_pos, y - y_pos, colored);
    drawPixel(x - x_pos, y - y_pos, colored);
    drawHorizontalLine(x + x_pos, y + y_pos, 2 * (-x_pos) + 1, colored);
    drawHorizontalLine(x + x_pos, y - y_pos, 2 * (-x_pos) + 1, colored);
    e2 = err;
    if (e2 <= y_pos) {
      err += ++y_pos * 2 + 1;
      if(-x_pos == y_pos && e2 <= x_pos) {
        e2 = 0;
      }
    }
    if(e2 > x_pos) {
      err += ++x_pos * 2 + 1;
    }
  } while(x_pos <= 0);
}

/* END OF FILE */
