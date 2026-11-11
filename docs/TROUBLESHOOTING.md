# Troubleshooting

| Symptom | Likely cause | Fix |
|---|---|---|
| `begin()` returns false, `lastError()` = `DUMIRROR_ERR_DISPLAY_NOT_FOUND` | wrong wiring, wrong address, no power, out of heap | check SDA/SCL, VCC, GND; try address `0x3D`; run an I2C scanner |
| `DUMIRROR_ERR_CONFIG_MISMATCH` | sketch and library built with different face profiles | define profile flags for the whole build (build flags), not inside the sketch |
| Blank screen, no error | module needs 3.3 V vs 5 V mismatch, loose jumper | verify supply, re-seat wires |
| Link error `undefined reference to FACE_SAD` (or similar) | that face is not in the active profile | enable it: `-DDUMIRROR_ENABLE_FACE_SAD=1`, or use FULL |
| `region 'text' overflowed` / sketch too big (Uno, Nano, Leonardo) | too many faces for 32 KB flash | use `DUMIRROR_PROFILE_MINIMAL` (default on those boards) or disable faces |
| Sketch hangs / garbled display on Uno | SRAM exhausted | one display only; drop `Serial` prints; check "Global variables" line |
| `showFaceByName()` returns false | name not in the active profile or misspelled | names are lower case, e.g. `"look_left"`; `faceIndex(name)` returns -1 |
| Animation frozen on one frame | `updateAnimation()` not called every `loop()` | call it each iteration; do not block with `delay()` |
| Animation looks slow | I2C at 100 kHz | the driver switches to 400 kHz during transfers; check cabling length and pull-ups |
| Wrong pins on ESP32 | passing GPIO 8/9 on classic ESP32 | omit the pins (defaults 21/22) or use free GPIOs |
| Touch never triggers | wrong pin, wrong polarity | check `TOUCH_PIN`, TTP223 jumpers, `TOUCH_ACTIVE_HIGH` |
| Touch triggers constantly | floating input / noisy supply | share GND, short wires, keep the sensor away from the OLED wires |
| Garbage text from `getName()` printed twice | `getName()` returns a shared buffer | copy the string or use `getNameP()` with `Serial.print()` |
| `Serial.printf` not found (AVR) | AVR has no `printf` on `Serial` | use `Serial.print/println` with `F()` |
