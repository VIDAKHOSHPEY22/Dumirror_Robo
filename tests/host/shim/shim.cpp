#include <stdlib.h>
#include "Arduino.h"
#include "Adafruit_SSD1306.h"
#include <sys/mman.h>
#include <unistd.h>
unsigned long g_millis = 0, g_ramPgmReads = 0;
const uint8_t* g_shadow = nullptr;
TwoWire Wire;
bool Adafruit_SSD1306::beginResult = true;
unsigned long Adafruit_SSD1306::displayCalls = 0, Adafruit_SSD1306::drawCalls = 0, Adafruit_SSD1306::clearCalls = 0;
const uint8_t* Adafruit_SSD1306::lastBitmap = nullptr;
char Adafruit_SSD1306::lastText[64]; int Adafruit_SSD1306::lastTextSize; unsigned Adafruit_SSD1306::lastColor;
int Adafruit_SSD1306::lastCursorX, Adafruit_SSD1306::lastCursorY; uint8_t Adafruit_SSD1306::lastAddr;
const uint8_t* shim_flash_read(const void* a, size_t n) {
  const char* p = (const char*)a;
  if (p >= __start_progmem_data && p + n <= __stop_progmem_data) return g_shadow + (p - __start_progmem_data);
  ++g_ramPgmReads;                      // pgm_read on a RAM address = bug on AVR
  return (const uint8_t*)a;
}
__attribute__((constructor(101))) static void protectFlash() {
  size_t len = __stop_progmem_data - __start_progmem_data;
  if (!len) return;
  uint8_t* sh = (uint8_t*)malloc(len); memcpy(sh, __start_progmem_data, len); g_shadow = sh;
  uintptr_t pg = sysconf(_SC_PAGESIZE);
  uintptr_t lo = ((uintptr_t)__start_progmem_data + pg - 1) & ~(pg - 1);
  uintptr_t hi = (uintptr_t)__stop_progmem_data & ~(pg - 1);
  if (hi > lo) mprotect((void*)lo, hi - lo, PROT_NONE);   // direct reads now crash
}
size_t shim_flash_bytes() { return __stop_progmem_data - __start_progmem_data; }
