# API reference (v1.1.0)

```cpp
#include <Dumirror_Robo.h>
DumirrorRobo robo;                 // or DumirrorRobo robo(sda, scl, addr);
```

## Construction and start-up

| Call | Description |
|---|---|
| `DumirrorRobo(sda = DUMIRROR_PIN_DEFAULT, scl = DUMIRROR_PIN_DEFAULT, addr = 0x3C)` | Pins are honoured on ESP32 family, ESP8266 and RP2040 (arduino-pico); ignored on AVR and other fixed-pin cores. Default = the board core's own pins. |
| `bool begin()` | Initialises I2C and the OLED. On failure returns `false`; see `lastError()`. |
| `DumirrorError lastError() const` | `DUMIRROR_OK`, `_ERR_DISPLAY_NOT_FOUND`, `_ERR_CONFIG_MISMATCH`, `_ERR_INVALID_FACE`, `_ERR_INVALID_ANIMATION`, `_ERR_NOT_STARTED`. |

## Faces

| Call | Description |
|---|---|
| `bool showFace(uint8_t id)` | Static ids draw once; animated ids start the animation. `false` if invalid. |
| `bool showFaceByName(const char*)` | Case-sensitive. `false` for unknown or `nullptr`. |
| `int16_t faceIndex(const char*) const` | Id or `-1` (**new**). |
| `const char* getName(uint8_t) const` | RAM copy in a shared buffer, valid until the next call; `nullptr` if invalid. |
| `const __FlashStringHelper* getNameP(uint8_t) const` | Flash string for `Serial.print()` (**new**). |
| `const uint8_t* getBitmap(uint8_t) const` | PROGMEM pointer (first frame for animations); `nullptr` if invalid. |
| `static uint8_t faceCount()` / `uint8_t currentFace() const` | Registered count / selected id (`currentFace` is **new**). |
| `void update(unsigned long intervalMs = 1000)` | Auto-cycles faces; also advances animations. |

## Animation

| Call | Description |
|---|---|
| `void playAnimation(const Animation&)` | Copies the descriptor; draws frame 1 immediately. Frames/bitmaps must stay valid (static/global). |
| `void updateAnimation()` | Call every `loop()`. Non-blocking, `millis()`-based; advances at most one frame per call. |
| `void stopAnimation()` | Halts; current frame stays on screen. |
| `bool isAnimationPlaying() const` | `false` after a one-shot has shown its last frame for its full duration. |

```cpp
struct Frame     { const uint8_t* bitmap; uint16_t durationMs; };
struct Animation { const Frame* frames; uint8_t count; bool loop; bool inFlash; };
```

`inFlash = true` means the `Frame` array is declared `PROGMEM` (recommended on AVR);
`{frames, n, loop}` (three values) still compiles and means RAM-resident frames.

### Timing semantics

A frame's duration is counted from when it has finished drawing. A one-shot
becomes "not playing" only after its **last frame** has been visible for its
full duration, then it holds that frame. Consecutive identical bitmaps are not
re-sent to the OLED.

## Text and display

`showText(text, x=0, y=0, size=2)`, `showCenteredText(text, size=2)`,
`clearText()`, `setTextSize(n)`, `setTextColor(c)`, `setTextPosition(x, y)`,
`display()`. Text shares the single framebuffer and stops any animation.
`setTextColor()` now applies to later `showText()/showCenteredText()` calls.

`Adafruit_SSD1306& oled()` (**new**) gives access to the owned display for
custom drawing without allocating a second framebuffer.

## Configuration macros (`Dumirror_Config.h`)

`DUMIRROR_PROFILE_FULL | _COMPACT | _MINIMAL`, `DUMIRROR_ENABLE_FACE_<NAME>`,
`DUMIRROR_PIN_DEFAULT`, `DUMIRROR_ROBO_VERSION`, `DUMIRROR_OLED_ADDR`,
`DUMIRROR_WIDTH`, `DUMIRROR_HEIGHT`, `DUMIRROR_BITMAP_BYTES`.

## Backward compatibility with v1.0.0

All v1.0.0 calls are unchanged. Differences you may notice:

* Default constructor pins are now "board default" instead of fixed `8, 9`
  (identical on ESP32-S2/S3/C3, fixed on classic ESP32 where 8/9 never worked).
* `getName()` returns a pointer to a per-object buffer (previously a pointer to
  a literal) - copy it if you need two names at once.
* `playAnimation()` copies the descriptor; `Animation` has a 4th member `inFlash`.
* On 32 KB AVR boards only the MINIMAL face set is built.
