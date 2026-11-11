/**
 * @file Dumirror_Storage.h
 * @brief Architecture-aware accessors for data stored in flash (PROGMEM).
 *
 * On AVR, flash and RAM are separate address spaces: PROGMEM data must be
 * read with pgm_read_*(). ESP8266 flash data also needs aligned reads. On
 * ESP32 / RP2040 / most ARM cores pgm_read_* is a plain load, so the same
 * code path is correct everywhere and no per-architecture #if is needed
 * beyond providing the macros below.
 *
 * Limitation: AVR "near" pgm_read_* addresses cover the first 64 KB of
 * flash. The 33,792 bytes of artwork sit below that on every supported
 * AVR (the linker places .progmem.data first).
 */
#ifndef DUMIRROR_STORAGE_H
#define DUMIRROR_STORAGE_H

#include <Arduino.h>
#include <string.h>
#ifndef PROGMEM
  #define PROGMEM
#endif
#ifndef pgm_read_byte
  #define pgm_read_byte(addr) (*(const uint8_t*)(addr))
#endif
#ifndef pgm_read_word
  #define pgm_read_word(addr) (*(const uint16_t*)(addr))
#endif
#ifndef pgm_read_ptr
  #define pgm_read_ptr(addr) (*(void* const*)(addr))
#endif

#include "../faces/animation_types.h"

namespace dumirror {

/** Bitmap pointer of frame `i` of `anim`. */
inline const uint8_t* frameBitmap(const Animation& anim, uint8_t i) {
  const Frame* f = anim.frames + i;
  return anim.inFlash ? static_cast<const uint8_t*>(pgm_read_ptr(&f->bitmap))
                      : f->bitmap;
}

/** Duration (ms) of frame `i` of `anim`. */
inline uint16_t frameDuration(const Animation& anim, uint8_t i) {
  const Frame* f = anim.frames + i;
  return anim.inFlash ? pgm_read_word(&f->durationMs) : f->durationMs;
}

/** Copy a built-in Animation descriptor stored in flash into RAM. */
inline Animation loadAnimationP(const Animation* p) {
  Animation a;
  a.frames  = static_cast<const Frame*>(pgm_read_ptr(&p->frames));
  a.count   = pgm_read_byte(&p->count);
  a.loop    = pgm_read_byte(&p->loop) != 0;
  a.inFlash = pgm_read_byte(&p->inFlash) != 0;
  return a;
}

} // namespace dumirror
#endif
