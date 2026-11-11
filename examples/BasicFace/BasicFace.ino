/**
 * BasicFace - the smallest useful Dumirror_Robo sketch.
 * Shows the "normal" face and plays a blink animation forever.
 */

// =====================================================
// HARDWARE CONNECTIONS
// =====================================================
//
// OLED: SSD1306 128x64, I2C, default address 0x3C (some modules: 0x3D).
//
//   Board                  OLED SDA        OLED SCL        Logic
//   ---------------------  --------------  --------------  -----
//   ESP32 (classic)        GPIO 21 *       GPIO 22 *       3.3 V
//   ESP32-S2/S3/C3         GPIO 8  *       GPIO 9  *       3.3 V
//   Arduino Uno R3/Nano    A4 (fixed)      A5 (fixed)      5 V
//   Arduino Mega 2560      D20 (fixed)     D21 (fixed)     5 V
//   Arduino Leonardo       D2 (fixed)      D3 (fixed)      5 V
//   ESP8266 (NodeMCU)      GPIO 4 / D2 *   GPIO 5 / D1 *   3.3 V
//   RP2040 (Pico)          GP4 *           GP5 *           3.3 V
//   * board-core default; can be changed on ESP32/ESP8266/RP2040 with
//     DumirrorRobo(sda, scl). Fixed on AVR boards (hardware TWI pins).
//
//   OLED VCC -> 3.3 V on 3.3 V boards (their GPIOs are not 5 V tolerant and
//               a 5 V powered module pulls SDA/SCL up to 5 V). On 5 V boards
//               use 5 V only if your module has an onboard regulator.
//   OLED GND -> GND (all modules must share a common ground).
//
// The library defaults to the board's own I2C pins, so no pin numbers are
// needed below unless your wiring differs.
// =====================================================

#include <Dumirror_Robo.h>

DumirrorRobo robo;                       // board-default I2C pins, address 0x3C

void setup() {
  Serial.begin(115200);

  if (!robo.begin()) {
    Serial.println(F("OLED not found - check wiring and I2C address"));
    while (true) {
      delay(1000);
    }
  }

  robo.showFaceByName("blink");          // looping animation (open eyes, quick blink)
}

void loop() {
  robo.updateAnimation();                // drives the animation without delay()
}
