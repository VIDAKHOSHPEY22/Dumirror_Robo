#pragma once
#include "Wire.h"
#define SSD1306_WHITE 1
#define SSD1306_BLACK 0
#define SSD1306_SWITCHCAPVCC 0x02
// Records what a real display would receive. drawBitmap reads through
// pgm_read_byte exactly like Adafruit_GFX does on AVR.
struct Adafruit_SSD1306 {
  static bool beginResult;
  static unsigned long displayCalls, drawCalls, clearCalls;
  static const uint8_t* lastBitmap;
  static char lastText[64]; static int lastTextSize; static unsigned lastColor;
  static int lastCursorX, lastCursorY; static uint8_t lastAddr;
  Adafruit_SSD1306(int, int, TwoWire*, int) {}
  bool begin(uint8_t, uint8_t addr) { lastAddr = addr; return beginResult; }
  void clearDisplay() { ++clearCalls; }
  void display() { ++displayCalls; }
  void drawBitmap(int, int, const uint8_t* b, int w, int h, unsigned) {
    ++drawCalls; lastBitmap = b;
    for (int i = 0; i < w * h / 8; ++i) (void)pgm_read_byte(b + i);
  }
  void setTextSize(uint8_t s) { lastTextSize = s; }
  void setTextColor(unsigned c) { lastColor = c; }
  void setTextWrap(bool) {}
  void setCursor(int x, int y) { lastCursorX = x; lastCursorY = y; }
  void print(const char* t) { strncpy(lastText, t, 63); lastText[63] = 0; }
  void getTextBounds(const char* t, int, int, int16_t* x1, int16_t* y1, uint16_t* w, uint16_t* h) {
    *x1 = 0; *y1 = 0; *w = (uint16_t)(strlen(t) * 6); *h = 8;
  }
};
