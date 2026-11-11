/**
 * @file Dumirror_Robo.cpp
 * @brief Implementation of DumirrorRobo.
 * @version 1.1.0
 */

#include "Dumirror_Robo.h"
#include "internal/Dumirror_Storage.h"
#include <string.h>

// Bitmap and animation definitions are compiled once in this translation unit.
// Each header is compiled only when the active profile needs it.
#if DUMIRROR_NEED_NORMAL
#include "faces/face_normal.h"
#endif
#if DUMIRROR_ENABLE_FACE_POOKER
#include "faces/face_pooker.h"
#endif
#if DUMIRROR_NEED_LOVE
#include "faces/face_love.h"
#endif
#if DUMIRROR_ENABLE_FACE_TOUCH
#include "faces/face_touch.h"
#endif
#if DUMIRROR_ENABLE_FACE_CRY
#include "faces/face_cry.h"
#endif
#if DUMIRROR_ENABLE_FACE_DIZZY
#include "faces/face_dizzy.h"
#endif
#if DUMIRROR_ENABLE_FACE_KISS
#include "faces/face_kiss.h"
#endif
#if DUMIRROR_ENABLE_FACE_WARM
#include "faces/face_warm.h"
#endif
#if DUMIRROR_ENABLE_FACE_HOT
#include "faces/face_hot.h"
#endif
#if DUMIRROR_ENABLE_FACE_CHILLY
#include "faces/face_chilly.h"
#endif
#if DUMIRROR_ENABLE_FACE_COLD
#include "faces/face_cold.h"
#endif
#if DUMIRROR_ENABLE_FACE_ANGRY
#include "faces/face_angry.h"
#endif
#if DUMIRROR_ENABLE_FACE_HUH
#include "faces/face_huh.h"
#endif
#if DUMIRROR_ENABLE_FACE_LAUGH
#include "faces/face_laugh.h"
#endif
#if DUMIRROR_NEED_SLEEPY
#include "faces/face_sleepy.h"
#endif
#if DUMIRROR_ENABLE_FACE_DEAD
#include "faces/face_dead.h"
#endif
#if DUMIRROR_ENABLE_FACE_WINK
#include "faces/face_wink.h"
#endif
#if DUMIRROR_ENABLE_FACE_LOL
#include "faces/face_lol.h"
#endif
#if DUMIRROR_ENABLE_FACE_SAD
#include "faces/face_sad.h"
#endif
#if DUMIRROR_ENABLE_FACE_SNEEZE
#include "faces/anim_sneeze.h"
#endif
#if DUMIRROR_ENABLE_FACE_BLINK
#include "faces/anim_blink.h"
#endif
#if DUMIRROR_ENABLE_FACE_LOOK_LEFT
#include "faces/anim_look_left.h"
#endif
#if DUMIRROR_ENABLE_FACE_LOOK_RIGHT
#include "faces/anim_look_right.h"
#endif
#if DUMIRROR_ENABLE_FACE_SMILE_BIG
#include "faces/anim_smile_big.h"
#endif
#if DUMIRROR_ENABLE_FACE_HEARTBEAT
#include "faces/anim_heartbeat.h"
#endif
#if DUMIRROR_ENABLE_FACE_SLEEP_BREATH
#include "faces/anim_sleep_breath.h"
#endif
#if DUMIRROR_ENABLE_FACE_WAKE_UP
#include "faces/anim_wake_up.h"
#endif
#if DUMIRROR_ENABLE_FACE_HICCUP
#include "faces/anim_hiccup.h"
#endif
#if DUMIRROR_ENABLE_FACE_CHEW
#include "faces/anim_chew.h"
#endif

// ============================================================
// Flash-resident lookup tables
//
// Everything below is PROGMEM so that, on AVR, it costs no SRAM. It is read
// back with pgm_read_*() through dumirror:: helpers.
// ============================================================

// ---- Face names (one flash string per face) ----
#define X(bitmap, enumId, name)                                  static const char NAME_##enumId[] PROGMEM = name;
#include "faces.def"
#undef X
#define X_ANIM(tag, enumId, name, animArray, frameCount, loops)  static const char NAME_##enumId[] PROGMEM = name;
#include "faces.def"
#undef X_ANIM

static const char* const FACE_NAMES[] PROGMEM = {
#define X(bitmap, enumId, name) NAME_##enumId,
#include "faces.def"
#undef X
#define X_ANIM(tag, enumId, name, animArray, frameCount, loops) NAME_##enumId,
#include "faces.def"
#undef X_ANIM
};

// ---- Static bitmap table ----
static const uint8_t* const FACE_BITMAPS[] PROGMEM = {
#define X(bitmap, enumId, name) bitmap,
#include "faces.def"
#undef X
};

// ---- Animation descriptors (last entry is an unused sentinel so the array
//      is never empty when every animation is disabled) ----
static const Animation FACE_ANIMATIONS[] PROGMEM = {
#define X_ANIM(tag, enumId, name, animArray, frameCount, loops) \
  { animArray, frameCount, loops, true },
#include "faces.def"
#undef X_ANIM
  { nullptr, 0, false, false }
};

constexpr uint8_t ANIM_COUNT =
  (sizeof(FACE_ANIMATIONS) / sizeof(FACE_ANIMATIONS[0])) - 1;

// ============================================================
// Compile-time consistency checks
// ============================================================

static_assert(FACE_STATIC_COUNT > 0, "At least one static face must be enabled");

static_assert(
  sizeof(FACE_BITMAPS) / sizeof(FACE_BITMAPS[0]) == FACE_STATIC_COUNT,
  "faces.def mismatch: bitmap table size != FACE_STATIC_COUNT"
);

static_assert(
  sizeof(FACE_NAMES) / sizeof(FACE_NAMES[0]) == FACE_COUNT,
  "faces.def mismatch: name table size != FACE_COUNT"
);

static_assert(
  ANIM_COUNT == FACE_COUNT - FACE_STATIC_COUNT,
  "faces.def mismatch: animation table size != animated face count"
);

// Every static bitmap is exactly 1024 bytes.
#define X(bitmap, enumId, name) \
  static_assert(sizeof(bitmap) == DUMIRROR_BITMAP_BYTES, #bitmap " must be 1024 bytes");
#include "faces.def"
#undef X

// Animation frame counts and loop flags.
#define X_ANIM(tag, enumId, name, animArray, frameCount, loops) \
  static_assert(tag##_FRAMES == (frameCount), #tag ": frame count mismatch"); \
  static_assert(sizeof(animArray) / sizeof(Frame) == (frameCount), #tag ": descriptor size mismatch"); \
  static_assert((frameCount) > 0 && (frameCount) < 256, #tag ": frame count out of range"); \
  static_assert(tag##_LOOP == (loops), #tag ": loop flag mismatch");
#include "faces.def"
#undef X_ANIM

static_assert(FACE_COUNT < 255, "Too many faces for uint8_t ids");

// ============================================================
// Constructor
// ============================================================

DumirrorRobo::DumirrorRobo(uint8_t sda, uint8_t scl, uint8_t addr)
  : _display(DUMIRROR_WIDTH, DUMIRROR_HEIGHT, &Wire, -1),
    _addr(addr),
    _sda(sda),
    _scl(scl),
    _currentFace(0),
    _started(false),
    _error(DUMIRROR_OK),
    _textColor(SSD1306_WHITE),
    _lastUpdate(0),
    _anim{nullptr, 0, false, false},
    _animIndex(0),
    _animFrameStart(0),
    _animPlaying(false),
    _lastBitmap(nullptr) {
  _nameBuf[0] = '\0';
}

// ============================================================
// I2C initialisation (architecture-aware)
// ============================================================

void DumirrorRobo::_initI2C() {
#if defined(ARDUINO_ARCH_ESP32) || defined(ARDUINO_ARCH_ESP8266)
  // Runtime pin selection is supported. "Default" keeps the board's own
  // SDA/SCL so different ESP32 variants work without edits.
  if (_sda == DUMIRROR_PIN_DEFAULT || _scl == DUMIRROR_PIN_DEFAULT) {
    Wire.begin();
  } else {
    Wire.begin(_sda, _scl);
  }
#elif defined(ARDUINO_ARCH_RP2040) && !defined(ARDUINO_ARCH_MBED)
  // arduino-pico: pins must be set before begin().
  if (_sda != DUMIRROR_PIN_DEFAULT) Wire.setSDA(_sda);
  if (_scl != DUMIRROR_PIN_DEFAULT) Wire.setSCL(_scl);
  Wire.begin();
#else
  // AVR and other cores: I2C pins are fixed by the hardware.
  Wire.begin();
#endif
}

// ============================================================
// Initialise
// ============================================================

bool DumirrorRobo::_begin(uint8_t sketchFaceCount) {

  // The sketch and the library are compiled separately. If their face
  // profiles differ, enum ids would silently point to the wrong faces.
  if (sketchFaceCount != FACE_COUNT) {
    _error = DUMIRROR_ERR_CONFIG_MISMATCH;
    return false;
  }

  _initI2C();

  if (!_display.begin(SSD1306_SWITCHCAPVCC, _addr)) {
    _started = false;
    _error = DUMIRROR_ERR_DISPLAY_NOT_FOUND;
    return false;
  }

  _display.clearDisplay();
  _display.display();

  _started = true;
  _error = DUMIRROR_OK;
  _currentFace = 0;
  _lastUpdate = millis();
  _lastBitmap = nullptr;

  stopAnimation();
  return true;
}

// ============================================================
// Draw a bitmap (skips the I2C transfer if it is already on screen)
// ============================================================

void DumirrorRobo::_draw(const uint8_t* bitmap) {

  if (bitmap == nullptr || !_started) {
    return;
  }

  if (bitmap == _lastBitmap) {
    return;
  }

  _display.clearDisplay();
  _display.drawBitmap(0, 0, bitmap, DUMIRROR_WIDTH, DUMIRROR_HEIGHT, SSD1306_WHITE);
  _display.display();

  _lastBitmap = bitmap;
}

// ============================================================
// Lookup helpers
// ============================================================

const uint8_t* DumirrorRobo::getBitmap(uint8_t face) const {

  if (face >= FACE_COUNT) {
    return nullptr;
  }

  if (face < FACE_STATIC_COUNT) {
    return static_cast<const uint8_t*>(pgm_read_ptr(&FACE_BITMAPS[face]));
  }

  const Animation anim =
    dumirror::loadAnimationP(&FACE_ANIMATIONS[face - FACE_STATIC_COUNT]);

  if (anim.frames == nullptr || anim.count == 0) {
    return nullptr;
  }

  return dumirror::frameBitmap(anim, 0);
}

const __FlashStringHelper* DumirrorRobo::getNameP(uint8_t face) const {

  if (face >= FACE_COUNT) {
    return nullptr;
  }

  return reinterpret_cast<const __FlashStringHelper*>(
    pgm_read_ptr(&FACE_NAMES[face]));
}

const char* DumirrorRobo::getName(uint8_t face) const {

  if (face >= FACE_COUNT) {
    return nullptr;
  }

  const char* src = static_cast<const char*>(pgm_read_ptr(&FACE_NAMES[face]));

  uint8_t i = 0;
  for (; i < DUMIRROR_MAX_NAME_LEN; ++i) {
    const char c = static_cast<char>(pgm_read_byte(src + i));
    _nameBuf[i] = c;
    if (c == '\0') {
      return _nameBuf;
    }
  }
  _nameBuf[i] = '\0';
  return _nameBuf;
}

int16_t DumirrorRobo::faceIndex(const char* name) const {

  if (name == nullptr) {
    return -1;
  }

  for (uint8_t i = 0; i < FACE_COUNT; ++i) {
    const char* candidate =
      static_cast<const char*>(pgm_read_ptr(&FACE_NAMES[i]));
    if (strcmp_P(name, candidate) == 0) {
      return i;
    }
  }

  return -1;
}

// ============================================================
// Show a face
// ============================================================

bool DumirrorRobo::showFace(uint8_t face) {

  if (!_started) {
    _error = DUMIRROR_ERR_NOT_STARTED;
    return false;
  }

  if (face >= FACE_COUNT) {
    _error = DUMIRROR_ERR_INVALID_FACE;
    return false;
  }

  if (face >= FACE_STATIC_COUNT) {

    const Animation anim =
      dumirror::loadAnimationP(&FACE_ANIMATIONS[face - FACE_STATIC_COUNT]);

    if (anim.frames == nullptr || anim.count == 0) {
      _error = DUMIRROR_ERR_INVALID_ANIMATION;
      return false;
    }

    _currentFace = face;
    playAnimation(anim);

  } else {

    _currentFace = face;
    stopAnimation();
    _draw(getBitmap(face));
  }

  _error = DUMIRROR_OK;
  return true;
}

bool DumirrorRobo::showFaceByName(const char* name) {

  const int16_t index = faceIndex(name);

  if (index < 0) {
    _error = DUMIRROR_ERR_INVALID_FACE;
    return false;
  }

  return showFace(static_cast<uint8_t>(index));
}

// ============================================================
// Animation engine (non-blocking, millis()-based)
//
// A frame's duration is counted from the moment it has been drawn, so slow
// I2C transfers never shorten what the viewer sees.
// ============================================================

void DumirrorRobo::playAnimation(const Animation& anim) {

  if (!_started) {
    _error = DUMIRROR_ERR_NOT_STARTED;
    return;
  }

  if (anim.frames == nullptr || anim.count == 0) {
    _error = DUMIRROR_ERR_INVALID_ANIMATION;
    stopAnimation();
    return;
  }

  _anim = anim;            // copy: caller's descriptor need not stay alive
  _animIndex = 0;
  _animPlaying = true;
  _error = DUMIRROR_OK;

  _draw(dumirror::frameBitmap(_anim, _animIndex));

  _animFrameStart = millis();
}

void DumirrorRobo::stopAnimation() {

  _animPlaying = false;
  _anim.frames = nullptr;
  _anim.count = 0;
  _anim.loop = false;
  _anim.inFlash = false;
  _animIndex = 0;
  _animFrameStart = 0;
}

void DumirrorRobo::updateAnimation() {

  if (!_animPlaying) {
    return;
  }

  if (_anim.frames == nullptr || _anim.count == 0) {
    stopAnimation();
    return;
  }

  const unsigned long now = millis();

  if (now - _animFrameStart < dumirror::frameDuration(_anim, _animIndex)) {
    return;
  }

  uint8_t next = _animIndex + 1;

  if (next >= _anim.count) {

    if (!_anim.loop) {
      // One-shot finished: hold the last frame on screen.
      _animPlaying = false;
      return;
    }

    next = 0;
  }

  _animIndex = next;

  _draw(dumirror::frameBitmap(_anim, _animIndex));

  _animFrameStart = millis();
}

// ============================================================
// Automatic face cycling (use only when nothing else selects faces)
// ============================================================

void DumirrorRobo::update(unsigned long intervalMs) {

  updateAnimation();

  const unsigned long now = millis();

  if (intervalMs == 0 || now - _lastUpdate >= intervalMs) {

    _lastUpdate = now;

    showFace((_currentFace + 1) % FACE_COUNT);
  }
}

// ============================================================
// Text (same single framebuffer)
// ============================================================

void DumirrorRobo::showText(const char* text, int16_t x, int16_t y, uint8_t size) {

  if (text == nullptr || !_started) {
    return;
  }

  stopAnimation();
  _lastBitmap = nullptr;

  _display.clearDisplay();
  _display.setTextSize(size == 0 ? 1 : size);
  _display.setTextColor(_textColor);
  _display.setTextWrap(true);
  _display.setCursor(x, y);
  _display.print(text);
  _display.display();
}

void DumirrorRobo::showCenteredText(const char* text, uint8_t size) {

  if (text == nullptr || !_started) {
    return;
  }

  stopAnimation();
  _lastBitmap = nullptr;

  _display.clearDisplay();
  _display.setTextSize(size == 0 ? 1 : size);
  _display.setTextColor(_textColor);
  _display.setTextWrap(false);

  int16_t x1 = 0;
  int16_t y1 = 0;
  uint16_t textWidth = 0;
  uint16_t textHeight = 0;

  _display.getTextBounds(text, 0, 0, &x1, &y1, &textWidth, &textHeight);

  int16_t x = (DUMIRROR_WIDTH - static_cast<int16_t>(textWidth)) / 2 - x1;
  int16_t y = (DUMIRROR_HEIGHT - static_cast<int16_t>(textHeight)) / 2 - y1;

  if (x < 0) x = 0;
  if (y < 0) y = 0;

  _display.setCursor(x, y);
  _display.print(text);
  _display.display();
}

void DumirrorRobo::clearText() {

  stopAnimation();
  _lastBitmap = nullptr;

  if (!_started) {
    return;
  }

  _display.clearDisplay();
  _display.display();
}

void DumirrorRobo::setTextSize(uint8_t size) {
  _display.setTextSize(size == 0 ? 1 : size);
}

void DumirrorRobo::setTextColor(uint16_t color) {
  _textColor = color;
  _display.setTextColor(color);
}

void DumirrorRobo::setTextPosition(int16_t x, int16_t y) {
  _display.setCursor(x, y);
}

void DumirrorRobo::display() {

  if (!_started) {
    return;
  }

  _display.display();
}
