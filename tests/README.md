# Tests

| Script | What it does | Needs |
|---|---|---|
| `host/run_tests.sh` | Builds the real library `.cpp` with `g++ -Wall -Wextra -Wshadow -Werror` against shims and runs ~470 checks for several architecture macros/profiles; also runs the TouchPet state-machine simulation | `g++` |
| `host/check_examples.sh` | Syntax-checks every example against the library API for FULL, MINIMAL, ESP8266 and RP2040 macro sets | `g++` |
| `../tools/validate_library.py` | Static repository checks (faces.def, bitmaps, keywords, includes, versions, wiring docs, artwork vs. original) | Python 3 |
| `../verify_anims.py --pngs` | Compares animation headers with the PNG frames | Python 3 + Pillow |

## What the host tests are - and are not

They run the library logic on a PC. The shim emulates the AVR **Harvard memory
model**: PROGMEM objects sit in their own section whose memory pages are made
inaccessible, and `pgm_read_*()` reads a shadow copy. Therefore:

* a direct dereference of PROGMEM data crashes the test (proved by a negative control),
* a `pgm_read_*()` on a RAM address is counted and fails the test,
* the OLED shim reads every pixel byte of each drawn bitmap through `pgm_read_byte`, as Adafruit_GFX does on AVR.

They do **not** replace compiling with the real Arduino cores or running on a
board. `unsigned long` is 64-bit on the host, so `millis()` 32-bit rollover is
not exercised (the code uses same-type unsigned subtraction, which is
rollover-safe on-target).
