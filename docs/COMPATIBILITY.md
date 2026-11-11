# Compatibility matrix

Status categories:

* **Fully tested** - built *and* run on real hardware.
* **Compile-tested** - built with the real Arduino core and libraries.
* **Host-logic-tested** - library logic exercised on a PC with shims (this is
  *not* a board build; it cannot find toolchain-specific errors).
* **Compatible in principle (untested)** - code paths exist, nothing was run.
* **Unsupported**.

**Honest summary of this release:** the build environment had no network access
and no Arduino cores, so **no board compilation and no hardware test was
possible**. Nothing below is marked "Fully tested" or "Compile-tested".
The library is **host-logic-tested** (see `tests/`), with an emulated AVR flash
address space, for each architecture's code path.

| Board / core | Default face profile | Status | Notes |
|---|---|---|---|
| ESP32 | FULL (29 faces) | Host-logic-tested; compatible in principle | `Wire.begin(sda, scl)`; default pins used when unspecified |
| ESP32-S2 | FULL | Host-logic-tested (ESP32 path); untested | |
| ESP32-S3 | FULL | Host-logic-tested (ESP32 path); untested | Original author's target board |
| ESP32-C3 | FULL | Host-logic-tested (ESP32 path); untested | |
| Arduino Uno R3 | MINIMAL (12 faces) | Host-logic-tested; **flash fit unmeasured** | 32 KB flash: the full artwork (33,792 B) cannot fit |
| Arduino Nano Classic | MINIMAL | as Uno | ATmega328P, same limits |
| Arduino Leonardo | MINIMAL | as Uno; **flash fit unmeasured** | ~28 KB usable (bootloader); consider a smaller custom set |
| Arduino Mega 2560 | FULL | Host-logic-tested; untested | 256 KB flash, 8 KB SRAM; PROGMEM data must stay below 64 KB (it is ~35 KB) |
| ESP8266 | FULL | Host-logic-tested (ESP8266 path); untested | |
| RP2040 (arduino-pico) | FULL | Host-logic-tested; untested | `setSDA/setSCL` before `begin()` |
| RP2040 (Arduino mbed core) | FULL | Compatible in principle (untested) | Uses plain `Wire.begin()` |
| Nano Every (megaAVR) | COMPACT | Compatible in principle (untested) | Different architecture macro; flash fit unmeasured |
| STM32 / SAMD / others | FULL | Compatible in principle (untested) | Generic `Wire.begin()` path; not listed in `architectures` |

`library.properties` therefore lists `architectures=avr,esp32,esp8266,rp2040`,
the cores this release targets, rather than `*`.

## What *was* measured

From the host build (`tests/host/run_tests.sh`), the size of all PROGMEM data
(bitmaps, frame tables, names, descriptors):

| Profile | Faces | PROGMEM data |
|---|---|---|
| FULL | 29 (19 static + 10 animations) | 35,200 B |
| COMPACT | 13 | 18,080 B |
| MINIMAL | 12 | 13,856 B |

Adafruit GFX + SSD1306 + Wire code adds an estimated 8-10 KB on AVR (an
estimate, not a measurement). Measure your own build with the Arduino IDE
output ("Sketch uses ... bytes") and adjust the profile if it overflows.

## Requested compatibility table

| Board | Compilation | Flash | SRAM | Hardware Test | Status |
|---|---|---|---|---|---|
| ESP32 | not run (no toolchain) | n/a | n/a | none | host-logic-tested |
| ESP32-S3 | not run | n/a | n/a | none | host-logic-tested |
| ESP32-C3 | not run | n/a | n/a | none | host-logic-tested |
| Arduino Uno R3 | not run | n/a (data 13,856 B MINIMAL) | n/a (see MEMORY.md estimate) | none | host-logic-tested |
| Arduino Mega 2560 | not run | n/a (data 35,200 B) | n/a | none | host-logic-tested |
| Arduino Nano Classic | not run | as Uno | as Uno | none | host-logic-tested |
| ESP8266 | not run | n/a | n/a | none | host-logic-tested |
| RP2040 | not run | n/a | n/a | none | host-logic-tested |
