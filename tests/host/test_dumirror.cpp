// Host-side logic tests for Dumirror_Robo (see README in tests/).
// Unity build: the library .cpp is included so internal tables are testable.
// These tests exercise logic against an emulated AVR flash address space.
// They are NOT a substitute for compiling/running on a real board.
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string>
#include "shim/Arduino.h"
#include "shim/Wire.h"
#include "shim/Adafruit_SSD1306.h"
#define private public
#include "../../src/Dumirror_Robo.cpp"
#undef private

size_t shim_flash_bytes();
static int g_fail = 0, g_checks = 0;
#define CHECK(c) do { ++g_checks; if (!(c)) { ++g_fail; printf("  FAIL %s:%d  %s\n", __FILE__, __LINE__, #c); } } while (0)
using D = Adafruit_SSD1306;

static void reset(DumirrorRobo& r) {
  g_millis = 1000; g_ramPgmReads = 0;
  D::displayCalls = D::drawCalls = D::clearCalls = 0; D::lastBitmap = nullptr; D::beginResult = true;
  Wire = TwoWire();
  CHECK(r.begin());
}
static bool inFlash(const void* p) { return (const char*)p >= __start_progmem_data && (const char*)p < __stop_progmem_data; }

// Expected timeline of each built-in animation (from the original headers).
struct Exp { const char* name; uint8_t frames; bool loop; uint16_t dur[6]; };
static const Exp EXPECT[] = {
#if DUMIRROR_ENABLE_FACE_SNEEZE
  {"sneeze", 6, false, {300,200,150,250,200,200}},
#endif
#if DUMIRROR_ENABLE_FACE_BLINK
  {"blink", 2, true, {1900,130}},
#endif
#if DUMIRROR_ENABLE_FACE_LOOK_LEFT
  {"look_left", 3, false, {200,500,200}},
#endif
#if DUMIRROR_ENABLE_FACE_LOOK_RIGHT
  {"look_right", 3, false, {200,500,200}},
#endif
#if DUMIRROR_ENABLE_FACE_SMILE_BIG
  {"smile_big", 3, false, {250,250,600}},
#endif
#if DUMIRROR_ENABLE_FACE_HEARTBEAT
  {"heartbeat", 2, true, {450,200}},
#endif
#if DUMIRROR_ENABLE_FACE_SLEEP_BREATH
  {"sleep_breath", 2, true, {700,700}},
#endif
#if DUMIRROR_ENABLE_FACE_WAKE_UP
  {"wake_up", 4, false, {350,250,300,400}},
#endif
#if DUMIRROR_ENABLE_FACE_HICCUP
  {"hiccup", 3, false, {300,120,300}},
#endif
#if DUMIRROR_ENABLE_FACE_CHEW
  {"chew", 3, true, {250,250,250}},
#endif
  {nullptr, 0, false, {0}}
};

static void test_registry() {
  printf("[registry] faces=%u static=%u anims=%u flashData=%zu bytes\n",
         (unsigned)FACE_COUNT, (unsigned)FACE_STATIC_COUNT, (unsigned)ANIM_COUNT, shim_flash_bytes());
  DumirrorRobo r; reset(r);
  std::string seen;
  for (uint8_t i = 0; i < FACE_COUNT; ++i) {
    const char* n = r.getName(i);
    CHECK(n && *n);
    CHECK(r.faceIndex(n) == i);                        // name <-> index round trip
    CHECK(seen.find(std::string("|") + n + "|") == std::string::npos);  // unique
    seen += std::string("|") + n + "|";
    const uint8_t* bm = r.getBitmap(i);
    CHECK(bm && inFlash(bm));                          // bitmaps are flash resident
    CHECK(strlen(n) <= DUMIRROR_MAX_NAME_LEN);
    CHECK(strcmp_P(n, (const char*)r.getNameP(i)) == 0);
  }
  CHECK(r.getName(FACE_COUNT) == nullptr);
  CHECK(r.getName(255) == nullptr);
  CHECK(r.getBitmap(FACE_COUNT) == nullptr);
  CHECK(r.getNameP(FACE_COUNT) == nullptr);
  CHECK(r.faceIndex(nullptr) == -1);
  CHECK(r.faceIndex("") == -1);
  CHECK(r.faceIndex("no_such_face") == -1);
  CHECK(r.faceIndex("norma") == -1);
  CHECK(r.faceIndex("normalx") == -1);
  CHECK(r.faceIndex("NORMAL") == -1);
  CHECK(g_ramPgmReads == 0);
}

static void test_invalid_inputs() {
  DumirrorRobo r; reset(r);
  CHECK(!r.showFaceByName("nope")); CHECK(r.lastError() == DUMIRROR_ERR_INVALID_FACE);
  CHECK(!r.showFaceByName(nullptr));
  CHECK(!r.showFace(FACE_COUNT)); CHECK(!r.showFace(200));
  CHECK(r.showFace(0)); CHECK(r.lastError() == DUMIRROR_OK);
  Animation bad1 = {nullptr, 3, false, false};
  r.playAnimation(bad1); CHECK(r.lastError() == DUMIRROR_ERR_INVALID_ANIMATION); CHECK(!r.isAnimationPlaying());
  static const Frame f[] = {{FACE_NORMAL, 100}};
  Animation bad2 = {f, 0, false, false};
  r.playAnimation(bad2); CHECK(r.lastError() == DUMIRROR_ERR_INVALID_ANIMATION);
  // before begin
  DumirrorRobo u; CHECK(!u.showFace(0)); CHECK(u.lastError() == DUMIRROR_ERR_NOT_STARTED);
  u.updateAnimation(); u.display(); u.showText("x"); u.clearText();        // must not crash
  // display missing
  DumirrorRobo m; D::beginResult = false; CHECK(!m.begin()); CHECK(m.lastError() == DUMIRROR_ERR_DISPLAY_NOT_FOUND);
  D::beginResult = true;
  // config mismatch guard
  DumirrorRobo c; CHECK(!c._begin(FACE_COUNT + 1)); CHECK(c.lastError() == DUMIRROR_ERR_CONFIG_MISMATCH);
}

static void test_builtin_animations() {
  for (const Exp* e = EXPECT; e->name; ++e) {
    DumirrorRobo r; reset(r);
    CHECK(r.showFaceByName(e->name));
    CHECK(r.isAnimationPlaying());
    CHECK(D::drawCalls == 1);                                   // frame 1 drawn immediately
    const uint8_t* first = D::lastBitmap;
    CHECK(first == r.getBitmap(r.faceIndex(e->name)));
    unsigned long t0 = g_millis;
    uint8_t idx = 0;
    for (int cycle = 0; cycle < (e->loop ? 3 : 1); ++cycle) {
      for (idx = 0; idx < e->frames; ++idx) {
        // one ms early: must not advance
        g_millis = t0 + e->dur[idx] - 1;
        unsigned long before = D::displayCalls; const uint8_t* shown = D::lastBitmap;
        r.updateAnimation();
        CHECK(D::displayCalls == before); CHECK(D::lastBitmap == shown);
        CHECK(r.isAnimationPlaying());
        // exactly on time: advance
        g_millis = t0 + e->dur[idx];
        r.updateAnimation();
        t0 = g_millis;
      }
      if (!e->loop) break;
      CHECK(r.isAnimationPlaying());
    }
    if (!e->loop) {
      CHECK(!r.isAnimationPlaying());                           // one-shot finished
      unsigned long calls = D::displayCalls;
      g_millis += 100000; r.updateAnimation(); r.updateAnimation();
      CHECK(D::displayCalls == calls);                          // nothing redrawn afterwards
    } else {
      CHECK(r.isAnimationPlaying());
    }
    CHECK(g_ramPgmReads == 0);
    printf("[anim] %-12s frames=%u loop=%d ok\n", e->name, e->frames, e->loop);
  }
}

static void test_engine_edge_cases() {
  DumirrorRobo r; reset(r);
  // late update advances one frame only (no burst)
  CHECK(r.showFaceByName("blink") || true);
  if (r.faceIndex("blink") >= 0) {
    unsigned long d = D::drawCalls; g_millis += 60000; r.updateAnimation();
    CHECK(D::drawCalls == d + 1);
  }
  // RAM-resident custom animation
  static const Frame ramF[] = {{FACE_NORMAL, 50}, {FACE_NORMAL, 70}};
  reset(r);
  Animation a = {ramF, 2, false, false};
  r.playAnimation(a);
  // identical consecutive bitmaps are not redrawn
  CHECK(D::drawCalls == 1); g_millis += 50; r.updateAnimation(); CHECK(D::drawCalls == 1);
  g_millis += 70; r.updateAnimation(); CHECK(!r.isAnimationPlaying());
  // PROGMEM custom animation, temporary descriptor
  static const Frame flF[] PROGMEM = {{FACE_NORMAL, 40}, {FACE_LOVE, 40}};
  reset(r);
  { Animation tmp = {flF, 2, true, true}; r.playAnimation(tmp); tmp.frames = nullptr; tmp.count = 0; }
  CHECK(r.isAnimationPlaying()); g_millis += 40; r.updateAnimation();
  CHECK(D::lastBitmap == FACE_LOVE); g_millis += 40; r.updateAnimation(); CHECK(D::lastBitmap == FACE_NORMAL);
  CHECK(g_ramPgmReads == 0);
  // stop keeps frame, restart works
  r.stopAnimation(); CHECK(!r.isAnimationPlaying()); unsigned long c = D::displayCalls;
  g_millis += 1000; r.updateAnimation(); CHECK(D::displayCalls == c);
  r.playAnimation(Animation{flF, 2, true, true}); CHECK(r.isAnimationPlaying());
  // interruption by a static face
  r.showFace(0); CHECK(!r.isAnimationPlaying());
  // interruption by text
  r.playAnimation(Animation{flF, 2, true, true}); r.showCenteredText("hi"); CHECK(!r.isAnimationPlaying());
  CHECK(strcmp(D::lastText, "hi") == 0);
  // re-showing the same face after text redraws it
  unsigned long dc = D::drawCalls; r.showFace(0); CHECK(D::drawCalls == dc + 1);
  r.showFace(0); CHECK(D::drawCalls == dc + 1);            // but not twice in a row
  // text colour honoured
  r.setTextColor(SSD1306_BLACK); r.showText("a"); CHECK(D::lastColor == SSD1306_BLACK);
  // update() cycles
  reset(r); r.showFace(0); g_millis += 1000; r.update(1000); CHECK(r.currentFace() == 1 % FACE_COUNT);
  // oled() invalidates cache
  reset(r); r.showFace(0); r.oled(); dc = D::drawCalls; r.showFace(0); CHECK(D::drawCalls == dc + 1);
}

static void test_i2c() {
  DumirrorRobo d; reset(d);
#if defined(ARDUINO_ARCH_ESP32) || defined(ARDUINO_ARCH_ESP8266)
  CHECK(Wire.beginCalls == 1 && Wire.beginPinCalls == 0);            // default pins
  DumirrorRobo p(5, 6, 0x3D); reset(p);
  CHECK(Wire.beginPinCalls == 1 && Wire.sda == 5 && Wire.scl == 6); CHECK(D::lastAddr == 0x3D);
  printf("[i2c] ESP-style pin selection ok\n");
#elif defined(ARDUINO_ARCH_RP2040)
  DumirrorRobo p(4, 5); reset(p); CHECK(Wire.sdaSet == 4 && Wire.sclSet == 5);
  reset(d); CHECK(Wire.sdaSet == -1 && Wire.sclSet == -1);
  printf("[i2c] RP2040-style setSDA/setSCL ok\n");
#else
  DumirrorRobo p(5, 6); reset(p); CHECK(Wire.beginCalls == 1 && Wire.beginPinCalls == 0);  // pins ignored on fixed-pin cores
  printf("[i2c] fixed-pin core: plain Wire.begin() ok\n");
#endif
}

static void test_profile_expectations() {
  DumirrorRobo r; reset(r);
#if defined(DUMIRROR_PROFILE_FULL)
  CHECK(FACE_COUNT == 29); CHECK(shim_flash_bytes() >= 33792);
  CHECK(r.faceIndex("sad") >= 0); CHECK(r.faceIndex("chew") >= 0);
#elif defined(DUMIRROR_PROFILE_COMPACT)
  CHECK(FACE_COUNT == 13); CHECK(r.faceIndex("sad") == -1); CHECK(r.faceIndex("sneeze") >= 0);
  CHECK(r.faceIndex("dizzy") >= 0 && r.faceIndex("wake_up") >= 0);
#elif defined(DUMIRROR_PROFILE_MINIMAL)
  CHECK(FACE_COUNT == 12); CHECK(r.faceIndex("sneeze") == -1); CHECK(r.faceIndex("wake_up") >= 0); CHECK(r.faceIndex("dizzy") >= 0);
#endif
}

// Negative control: prove the harness really catches a direct PROGMEM read.
static void test_harness_control() {
  pid_t pid = fork();
  if (pid == 0) { const volatile uint8_t* volatile p = (const volatile uint8_t*)__start_progmem_data + shim_flash_bytes() / 2; volatile uint8_t v = *p; (void)v; _exit(0); }
  int st = 0; waitpid(pid, &st, 0);
  CHECK(WIFSIGNALED(st));                                  // must have crashed
  unsigned long before = g_ramPgmReads; uint8_t ramv[2] = {1, 2}; (void)pgm_read_byte(ramv);
  CHECK(g_ramPgmReads == before + 1);
  printf("[harness] direct PROGMEM deref crashes (%s); pgm_read on RAM counted\n", WIFSIGNALED(st) ? "yes" : "NO");
}

int main() {
  test_harness_control();
  test_registry();
  test_invalid_inputs();
  test_builtin_animations();
  test_engine_edge_cases();
  test_i2c();
  test_profile_expectations();
  printf("%s: %d checks, %d failures\n", g_fail ? "FAILED" : "PASSED", g_checks, g_fail);
  return g_fail ? 1 : 0;
}
