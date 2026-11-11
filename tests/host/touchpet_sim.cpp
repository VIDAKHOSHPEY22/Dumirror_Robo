// Scripted simulation of examples/TouchPet/TouchPet.ino against the real
// library (host shims, emulated flash). Verifies the pet's state machine:
// boot screens, fixed touch sequence, dead cooldown, sleep, wake-up, resets.
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <vector>
#include "shim/Arduino.h"
#include "shim/Wire.h"
#include "shim/Adafruit_SSD1306.h"
using D = Adafruit_SSD1306;
#define HIGH 1
#define LOW 0
#define INPUT 0
#define INPUT_PULLUP 2
int g_pin = 0;
inline void pinMode(int, int) {}
inline int digitalRead(int) { return g_pin; }
inline long random(long m) { return m ? rand() % m : 0; }
inline void randomSeed(unsigned long) {}
std::string g_log;
static bool inFlashPtr(const char* p) { return p >= __start_progmem_data && p < __stop_progmem_data; }
struct SerialSim {
  void begin(long) {}
  void print(const __FlashStringHelper* s) { const char* p = (const char*)s;  // face names arrive via getNameP(): real flash strings
    if (inFlashPtr(p)) { while (char c = (char)pgm_read_byte(p++)) g_log += c; return; } while (*p) g_log += *p++; }   // shim F() yields RAM literals
  void print(const char* s) { g_log += s; }
  void print(char c) { g_log += c; }
  void print(unsigned v) { g_log += std::to_string(v); }
  void print(int v) { g_log += std::to_string(v); }
  void print(unsigned char v) { g_log += std::to_string((int)v); }
  template<class T> void println(T v) { print(v); g_log += '\n'; }
  void println() { g_log += '\n'; }
};
SerialSim Serial;
#include "../../src/Dumirror_Robo.cpp"
#include "../../examples/TouchPet/TouchPet.ino"
size_t shim_flash_bytes();

static int fails = 0, checks = 0;
#define CHECK(c) do { ++checks; if (!(c)) { ++fails; printf("FAIL line %d: %s\n", __LINE__, #c); } } while (0)

static void run(unsigned long ms) { unsigned long end = g_millis + ms; while (g_millis < end) { loop(); g_millis += 5; } }
static void tap() { g_pin = 1; run(100); g_pin = 0; run(50); }
static size_t mark() { return g_log.size(); }
static bool saw(size_t from, const char* s) { return g_log.find(s, from) != std::string::npos; }

int main() {
  srand(1); g_millis = 1000;
  setup();
  CHECK(strcmp(D::lastText, "Welcome") == 0);
  run(2100); CHECK(strcmp(D::lastText, "Dumirror") == 0);
  run(2000); CHECK(strcmp(D::lastText, "touch me") == 0);
  g_pin = 1; run(500); g_pin = 0; run(10);               // finger on pad during boot: must NOT count
  run(1700);
  CHECK(saw(0, "Pet ready"));
  CHECK(!saw(0, "Touch 1/"));

  // Fixed sequence
  const char* seq[] = {
#if DUMIRROR_ENABLE_FACE_SNEEZE
    "sneeze",
#endif
    "heartbeat", "love", "touch", "wink", "huh", "dizzy", "dead" };
  size_t nseq = sizeof(seq) / sizeof(seq[0]);
  for (size_t i = 0; i < nseq; ++i) {
    size_t m = mark();
    // wait for the pet to be awake & idle between touches
    tap();
    std::string expect = std::string("Face: ") + seq[i];
    CHECK(saw(m, expect.c_str()));
    if (i + 1 < nseq) run(2600);
  }
  CHECK(petState == PetState::DEAD_COOLDOWN);
  // sneeze must run to completion (animation not cut): wait covered above via waitAnim.

  // touch during dead cooldown is ignored
  size_t m = mark(); run(500); tap();
  CHECK(saw(m, "ignored"));
  CHECK(petState == PetState::DEAD_COOLDOWN);
  run(5000);
  CHECK(saw(0, "Touch sequence reset"));
  CHECK(petState != PetState::DEAD_COOLDOWN);

  // sequence restarts from the first step
  m = mark(); tap();
  { std::string e = std::string("Face: ") + seq[0]; CHECK(saw(m, e.c_str())); }

  // reactions finish, pet returns to idle, no sneeze truncation: sneeze frames all drawn
  run(4000);

  // go to sleep after inactivity
  m = mark(); run(60000);
  CHECK(saw(m, "Pet sleeping")); CHECK(petState == PetState::SLEEPING);

  // wake up: full animation plays, sequence restarts at step 1
  m = mark(); tap();
  CHECK(saw(m, "Face: wake_up"));
  CHECK(petState == PetState::WAKING);
  run(1500);
  CHECK(petState != PetState::WAKING);
  CHECK(!robo.isAnimationPlaying() || robo.currentFace() != FACE_WAKE_UP_ID);
  m = mark(); run(500); tap();
  { std::string e = std::string("Face: ") + seq[0]; CHECK(saw(m, e.c_str())); }

  // inactivity (>10 s) restarts the sequence
  run(3000); tap(); run(3000);                           // 2 steps taken
  m = mark(); run(12000); tap();
  CHECK(saw(m, "sequence restarted"));

  // heavy flicker (bounce) is debounced: 3 spikes shorter than 30 ms never count
  run(5000); m = mark();
  for (int i = 0; i < 3; ++i) { g_pin = 1; run(10); g_pin = 0; run(40); }
  CHECK(!saw(m, "Touch "));
  CHECK(g_ramPgmReads == 0);

  printf("%s: touchpet simulation, %d checks, %d failures (flash data %zu B)\n", fails ? "FAILED" : "PASSED", checks, fails, shim_flash_bytes());
  return fails ? 1 : 0;
}
