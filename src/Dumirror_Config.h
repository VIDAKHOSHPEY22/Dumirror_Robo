/**
 * @file Dumirror_Config.h
 * @brief Compile-time configuration: face profiles and feature switches.
 *
 * WHY THIS EXISTS
 * The complete artwork set is 33 unique 1024-byte bitmaps (33,792 bytes of
 * flash). That is more than the whole flash of an Arduino Uno / Nano /
 * Leonardo (32 KB), so small AVR boards must build a subset of the faces.
 *
 * PROFILES (choose at most one)
 *   DUMIRROR_PROFILE_FULL     all 19 static faces + 10 animations
 *   DUMIRROR_PROFILE_COMPACT  13 faces: everything the TouchPet example uses
 *   DUMIRROR_PROFILE_MINIMAL  12 faces: COMPACT without "sneeze" (-4 KB)
 *
 * Measured size of the face data (PROGMEM) per profile, from the host test
 * build: FULL 35,200 B, COMPACT 18,080 B, MINIMAL ~14 KB. Adafruit GFX +
 * SSD1306 + Wire add roughly 8-10 KB of code on top (not measured here).
 *
 * If none is defined: AVR with <=32 KB flash (Uno, Nano, Leonardo) selects
 * MINIMAL, AVR 48-64 KB and megaAVR select COMPACT, everything else FULL.
 *
 * HOW TO SELECT
 * The library is compiled separately from your sketch, so a #define inside
 * the sketch is NOT seen by the library. Use one of:
 *   - build flags:   -DDUMIRROR_PROFILE_MINIMAL
 *                    -DDUMIRROR_ENABLE_FACE_SAD=1
 *     (PlatformIO build_flags, arduino-cli --build-property, ...)
 *   - or edit this file directly.
 * begin() returns false with DUMIRROR_ERR_CONFIG_MISMATCH if the sketch and
 * the library were built with a different number of faces.
 *
 * Per-face override: DUMIRROR_ENABLE_FACE_<NAME> = 0 or 1, for example
 * DUMIRROR_ENABLE_FACE_HICCUP. Animations pull in the bitmaps they need
 * automatically.
 */
#ifndef DUMIRROR_CONFIG_H
#define DUMIRROR_CONFIG_H

#include <Arduino.h>

#if (defined(DUMIRROR_PROFILE_FULL) + defined(DUMIRROR_PROFILE_COMPACT) + defined(DUMIRROR_PROFILE_MINIMAL)) > 1
#error "Define at most one DUMIRROR_PROFILE_* macro."
#endif

#if !defined(DUMIRROR_PROFILE_FULL) && !defined(DUMIRROR_PROFILE_COMPACT) && !defined(DUMIRROR_PROFILE_MINIMAL)
  #if defined(ARDUINO_ARCH_AVR) && defined(FLASHEND) && (FLASHEND <= 0x7FFFUL)
    #define DUMIRROR_PROFILE_MINIMAL          // 32 KB parts: Uno, Nano, Leonardo
  #elif (defined(ARDUINO_ARCH_AVR) && defined(FLASHEND) && (FLASHEND <= 0xFFFFUL)) || defined(ARDUINO_ARCH_MEGAAVR)
    #define DUMIRROR_PROFILE_COMPACT          // 48-64 KB parts (e.g. Nano Every)
  #else
    #define DUMIRROR_PROFILE_FULL
  #endif
#endif

#ifndef DUMIRROR_ENABLE_FACE_NORMAL
  #if defined(DUMIRROR_PROFILE_FULL) || defined(DUMIRROR_PROFILE_COMPACT) || defined(DUMIRROR_PROFILE_MINIMAL)
    #define DUMIRROR_ENABLE_FACE_NORMAL 1
  #else
    #define DUMIRROR_ENABLE_FACE_NORMAL 0
  #endif
#endif
#ifndef DUMIRROR_ENABLE_FACE_POOKER
  #if defined(DUMIRROR_PROFILE_FULL)
    #define DUMIRROR_ENABLE_FACE_POOKER 1
  #else
    #define DUMIRROR_ENABLE_FACE_POOKER 0
  #endif
#endif
#ifndef DUMIRROR_ENABLE_FACE_LOVE
  #if defined(DUMIRROR_PROFILE_FULL) || defined(DUMIRROR_PROFILE_COMPACT) || defined(DUMIRROR_PROFILE_MINIMAL)
    #define DUMIRROR_ENABLE_FACE_LOVE 1
  #else
    #define DUMIRROR_ENABLE_FACE_LOVE 0
  #endif
#endif
#ifndef DUMIRROR_ENABLE_FACE_TOUCH
  #if defined(DUMIRROR_PROFILE_FULL) || defined(DUMIRROR_PROFILE_COMPACT) || defined(DUMIRROR_PROFILE_MINIMAL)
    #define DUMIRROR_ENABLE_FACE_TOUCH 1
  #else
    #define DUMIRROR_ENABLE_FACE_TOUCH 0
  #endif
#endif
#ifndef DUMIRROR_ENABLE_FACE_CRY
  #if defined(DUMIRROR_PROFILE_FULL)
    #define DUMIRROR_ENABLE_FACE_CRY 1
  #else
    #define DUMIRROR_ENABLE_FACE_CRY 0
  #endif
#endif
#ifndef DUMIRROR_ENABLE_FACE_DIZZY
  #if defined(DUMIRROR_PROFILE_FULL) || defined(DUMIRROR_PROFILE_COMPACT) || defined(DUMIRROR_PROFILE_MINIMAL)
    #define DUMIRROR_ENABLE_FACE_DIZZY 1
  #else
    #define DUMIRROR_ENABLE_FACE_DIZZY 0
  #endif
#endif
#ifndef DUMIRROR_ENABLE_FACE_KISS
  #if defined(DUMIRROR_PROFILE_FULL)
    #define DUMIRROR_ENABLE_FACE_KISS 1
  #else
    #define DUMIRROR_ENABLE_FACE_KISS 0
  #endif
#endif
#ifndef DUMIRROR_ENABLE_FACE_WARM
  #if defined(DUMIRROR_PROFILE_FULL)
    #define DUMIRROR_ENABLE_FACE_WARM 1
  #else
    #define DUMIRROR_ENABLE_FACE_WARM 0
  #endif
#endif
#ifndef DUMIRROR_ENABLE_FACE_HOT
  #if defined(DUMIRROR_PROFILE_FULL)
    #define DUMIRROR_ENABLE_FACE_HOT 1
  #else
    #define DUMIRROR_ENABLE_FACE_HOT 0
  #endif
#endif
#ifndef DUMIRROR_ENABLE_FACE_CHILLY
  #if defined(DUMIRROR_PROFILE_FULL)
    #define DUMIRROR_ENABLE_FACE_CHILLY 1
  #else
    #define DUMIRROR_ENABLE_FACE_CHILLY 0
  #endif
#endif
#ifndef DUMIRROR_ENABLE_FACE_COLD
  #if defined(DUMIRROR_PROFILE_FULL)
    #define DUMIRROR_ENABLE_FACE_COLD 1
  #else
    #define DUMIRROR_ENABLE_FACE_COLD 0
  #endif
#endif
#ifndef DUMIRROR_ENABLE_FACE_ANGRY
  #if defined(DUMIRROR_PROFILE_FULL)
    #define DUMIRROR_ENABLE_FACE_ANGRY 1
  #else
    #define DUMIRROR_ENABLE_FACE_ANGRY 0
  #endif
#endif
#ifndef DUMIRROR_ENABLE_FACE_HUH
  #if defined(DUMIRROR_PROFILE_FULL) || defined(DUMIRROR_PROFILE_COMPACT) || defined(DUMIRROR_PROFILE_MINIMAL)
    #define DUMIRROR_ENABLE_FACE_HUH 1
  #else
    #define DUMIRROR_ENABLE_FACE_HUH 0
  #endif
#endif
#ifndef DUMIRROR_ENABLE_FACE_LAUGH
  #if defined(DUMIRROR_PROFILE_FULL)
    #define DUMIRROR_ENABLE_FACE_LAUGH 1
  #else
    #define DUMIRROR_ENABLE_FACE_LAUGH 0
  #endif
#endif
#ifndef DUMIRROR_ENABLE_FACE_SLEEPY
  #if defined(DUMIRROR_PROFILE_FULL) || defined(DUMIRROR_PROFILE_COMPACT) || defined(DUMIRROR_PROFILE_MINIMAL)
    #define DUMIRROR_ENABLE_FACE_SLEEPY 1
  #else
    #define DUMIRROR_ENABLE_FACE_SLEEPY 0
  #endif
#endif
#ifndef DUMIRROR_ENABLE_FACE_DEAD
  #if defined(DUMIRROR_PROFILE_FULL) || defined(DUMIRROR_PROFILE_COMPACT) || defined(DUMIRROR_PROFILE_MINIMAL)
    #define DUMIRROR_ENABLE_FACE_DEAD 1
  #else
    #define DUMIRROR_ENABLE_FACE_DEAD 0
  #endif
#endif
#ifndef DUMIRROR_ENABLE_FACE_WINK
  #if defined(DUMIRROR_PROFILE_FULL) || defined(DUMIRROR_PROFILE_COMPACT) || defined(DUMIRROR_PROFILE_MINIMAL)
    #define DUMIRROR_ENABLE_FACE_WINK 1
  #else
    #define DUMIRROR_ENABLE_FACE_WINK 0
  #endif
#endif
#ifndef DUMIRROR_ENABLE_FACE_LOL
  #if defined(DUMIRROR_PROFILE_FULL)
    #define DUMIRROR_ENABLE_FACE_LOL 1
  #else
    #define DUMIRROR_ENABLE_FACE_LOL 0
  #endif
#endif
#ifndef DUMIRROR_ENABLE_FACE_SAD
  #if defined(DUMIRROR_PROFILE_FULL)
    #define DUMIRROR_ENABLE_FACE_SAD 1
  #else
    #define DUMIRROR_ENABLE_FACE_SAD 0
  #endif
#endif
#ifndef DUMIRROR_ENABLE_FACE_SNEEZE
  #if defined(DUMIRROR_PROFILE_FULL) || defined(DUMIRROR_PROFILE_COMPACT)
    #define DUMIRROR_ENABLE_FACE_SNEEZE 1
  #else
    #define DUMIRROR_ENABLE_FACE_SNEEZE 0
  #endif
#endif
#ifndef DUMIRROR_ENABLE_FACE_BLINK
  #if defined(DUMIRROR_PROFILE_FULL) || defined(DUMIRROR_PROFILE_COMPACT) || defined(DUMIRROR_PROFILE_MINIMAL)
    #define DUMIRROR_ENABLE_FACE_BLINK 1
  #else
    #define DUMIRROR_ENABLE_FACE_BLINK 0
  #endif
#endif
#ifndef DUMIRROR_ENABLE_FACE_LOOK_LEFT
  #if defined(DUMIRROR_PROFILE_FULL)
    #define DUMIRROR_ENABLE_FACE_LOOK_LEFT 1
  #else
    #define DUMIRROR_ENABLE_FACE_LOOK_LEFT 0
  #endif
#endif
#ifndef DUMIRROR_ENABLE_FACE_LOOK_RIGHT
  #if defined(DUMIRROR_PROFILE_FULL)
    #define DUMIRROR_ENABLE_FACE_LOOK_RIGHT 1
  #else
    #define DUMIRROR_ENABLE_FACE_LOOK_RIGHT 0
  #endif
#endif
#ifndef DUMIRROR_ENABLE_FACE_SMILE_BIG
  #if defined(DUMIRROR_PROFILE_FULL)
    #define DUMIRROR_ENABLE_FACE_SMILE_BIG 1
  #else
    #define DUMIRROR_ENABLE_FACE_SMILE_BIG 0
  #endif
#endif
#ifndef DUMIRROR_ENABLE_FACE_HEARTBEAT
  #if defined(DUMIRROR_PROFILE_FULL) || defined(DUMIRROR_PROFILE_COMPACT) || defined(DUMIRROR_PROFILE_MINIMAL)
    #define DUMIRROR_ENABLE_FACE_HEARTBEAT 1
  #else
    #define DUMIRROR_ENABLE_FACE_HEARTBEAT 0
  #endif
#endif
#ifndef DUMIRROR_ENABLE_FACE_SLEEP_BREATH
  #if defined(DUMIRROR_PROFILE_FULL) || defined(DUMIRROR_PROFILE_COMPACT) || defined(DUMIRROR_PROFILE_MINIMAL)
    #define DUMIRROR_ENABLE_FACE_SLEEP_BREATH 1
  #else
    #define DUMIRROR_ENABLE_FACE_SLEEP_BREATH 0
  #endif
#endif
#ifndef DUMIRROR_ENABLE_FACE_WAKE_UP
  #if defined(DUMIRROR_PROFILE_FULL) || defined(DUMIRROR_PROFILE_COMPACT) || defined(DUMIRROR_PROFILE_MINIMAL)
    #define DUMIRROR_ENABLE_FACE_WAKE_UP 1
  #else
    #define DUMIRROR_ENABLE_FACE_WAKE_UP 0
  #endif
#endif
#ifndef DUMIRROR_ENABLE_FACE_HICCUP
  #if defined(DUMIRROR_PROFILE_FULL)
    #define DUMIRROR_ENABLE_FACE_HICCUP 1
  #else
    #define DUMIRROR_ENABLE_FACE_HICCUP 0
  #endif
#endif
#ifndef DUMIRROR_ENABLE_FACE_CHEW
  #if defined(DUMIRROR_PROFILE_FULL)
    #define DUMIRROR_ENABLE_FACE_CHEW 1
  #else
    #define DUMIRROR_ENABLE_FACE_CHEW 0
  #endif
#endif

// Bitmaps required by animations (their frames alias these static faces).
// Used by the implementation to decide which face headers to compile.
#define DUMIRROR_NEED_NORMAL (DUMIRROR_ENABLE_FACE_NORMAL || DUMIRROR_ENABLE_FACE_SNEEZE || DUMIRROR_ENABLE_FACE_BLINK || \
  DUMIRROR_ENABLE_FACE_LOOK_LEFT || DUMIRROR_ENABLE_FACE_LOOK_RIGHT || DUMIRROR_ENABLE_FACE_SMILE_BIG || \
  DUMIRROR_ENABLE_FACE_WAKE_UP || DUMIRROR_ENABLE_FACE_HICCUP || DUMIRROR_ENABLE_FACE_CHEW)
#define DUMIRROR_NEED_LOVE   (DUMIRROR_ENABLE_FACE_LOVE || DUMIRROR_ENABLE_FACE_HEARTBEAT || DUMIRROR_ENABLE_FACE_SMILE_BIG)
#define DUMIRROR_NEED_SLEEPY (DUMIRROR_ENABLE_FACE_SLEEPY || DUMIRROR_ENABLE_FACE_SLEEP_BREATH || DUMIRROR_ENABLE_FACE_WAKE_UP)

/** @brief "Use the board's default I2C pin" marker for the constructor. */
#define DUMIRROR_PIN_DEFAULT 0xFF

#endif // DUMIRROR_CONFIG_H
