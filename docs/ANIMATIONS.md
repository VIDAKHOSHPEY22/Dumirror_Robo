# Dumirror Animations

All 10 animations are unchanged in v1.1.0 (frames, order, durations, loop flags);
only their frame tables moved to PROGMEM. On 32 KB AVR boards the default MINIMAL
profile builds blink, heartbeat, sleep_breath and wake_up (COMPACT adds sneeze);
see `src/Dumirror_Config.h`.

## Static Faces (19)
normal, pooker, love, touch, cry, dizzy, kiss, warm, hot,
chilly, cold, angry, huh, laugh, sleepy, dead, wink, lol, sad

## Animated Faces (10)

| Name | Frames | Total | Loop | File | Use case |
|------|--------|-------|------|------|----------|
| sneeze | 6 | 1300ms | ❌ | anim_sneeze.h | sudden reaction |
| blink | 2 | 2030ms | ✅ | anim_blink.h | idle base |
| look_left | 3 | 900ms | ❌ | anim_look_left.h | looking around |
| look_right | 3 | 900ms | ❌ | anim_look_right.h | looking around |
| smile_big | 3 | 1100ms | ❌ | anim_smile_big.h | happy |
| heartbeat | 2 | 650ms | ✅ | anim_heartbeat.h | love / alive |
| sleep_breath | 2 | 1400ms | ✅ | anim_sleep_breath.h | sleeping |
| wake_up | 4 | 1300ms | ❌ | anim_wake_up.h | waking up |
| hiccup | 3 | 720ms | ❌ | anim_hiccup.h | small glitch |
| chew | 3 | 750ms | ✅ | anim_chew.h | eating |

## Adding a New Animation

1. Put frames as `images/oled-128x64-<name>-<n>.png` (128x64, 1-bit)
2. Preview: `python images_to_gif.py images/oled-128x64-<name>-*.png -o preview.gif -s 4 --oled`
3. Generate headers:
   `python png_to_header.py images/oled-128x64-<name>-2.png FACE_<NAME>_ANIM_2 --out tmp2.h`
4. Create `src/faces/anim_<name>.h` with all frames + FACE_<NAME>_ANIM[] + FACE_<NAME>_FRAMES + FACE_<NAME>_LOOP
5. Add `#include "faces/anim_<name>.h"` in `src/Dumirror_Robo.cpp`
6. Add `X_ANIM(...)` line in `src/faces.def` (update FRAMES and LOOP!)
7. Verify: `python verify_anims.py --pngs` and `python tools/validate_library.py`
8. Upload and test with `robo.showFaceByName("<name>")`

## Tools

- `images_to_gif.py` — PNG sequence → preview GIF
- `png_to_header.py` — PNG → C header array
- `verify_anims.py` — validate all anim_*.h (structure + faces.def + PNGs)