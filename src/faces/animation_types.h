/**
 * @file animation_types.h
 * @brief Frame / Animation descriptors shared by all animated faces.
 */

#ifndef DUMIRROR_ANIMATION_TYPES_H
#define DUMIRROR_ANIMATION_TYPES_H

#include <Arduino.h>

/** @brief Size in bytes of one 128x64 1-bit frame (64 rows x 16 bytes). */
constexpr uint16_t DUMIRROR_BITMAP_BYTES = 1024;

/** @brief One animation frame: a bitmap and how long it stays on screen. */
struct Frame {
  const uint8_t* bitmap;     ///< 1024-byte bitmap, normally stored in PROGMEM.
  uint16_t       durationMs; ///< Time the frame stays visible.
};

/**
 * @brief A sequence of frames.
 *
 * `inFlash` tells the engine where the Frame array itself lives:
 *   false (default) - ordinary RAM / const data:   `const Frame f[] = {...};`
 *   true            - PROGMEM:  `const Frame f[] PROGMEM = {...};`
 * On AVR the two storage classes are different address spaces, so the flag
 * must match the declaration. On ESP32/RP2040 both are plain memory and
 * either works. The bitmaps are always read as PROGMEM data.
 *
 * Brace initialisation with three values stays valid: `{frames, n, loop}`
 * leaves inFlash = false.
 */
struct Animation {
  const Frame* frames;
  uint8_t      count;
  bool         loop;
  bool         inFlash;
};

#endif // DUMIRROR_ANIMATION_TYPES_H
