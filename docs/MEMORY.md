# Memory guide

## Where everything lives

| Item | Storage | Notes |
|---|---|---|
| Face and frame bitmaps (1024 B each) | Flash (PROGMEM) | read with `pgm_read_byte` by Adafruit_GFX |
| Built-in animation frame tables | Flash (PROGMEM) | **v1.1.0: moved out of SRAM** (were `.data` on AVR) |
| Built-in animation descriptors | Flash (PROGMEM) | v1.1.0 |
| Face name strings and name table | Flash (PROGMEM) | v1.1.0; `getName()` copies into a 13-byte member buffer |
| Static face pointer table | Flash (PROGMEM) | v1.1.0 |
| OLED framebuffer | Heap, 1024 B | allocated by `Adafruit_SSD1306::begin()` - **only one per sketch** |
| `DumirrorRobo` object | Static/stack | roughly 60-70 bytes of state plus the `Adafruit_SSD1306` object |

### SRAM saved on AVR by v1.1.0 (analysis, not a measurement)

In v1.0.0 the following were ordinary `const` data, which AVR copies into SRAM
at startup: 33 `Frame` entries (4 B each = 132 B), the animation table
(10 x 4 B), the bitmap pointer table (19 x 2 B), the name pointer table
(29 x 2 B) and 29 name strings (~190 B). That is about **450-500 bytes of an
Uno's 2048-byte SRAM**. In v1.1.0 these live in flash. No compiler was
available to confirm the exact figure; build your sketch and compare the
"Global variables use ..." line.

## Framebuffer budget

An SSD1306 128x64 needs 1024 B. An Uno has 2048 B of SRAM, so:

* Create exactly **one** `DumirrorRobo` and never a second `Adafruit_SSD1306`.
  The v1.0.0 TouchPet used two display objects (2 KB) and could not run on an Uno.
* Use `robo.showCenteredText()` / `robo.showText()` for text, and `robo.oled()`
  for custom drawing on the same buffer.
* `Wire` (~130 B) and `Serial` (~150 B) buffers also come out of SRAM.
  Remove `Serial` use if you run out.

## Flash budget and face profiles

The artwork is 33 unique bitmaps = **33,792 bytes**, more than an Uno, Nano or
Leonardo can hold. `src/Dumirror_Config.h` selects a subset by profile
(see COMPATIBILITY.md for the measured sizes). To build exactly the faces you need:

```
-DDUMIRROR_PROFILE_MINIMAL -DDUMIRROR_ENABLE_FACE_SNEEZE=1 -DDUMIRROR_ENABLE_FACE_HICCUP=0
```

(PlatformIO `build_flags`, arduino-cli `--build-property`, or edit the header).
Defining these inside a sketch has no effect on the already-separately-compiled
library; `begin()` returns `false` and `lastError()` reports
`DUMIRROR_ERR_CONFIG_MISMATCH` if the sketch and library disagree on the number
of faces.

## Addressing limit on AVR

`pgm_read_*` "near" addressing reaches the first 64 KB of flash. The 35 KB of
FULL data sits below that on every AVR (the linker places `.progmem.data` first),
so no `PROGMEM_FAR` handling is needed.

## Dynamic allocation

The only heap allocation is the framebuffer inside `Adafruit_SSD1306::begin()`.
The library itself never calls `malloc`/`new`. `begin()` returns `false` if that
allocation fails.
