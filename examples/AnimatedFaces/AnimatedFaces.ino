/**
 * AnimatedFaces - plays every registered face, one every 3 seconds, then a
 * custom wink animation, and repeats. The playlist is read from the library's
 * registry, so it adapts to the active face profile automatically.
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

constexpr uint32_t SHOW_TIME_MS = 3000;

// Custom wink animation played after the full list: normal -> wink -> normal.
static const Frame CUSTOM_FRAMES[] PROGMEM = {
  { FACE_NORMAL, 600 },
  { FACE_WINK,   300 },
  { FACE_NORMAL, 600 },
};
static const Animation CUSTOM_ANIM = { CUSTOM_FRAMES, 3, false, true };

enum class Mode : uint8_t { PLAY_FACE, PLAY_CUSTOM };

Mode          mode       = Mode::PLAY_FACE;
uint8_t       idx        = 0;
unsigned long lastSwitch = 0;

void showCurrent() {
  Serial.print('[');
  Serial.print(idx + 1);
  Serial.print('/');
  Serial.print(robo.faceCount());
  Serial.print(F("] "));
  Serial.println(robo.getNameP(idx));
  robo.showFace(idx);
}

void advance() {
  if (mode == Mode::PLAY_FACE) {
    if (idx + 1 < robo.faceCount()) {
      ++idx;
      showCurrent();
    } else {
      Serial.println(F("custom wink"));
      robo.playAnimation(CUSTOM_ANIM);
      mode = Mode::PLAY_CUSTOM;
    }
  } else {
    mode = Mode::PLAY_FACE;
    idx = 0;
    showCurrent();
  }
}

void setup() {
  Serial.begin(115200);

  if (!robo.begin()) {
    Serial.println(F("ERROR: OLED not found - check wiring (SDA, SCL, VCC, GND)"));
    while (true) {
      delay(1000);
    }
  }

  Serial.print(F("Dumirror_Robo ready. Faces: "));
  Serial.println(robo.faceCount());

  showCurrent();
  lastSwitch = millis();
}

void loop() {
  robo.updateAnimation();                // must run every loop()

  if (millis() - lastSwitch >= SHOW_TIME_MS) {
    lastSwitch = millis();
    advance();
  }
}
