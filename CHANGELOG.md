# Changelog

## 1.1.0 - Engineering release (multi-architecture, memory, tests)

Status: validated with host-side logic tests only. **No board compilation and
no hardware test were possible** in the build environment (no network, no
Arduino cores). See `docs/COMPATIBILITY.md`.

### Fixed
- **AVR SRAM:** animation frame tables, the animation table, the bitmap pointer
  table, the name table and all face names were ordinary `const` data, which AVR
  copies into SRAM (about 450-500 bytes of an Uno's 2 KB). They are now PROGMEM
  and read through `pgm_read_*` accessors (`src/internal/Dumirror_Storage.h`).
- **Flash overflow on 32 KB AVR:** the 33,792-byte artwork cannot fit an Uno,
  Nano or Leonardo. Added face profiles (`Dumirror_Config.h`); those boards now
  default to MINIMAL (12 faces). All artwork bytes are unchanged.
- **Invalid default I2C pins:** the constructor defaulted to GPIO 8/9, which are
  flash pins on a classic ESP32. Default is now "board default"
  (`DUMIRROR_PIN_DEFAULT`). RP2040 (arduino-pico) now honours custom pins.
- `playAnimation()` stored a pointer to the caller's descriptor (dangling if it
  was a temporary). It now copies the descriptor.
- Redundant OLED transfers: frames identical to the one on screen are skipped.
- `setTextColor()` had no effect on `showText()` / `showCenteredText()`.
- `showCenteredText()` ignored the text-bounds offset.
- Unknown face names, null pointers, out-of-range ids, empty animations and
  calls before `begin()` are handled safely and reported through `lastError()`.

### Added
- `Dumirror_Config.h`: FULL / COMPACT / MINIMAL profiles and
  `DUMIRROR_ENABLE_FACE_<NAME>` switches; `begin()` detects sketch/library
  profile mismatches (`DUMIRROR_ERR_CONFIG_MISMATCH`).
- `lastError()`, `faceIndex()`, `getNameP()`, `currentFace()`, `oled()`.
- `Animation::inFlash` so custom animations can be PROGMEM.
- Examples `BasicFace` and `CustomAnimation`.
- `tests/host` (logic tests with an emulated AVR flash address space, TouchPet
  simulation, example syntax checks), `tools/validate_library.py`.
- Docs: WIRING, COMPATIBILITY, MEMORY, API, TROUBLESHOOTING; wiring diagrams.

### Changed
- **TouchPet rewritten:** one framebuffer (the second `Adafruit_SSD1306` is gone;
  boot text uses `showCenteredText()`), no `Serial.printf`, ids instead of
  names, animation-completion-aware timing (sneeze, wake-up and look/smile
  animations are no longer cut off by fixed timers; wake-up was cut 100 ms
  short), signal debouncing plus the original 300 ms touch gap, dead cooldown
  ignores touches (previously a touch during the first 1.5 s of "dead" restarted
  the sequence), unreachable inactivity check removed, a finger resting on the
  pad at boot is not counted, hardware section and per-board pins added.
  Sequence, faces and timings are otherwise unchanged. On MINIMAL (32 KB AVR) the
  sequence starts at "heartbeat" because "sneeze" is not built.
- `CycleFaces`, `AnimatedFaces`: portable (`Serial.printf` removed); AnimatedFaces
  now walks the face registry instead of a hard-coded name list.
- `Mochi_Finnish_Dinner_Date`: hardware section; refuses to build without the
  faces it needs.
- `getName()` copies into a per-object buffer (use `getNameP()` for flash strings).
- `library.properties`: `architectures` lists the targeted cores instead of `*`.
- Line endings normalised to LF.

### Removed (repository hygiene only)
- `anim_sneeze.h.bak`, `TouchPet.ino.bak`, per-example `.vscode/` folders
  (contained a personal local path).

### Animations
No frame, order, duration or loop flag was changed. Frame tables moved to
PROGMEM only. Verified: all 33 bitmap arrays are byte-identical to 1.0.0
(`python3 tools/validate_library.py --original <1.0.0 zip>`).

## 1.0.0 - Initial public release candidate

- 19 built-in static face expressions.
- 10 frame-based animations with loop and one-shot playback.
- Name-based and numeric face selection.
- Non-blocking animation updates using `millis()`.
- Text rendering helpers for SSD1306 displays.
- Custom animation support through `Frame` and `Animation`.
- Bitmap definitions compiled once by the library implementation.
- I2C initialization adapted for ESP32/ESP8266 pin selection and default-pin
  operation on other Arduino cores.
