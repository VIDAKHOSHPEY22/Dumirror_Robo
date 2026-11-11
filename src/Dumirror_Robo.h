/**
 * @file Dumirror_Robo.h
 * @brief Dumirror_Robo - animated robot faces and text for SSD1306 128x64 OLED.
 *
 * Faces are registered via X-Macros in faces.def. Bitmap definitions are
 * compiled once by the implementation file so sketches do not carry a second
 * copy of the artwork. Which faces are built is controlled by Dumirror_Config.h
 * (profiles), because the full artwork set does not fit on 32 KB AVR boards.
 *
 * MEMORY MODEL
 *   - One SSD1306 framebuffer (1024 bytes, heap-allocated by Adafruit_SSD1306
 *     inside begin()) is owned by the DumirrorRobo object. Create exactly one
 *     DumirrorRobo per OLED and never a second Adafruit_SSD1306 for the same
 *     screen (use showCenteredText()/oled() instead).
 *   - Bitmaps, animation tables and face names live in flash (PROGMEM).
 *
 * @author VIDAKHOSHPEY22
 * @version 1.1.0
 * @license MIT
 */

#ifndef DUMIRROR_ROBO_H
#define DUMIRROR_ROBO_H

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include "Dumirror_Config.h"

#define DUMIRROR_ROBO_VERSION "1.1.0"

// ===== Public bitmap symbols for custom animations =====
// Definitions live in the library; they are PROGMEM data. Only the symbols of
// faces enabled in Dumirror_Config.h are defined (referencing a disabled one
// is a link error).
extern const uint8_t FACE_NORMAL[] PROGMEM;
extern const uint8_t FACE_POOKER[] PROGMEM;
extern const uint8_t FACE_LOVE[] PROGMEM;
extern const uint8_t FACE_TOUCH[] PROGMEM;
extern const uint8_t FACE_CRY[] PROGMEM;
extern const uint8_t FACE_DIZZY[] PROGMEM;
extern const uint8_t FACE_KISS[] PROGMEM;
extern const uint8_t FACE_WARM[] PROGMEM;
extern const uint8_t FACE_HOT[] PROGMEM;
extern const uint8_t FACE_CHILLY[] PROGMEM;
extern const uint8_t FACE_COLD[] PROGMEM;
extern const uint8_t FACE_ANGRY[] PROGMEM;
extern const uint8_t FACE_HUH[] PROGMEM;
extern const uint8_t FACE_LAUGH[] PROGMEM;
extern const uint8_t FACE_SLEEPY[] PROGMEM;
extern const uint8_t FACE_DEAD[] PROGMEM;
extern const uint8_t FACE_WINK[] PROGMEM;
extern const uint8_t FACE_LOL[] PROGMEM;
extern const uint8_t FACE_SAD[] PROGMEM;

#include "faces/animation_types.h"

/** @brief Default SSD1306 I2C address. */
#define DUMIRROR_OLED_ADDR 0x3C

/** @brief Display width in pixels. */
#define DUMIRROR_WIDTH  128

/** @brief Display height in pixels. */
#define DUMIRROR_HEIGHT 64

/** @brief Longest built-in face name ("sleep_breath") without the terminator. */
#define DUMIRROR_MAX_NAME_LEN 12

/**
 * @brief Face identifiers, generated automatically from faces.def.
 *
 * Static faces (X lines) come first, animated faces (X_ANIM lines) follow.
 * FACE_STATIC_COUNT is the number of static faces; FACE_COUNT is the total.
 */
enum DumirrorFace : uint8_t {

#define X(bitmap, enumId, name) enumId,
#include "faces.def"
#undef X

  FACE_STATIC_COUNT,

  // Alias trick: makes the first animated id equal FACE_STATIC_COUNT.
  FACE_ANIM_ORIGIN_ = FACE_STATIC_COUNT - 1,

#define X_ANIM(tag, enumId, name, animArray, frameCount, loops) enumId,
#include "faces.def"
#undef X_ANIM

  FACE_COUNT
};

/** @brief Result codes reported by DumirrorRobo::lastError(). */
enum DumirrorError : uint8_t {
  DUMIRROR_OK = 0,
  DUMIRROR_ERR_DISPLAY_NOT_FOUND,  ///< begin(): SSD1306 did not respond / no memory.
  DUMIRROR_ERR_CONFIG_MISMATCH,    ///< Sketch and library built with different face sets.
  DUMIRROR_ERR_INVALID_FACE,       ///< Unknown face id or name.
  DUMIRROR_ERR_INVALID_ANIMATION,  ///< Null / empty / oversized animation.
  DUMIRROR_ERR_NOT_STARTED         ///< begin() has not succeeded yet.
};

/**
 * @class DumirrorRobo
 * @brief Drives robot faces and text on an SSD1306 OLED.
 *
 * @code
 * DumirrorRobo robo;                 // board-default I2C pins
 *
 * void setup() {
 *   robo.begin();
 *   robo.showFaceByName("normal");
 * }
 *
 * void loop() {
 *   robo.updateAnimation();          // call as often as possible
 * }
 * @endcode
 */
class DumirrorRobo {

public:

  /**
   * @brief Construct a DumirrorRobo object.
   *
   * @param sda  SDA pin, or DUMIRROR_PIN_DEFAULT for the board's default.
   * @param scl  SCL pin, or DUMIRROR_PIN_DEFAULT for the board's default.
   * @param addr OLED I2C address (default 0x3C).
   *
   * Pin selection is honoured on ESP32 family, ESP8266 and RP2040
   * (arduino-pico). On AVR and most other cores the I2C pins are fixed by
   * the hardware; the arguments are ignored there.
   */
  DumirrorRobo(
    uint8_t sda = DUMIRROR_PIN_DEFAULT,
    uint8_t scl = DUMIRROR_PIN_DEFAULT,
    uint8_t addr = DUMIRROR_OLED_ADDR
  );

  /** @brief Initialise I2C and the OLED. @return true on success; see lastError(). */
  bool begin() { return _begin(FACE_COUNT); }

  /** @brief Show a face by numeric id. Animated ids start the animation. @return false if id invalid. */
  bool showFace(uint8_t face);

  /** @brief Show a face by name, e.g. "love" or "sneeze". @return false if not found. */
  bool showFaceByName(const char* name);

  /** @brief Index of a face name, or -1 if unknown / null. */
  int16_t faceIndex(const char* name) const;

  /** @brief Auto-cycle all faces; call from loop(). */
  void update(unsigned long intervalMs = 1000);

  /** @brief Raw bitmap pointer (PROGMEM) of a face; first frame for animations; nullptr if invalid. */
  const uint8_t* getBitmap(uint8_t face) const;

  /**
   * @brief Face name as a normal RAM string, or nullptr if invalid.
   *
   * The text is copied into a small buffer inside the object, so the pointer
   * is valid only until the next getName() call on the same object. Use
   * getNameP() to print without using RAM.
   */
  const char* getName(uint8_t face) const;

  /** @brief Face name as a flash string (print with Serial.print), or nullptr. */
  const __FlashStringHelper* getNameP(uint8_t face) const;

  /** @brief Number of registered faces (static + animated). */
  static uint8_t faceCount() { return FACE_COUNT; }

  /** @brief Id of the face currently selected with showFace*(). */
  uint8_t currentFace() const { return _currentFace; }

  /**
   * @brief Start playing an animation (draws frame 1 immediately).
   *
   * The Animation descriptor is copied, so a temporary is fine. The Frame
   * array and bitmaps it points to must stay valid while it plays (static
   * or global data).
   */
  void playAnimation(const Animation& anim);

  /** @brief Stop the running animation; the current frame stays on screen. */
  void stopAnimation();

  /** @brief Advance the animation if the frame time elapsed; call from loop(). */
  void updateAnimation();

  /** @brief True while an animation is running (one-shots become false when done). */
  bool isAnimationPlaying() const { return _animPlaying; }

  /** @brief Last error; cleared by the next successful call. */
  DumirrorError lastError() const { return _error; }

  // =====================================================
  // TEXT DISPLAY FUNCTIONS (share the single framebuffer)
  // =====================================================

  /** @brief Show text at x,y. Replaces the current face and stops animation. */
  void showText(const char* text, int16_t x = 0, int16_t y = 0, uint8_t size = 2);

  /** @brief Show text centred on screen. */
  void showCenteredText(const char* text, uint8_t size = 2);

  /** @brief Clear the OLED. */
  void clearText();

  /** @brief Set text size used by later showText() calls (0 is treated as 1). */
  void setTextSize(uint8_t size);

  /** @brief Set text colour (SSD1306_WHITE or SSD1306_BLACK) for later text calls. */
  void setTextColor(uint16_t color);

  /** @brief Set the text cursor. */
  void setTextPosition(int16_t x, int16_t y);

  /** @brief Push the framebuffer to the OLED. */
  void display();

  /**
   * @brief Access the library-owned Adafruit_SSD1306 for custom drawing.
   *
   * Use this instead of creating a second display object (a second 1 KB
   * framebuffer will not fit on an Uno). Stops no animation; call
   * stopAnimation() first if you draw over a running one, and call
   * display() when finished.
   */
  Adafruit_SSD1306& oled() { _lastBitmap = nullptr; return _display; }

private:

  Adafruit_SSD1306 _display;

  uint8_t _addr;
  uint8_t _sda;
  uint8_t _scl;
  uint8_t _currentFace;
  bool    _started;
  DumirrorError _error;

  uint16_t _textColor;

  unsigned long _lastUpdate;

  Animation     _anim;
  uint8_t       _animIndex;
  unsigned long _animFrameStart;
  bool          _animPlaying;
  const uint8_t* _lastBitmap;     // bitmap currently on screen (skip redundant redraws)

  mutable char _nameBuf[DUMIRROR_MAX_NAME_LEN + 1];

  bool _begin(uint8_t sketchFaceCount);
  void _initI2C();
  void _draw(const uint8_t* bitmap);
};

#endif // DUMIRROR_ROBO_H
