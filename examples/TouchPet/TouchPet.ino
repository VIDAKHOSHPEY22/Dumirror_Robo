/**
 * TouchPet - a touch-reactive Dumirror robot pet.
 *
 * Boot: "Welcome" -> "Dumirror" -> "touch me" -> pet runs.
 * Each touch advances a fixed reaction sequence:
 *   sneeze, heartbeat, love, touch, wink, huh, dizzy, dead
 * The pet then "plays dead" for a cooldown, and the sequence restarts.
 * While untouched it blinks, looks around and smiles at random, falls
 * asleep after 45 s, and wakes up (wake_up animation) when touched.
 *
 * =====================================================
 * HARDWARE CONNECTIONS
 * =====================================================
 *
 * OLED: SSD1306 128x64, I2C, default address 0x3C (some modules use 0x3D).
 * Touch sensor: TTP223 module, digital output (HIGH while touched).
 * Every module must share a common GND with the board.
 *
 * Board                 OLED SDA      OLED SCL      TOUCH OUT   Logic
 * --------------------  ------------  ------------  ----------  -----
 * ESP32 (classic)       GPIO 21 *     GPIO 22 *     GPIO 4      3.3 V
 * ESP32-S2              GPIO 8  *     GPIO 9  *     GPIO 4      3.3 V
 * ESP32-S3              GPIO 8  *     GPIO 9  *     GPIO 4      3.3 V
 * ESP32-C3              GPIO 8  *     GPIO 9  *     GPIO 4      3.3 V
 * Arduino Uno R3        A4 (fixed)    A5 (fixed)    D4          5 V
 * Arduino Nano Classic  A4 (fixed)    A5 (fixed)    D4          5 V
 * Arduino Mega 2560     D20 (fixed)   D21 (fixed)   D4          5 V
 * Arduino Leonardo      D2 (fixed)    D3 (fixed)    D4          5 V
 * ESP8266 (NodeMCU)     GPIO 4 / D2 * GPIO 5 / D1 * GPIO 14/D5  3.3 V
 * RP2040 (Pico)         GP4 *         GP5 *         GP15        3.3 V
 *   * = board-core default; changeable on ESP32/ESP8266/RP2040 by passing
 *       pins to DumirrorRobo(sda, scl). Fixed on AVR (hardware TWI pins).
 *
 * Power:
 *   - 3.3 V boards (ESP32/ESP8266/RP2040): power BOTH the OLED and the
 *     TTP223 from 3.3 V. These GPIOs are not 5 V tolerant, and a 5 V
 *     powered module pulls SDA/SCL (OLED) or its OUT pin (TTP223) up to
 *     5 V, which can damage the chip.
 *   - 5 V boards (Uno/Nano/Mega/Leonardo): OLED modules that include a
 *     regulator accept 5 V; bare 3.3 V-only modules must use the 3.3 V pin.
 *     Check your module's datasheet. TTP223 works from 2.0-5.5 V.
 *   - The default TTP223 output is active HIGH. If you move the module's
 *     solder jumpers to active LOW, set TOUCH_ACTIVE_HIGH to false.
 *
 * Why the touch pin is D4 on AVR: D2/D3 are the Leonardo's I2C pins and D2
 * is an interrupt pin on the Uno; D4 is a plain GPIO on all four boards.
 *
 * =====================================================
 * MEMORY NOTES (AVR)
 * =====================================================
 * The sketch uses ONE OLED framebuffer (owned by `robo`); the boot text uses
 * robo.showCenteredText() instead of a second display object. On 32 KB AVR
 * boards the library builds the MINIMAL profile (see Dumirror_Config.h), in
 * which "sneeze" is not available: the reaction sequence skips it and
 * starts at "heartbeat". Everything else is identical on all boards.
 */

#include <Arduino.h>
#include <Dumirror_Robo.h>

// ---------------------------------------------------------------------
// Profile check: TouchPet needs these faces (all present in FULL, COMPACT
// and MINIMAL except sneeze, which is optional).
// ---------------------------------------------------------------------
#if !(DUMIRROR_ENABLE_FACE_HEARTBEAT && DUMIRROR_ENABLE_FACE_LOVE && \
      DUMIRROR_ENABLE_FACE_TOUCH && DUMIRROR_ENABLE_FACE_WINK &&     \
      DUMIRROR_ENABLE_FACE_HUH && DUMIRROR_ENABLE_FACE_DIZZY &&      \
      DUMIRROR_ENABLE_FACE_DEAD && DUMIRROR_ENABLE_FACE_SLEEP_BREATH && \
      DUMIRROR_ENABLE_FACE_WAKE_UP && DUMIRROR_ENABLE_FACE_BLINK)
  #error "TouchPet needs the COMPACT, MINIMAL or FULL profile (see Dumirror_Config.h)."
#endif

// ---------------------------------------------------------------------
// Pins
// ---------------------------------------------------------------------
// DUMIRROR_PIN_DEFAULT = use the board core's default SDA/SCL (see table).
constexpr uint8_t OLED_SDA = DUMIRROR_PIN_DEFAULT;
constexpr uint8_t OLED_SCL = DUMIRROR_PIN_DEFAULT;
constexpr uint8_t OLED_ADDR = DUMIRROR_OLED_ADDR;

#if defined(ARDUINO_ARCH_ESP8266)
constexpr uint8_t TOUCH_PIN = 14;      // D5
#elif defined(ARDUINO_ARCH_RP2040)
constexpr uint8_t TOUCH_PIN = 15;      // GP15
#else
constexpr uint8_t TOUCH_PIN = 4;       // ESP32 family and all AVR boards
#endif

constexpr bool TOUCH_ACTIVE_HIGH = true;   // TTP223 default output polarity

// ---------------------------------------------------------------------
// Timing (ms)
// ---------------------------------------------------------------------
constexpr uint32_t WELCOME_MS = 2000;
constexpr uint32_t BRAND_MS = 2000;
constexpr uint32_t TOUCHME_MS = 2000;

constexpr uint32_t DEBOUNCE_MS = 30;           // signal must be stable this long
constexpr uint32_t TOUCH_GAP_MS = 300;         // minimum time between accepted touches
constexpr uint32_t TOUCH_MEMORY_MS = 10000;    // sequence restarts after this idle time
constexpr uint32_t DEAD_REACTION_MS = 1500;    // "dead" face shown...
constexpr uint32_t DEAD_COOLDOWN_MS = 3000;    // ...then touches are ignored this long

constexpr uint32_t SLEEP_AFTER_MS = 45000;
constexpr uint32_t IDLE_MIN_GAP_MS = 1800;
constexpr uint32_t IDLE_MAX_GAP_MS = 4200;

#define SERIAL_DEBUG 1

#if SERIAL_DEBUG
  #define DBG(x)  Serial.print(x)
  #define DBGLN(x) Serial.println(x)
#else
  #define DBG(x)
  #define DBGLN(x)
#endif

// ---------------------------------------------------------------------
// The single display owner
// ---------------------------------------------------------------------
DumirrorRobo robo(OLED_SDA, OLED_SCL, OLED_ADDR);

// ---------------------------------------------------------------------
// Reaction tables. A reaction ends when `holdMs` has passed AND, if
// waitAnim is set, the face's animation has finished. This fixes the
// original sketch, whose fixed timers could cut an animation's last frame.
// ---------------------------------------------------------------------
struct Reaction {
  uint8_t  face;      // DumirrorFace id
  uint16_t holdMs;
  bool     waitAnim;
};

const Reaction TOUCH_REACTIONS[] = {
#if DUMIRROR_ENABLE_FACE_SNEEZE
  { FACE_SNEEZE_ID,    1300, true  },
#endif
  { FACE_HEARTBEAT_ID, 2000, false },   // looping animation: timed
  { FACE_LOVE_ID,       900, false },
  { FACE_TOUCH_ID,      900, false },
  { FACE_WINK_ID,       900, false },
  { FACE_HUH_ID,       1000, false },
  { FACE_DIZZY_ID,     1500, false },
  { FACE_DEAD_ID,      DEAD_REACTION_MS, false }   // last step -> cooldown
};
constexpr uint8_t TOUCH_STEPS = sizeof(TOUCH_REACTIONS) / sizeof(TOUCH_REACTIONS[0]);

const Reaction IDLE_ACTIONS[] = {
  { FACE_BLINK_ID,      2200, false },  // looping: one full blink cycle plus margin
#if DUMIRROR_ENABLE_FACE_LOOK_LEFT
  { FACE_LOOK_LEFT_ID,   950, true  },
#endif
#if DUMIRROR_ENABLE_FACE_LOOK_RIGHT
  { FACE_LOOK_RIGHT_ID,  950, true  },
#endif
#if DUMIRROR_ENABLE_FACE_SMILE_BIG
  { FACE_SMILE_BIG_ID,  1100, true  },
#endif
  { FACE_NORMAL_ID,      900, false }
};
constexpr uint8_t IDLE_COUNT = sizeof(IDLE_ACTIONS) / sizeof(IDLE_ACTIONS[0]);

// ---------------------------------------------------------------------
// State
// ---------------------------------------------------------------------
enum class BootState : uint8_t { WELCOME, BRAND, TOUCHME, RUNNING };
enum class PetState  : uint8_t { AWAKE, REACTING, SLEEPING, WAKING, DEAD_COOLDOWN };

BootState bootState = BootState::WELCOME;
PetState  petState  = PetState::AWAKE;

uint32_t stateStartMs = 0;       // start of boot step / reaction / cooldown / wake
uint32_t lastTouchMs = 0;        // last accepted touch (or boot end)
uint32_t lastAcceptedMs = 0;     // for TOUCH_GAP_MS
uint32_t nextIdleMs = 0;
uint32_t signalChangedMs = 0;    // debounce timer

const Reaction* current = nullptr;   // reaction in progress

uint8_t touchStep = 0;           // number of sequence steps already played
bool rawTouched = false;         // last raw reading
bool stableTouched = false;      // debounced state

// ---------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------
bool readTouch() {
  const bool v = digitalRead(TOUCH_PIN) == HIGH;
  return TOUCH_ACTIVE_HIGH ? v : !v;
}

uint32_t randomBetween(uint32_t lo, uint32_t hi) {
  return (hi <= lo) ? lo : lo + random(hi - lo + 1);
}

void showFace(uint8_t id) {
  robo.showFace(id);
  DBG(F("Face: "));
  DBGLN(robo.getNameP(id));
}

void scheduleIdle(uint32_t now) {
  nextIdleMs = now + randomBetween(IDLE_MIN_GAP_MS, IDLE_MAX_GAP_MS);
}

void returnToIdle(uint32_t now) {
  petState = PetState::AWAKE;
  current = nullptr;
  showFace(FACE_NORMAL_ID);
  scheduleIdle(now);
}

void startReaction(const Reaction& r, uint32_t now) {
  current = &r;
  petState = PetState::REACTING;
  stateStartMs = now;
  showFace(r.face);
}

// ---------------------------------------------------------------------
// Boot sequence (non-blocking, single framebuffer)
// ---------------------------------------------------------------------
void updateBoot(uint32_t now) {
  switch (bootState) {
    case BootState::WELCOME:
      if (now - stateStartMs >= WELCOME_MS) {
        robo.showCenteredText("Dumirror", 2);
        bootState = BootState::BRAND;
        stateStartMs = now;
      }
      break;
    case BootState::BRAND:
      if (now - stateStartMs >= BRAND_MS) {
        robo.showCenteredText("touch me", 2);
        bootState = BootState::TOUCHME;
        stateStartMs = now;
      }
      break;
    case BootState::TOUCHME:
      if (now - stateStartMs >= TOUCHME_MS) {
        bootState = BootState::RUNNING;
        lastTouchMs = now;
        // Start from the real sensor state so a finger resting on the pad
        // during boot is not reported as a fresh touch.
        rawTouched = stableTouched = readTouch();
        signalChangedMs = now;
        returnToIdle(now);
        DBGLN(F("Pet ready"));
      }
      break;
    case BootState::RUNNING:
      break;
  }
}

// ---------------------------------------------------------------------
// A new (debounced) touch
// ---------------------------------------------------------------------
void handleTouch(uint32_t now) {

  // Touches during the "dead" cooldown are ignored and do not extend the
  // pet's awake time.
  if (petState == PetState::DEAD_COOLDOWN) {
    DBGLN(F("Touch ignored (dead cooldown)"));
    return;
  }

  const bool wasSleeping = (petState == PetState::SLEEPING);
  const bool idleTooLong = (now - lastTouchMs >= TOUCH_MEMORY_MS);
  lastTouchMs = now;

  if (wasSleeping) {
    // Wake up without consuming a sequence step; start the sequence fresh.
    touchStep = 0;
    petState = PetState::WAKING;
    stateStartMs = now;
    showFace(FACE_WAKE_UP_ID);
    return;
  }

  if (petState == PetState::WAKING) {
    returnToIdle(now);       // a touch while waking just finishes the wake-up
    return;
  }

  if (idleTooLong || touchStep >= TOUCH_STEPS) {
    touchStep = 0;
    DBGLN(F("Touch sequence restarted"));
  }

  const Reaction& r = TOUCH_REACTIONS[touchStep++];
  DBG(F("Touch "));
  DBG(touchStep);
  DBG('/');
  DBGLN(TOUCH_STEPS);

  startReaction(r, now);

  if (touchStep == TOUCH_STEPS) {
    // Last step: the pet plays dead; touches are ignored until it recovers.
    petState = PetState::DEAD_COOLDOWN;
  }
}

// ---------------------------------------------------------------------
// Pet state machine
// ---------------------------------------------------------------------
void updatePet(uint32_t now) {
  switch (petState) {

    case PetState::AWAKE:
      if (now - lastTouchMs >= SLEEP_AFTER_MS) {
        petState = PetState::SLEEPING;
        showFace(FACE_SLEEP_BREATH_ID);
        DBGLN(F("Pet sleeping"));
      } else if (now >= nextIdleMs) {
        startReaction(IDLE_ACTIONS[random(IDLE_COUNT)], now);
      }
      break;

    case PetState::REACTING:
      if (now - stateStartMs >= current->holdMs &&
          !(current->waitAnim && robo.isAnimationPlaying())) {
        returnToIdle(now);
      }
      break;

    case PetState::DEAD_COOLDOWN:
      if (now - stateStartMs >= (uint32_t)DEAD_REACTION_MS + DEAD_COOLDOWN_MS) {
        touchStep = 0;
        lastTouchMs = now;           // recover as "just touched": no instant sleep
        returnToIdle(now);
        DBGLN(F("Touch sequence reset"));
      }
      break;

    case PetState::SLEEPING:
      break;                          // sleep_breath loops until touched

    case PetState::WAKING:
      if (!robo.isAnimationPlaying()) {   // let the whole wake_up animation play
        returnToIdle(now);
      }
      break;
  }
}

// ---------------------------------------------------------------------
// Arduino entry points
// ---------------------------------------------------------------------
void setup() {
  Serial.begin(115200);

  if (!robo.begin()) {
    Serial.println(F("OLED init failed - check wiring/address"));
    while (true) {
      delay(1000);
    }
  }

  pinMode(TOUCH_PIN, TOUCH_ACTIVE_HIGH ? INPUT : INPUT_PULLUP);
  randomSeed(micros());

  DBGLN(F("=== Dumirror Pet ==="));

  robo.showCenteredText("Welcome", 2);
  bootState = BootState::WELCOME;
  stateStartMs = millis();
}

void loop() {
  const uint32_t now = millis();

  robo.updateAnimation();            // required for every animated face

  if (bootState != BootState::RUNNING) {
    updateBoot(now);
    return;
  }

  // Debounce: accept a new level only after it has been stable.
  const bool raw = readTouch();
  if (raw != rawTouched) {
    rawTouched = raw;
    signalChangedMs = now;
  }
  if (rawTouched != stableTouched && now - signalChangedMs >= DEBOUNCE_MS) {
    stableTouched = rawTouched;
    if (stableTouched && now - lastAcceptedMs >= TOUCH_GAP_MS) {
      lastAcceptedMs = now;
      handleTouch(now);
    }
  }

  updatePet(now);
}
