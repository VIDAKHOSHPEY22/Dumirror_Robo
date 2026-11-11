#!/bin/sh
# Syntax-checks every example against the host shims (-fsyntax-only).
# This validates the sketches against the library API and profile guards.
# It is NOT a board compile: the Arduino core, Adafruit libs and linker are absent.
cd "$(dirname "$0")"
FLAGS="-std=gnu++11 -Wall -Wextra -fsyntax-only -include shim/Arduino_extras.h -Ishim -I../../src -x c++"
fail=0
check() { ex=$1; shift
  for f in ../../examples/$ex/*.ino; do
    if g++ $FLAGS "$@" "$f" 2>/tmp/ex_err.txt; then echo "OK    $ex ($*)"; else echo "FAIL  $ex ($*)"; cat /tmp/ex_err.txt | head -15; fail=1; fi
  done; }
for ex in BasicFace CycleFaces AnimatedFaces CustomAnimation TouchPet Mochi_Finnish_Dinner_Date; do check $ex; done
for ex in BasicFace CycleFaces AnimatedFaces CustomAnimation TouchPet; do check $ex -DARDUINO_ARCH_AVR -DFLASHEND=0x7FFFUL; done
for ex in TouchPet; do check $ex -DARDUINO_ARCH_ESP8266; check $ex -DARDUINO_ARCH_RP2040; done
# Negative control: Mochi must be rejected on a 32 KB AVR profile (guard works)
if g++ $FLAGS -DARDUINO_ARCH_AVR -DFLASHEND=0x7FFFUL ../../examples/Mochi_Finnish_Dinner_Date/*.ino 2>/dev/null; then echo "FAIL  Mochi guard did not trigger"; fail=1; else echo "OK    Mochi correctly rejected on MINIMAL profile"; fi
exit $fail
