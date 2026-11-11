// Host-test shim for the Arduino core. NOT a board build.
// Emulates the AVR Harvard model: PROGMEM objects live in a dedicated section
// whose pages are made inaccessible; pgm_read_*() reads a shadow copy. A
// direct dereference of PROGMEM data therefore crashes (like garbage on AVR),
// and a pgm_read_*() on a RAM address is counted as a violation.
#pragma once
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>

#define PROGMEM __attribute__((section("progmem_data")))
extern "C" { extern const char __start_progmem_data[]; extern const char __stop_progmem_data[]; }

extern unsigned long g_millis;
extern unsigned long g_ramPgmReads;
extern const uint8_t* g_shadow;
const uint8_t* shim_flash_read(const void* a, size_t n);

inline unsigned long millis() { return g_millis; }
inline unsigned long micros() { return g_millis * 1000UL; }
inline void delay(unsigned long ms) { g_millis += ms; }

class __FlashStringHelper;
#define F(x) (reinterpret_cast<const __FlashStringHelper*>(x))

inline uint8_t pgm_read_byte_impl(const void* a) { return *shim_flash_read(a, 1); }
inline uint16_t pgm_read_word_impl(const void* a) { uint16_t v; memcpy(&v, shim_flash_read(a, 2), 2); return v; }
inline void* pgm_read_ptr_impl(const void* a) { void* v; memcpy(&v, shim_flash_read(a, sizeof(void*)), sizeof(void*)); return v; }
#define pgm_read_byte(a) pgm_read_byte_impl((const void*)(a))
#define pgm_read_word(a) pgm_read_word_impl((const void*)(a))
#define pgm_read_ptr(a)  pgm_read_ptr_impl((const void*)(a))
inline int strcmp_P(const char* ram, const char* flash) {
  for (size_t i = 0;; ++i) {
    unsigned char b = pgm_read_byte(flash + i);
    if ((unsigned char)ram[i] != b) return (unsigned char)ram[i] < b ? -1 : 1;
    if (b == 0) return 0;
  }
}
