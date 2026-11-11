/**
 * DUMIROR - MOCHI FINNISH DINNER DATE
 * A story-telling demo driven from the Serial Monitor (115200 baud).
 * Manual story control, no automatic progression.
 *
 * =====================================================
 * HARDWARE CONNECTIONS
 * =====================================================
 *
 * Target: ESP32-S3 (the story uses the full 29-face set, which only fits on
 * boards with enough flash: ESP32 family, ESP8266, RP2040 or Mega 2560).
 * It does not build for 32 KB AVR boards (Uno/Nano/Leonardo).
 *
 * OLED: SSD1306 128x64, I2C, address 0x3C
 *   OLED SDA -> GPIO 8   (ESP32-S3 default I2C SDA, used here explicitly)
 *   OLED SCL -> GPIO 9   (ESP32-S3 default I2C SCL)
 *   OLED VCC -> 3.3 V    (ESP32 GPIOs are not 5 V tolerant; a 5 V powered
 *                         module would pull SDA/SCL up to 5 V)
 *   OLED GND -> GND
 *
 * On a classic ESP32 use SDA = GPIO 21, SCL = GPIO 22 (GPIO 8/9 are flash
 * pins there): change the two numbers in the DumirrorRobo constructor below.
 * =====================================================
 */

#include <Arduino.h>
#include "Dumirror_Robo.h"

#if !(DUMIRROR_ENABLE_FACE_LAUGH && DUMIRROR_ENABLE_FACE_CRY && DUMIRROR_ENABLE_FACE_KISS && DUMIRROR_ENABLE_FACE_SNEEZE && DUMIRROR_ENABLE_FACE_LOOK_LEFT)
  #error "This story needs the FULL face profile (see Dumirror_Config.h)."
#endif

// -----------------------------------------------------
// HARDWARE
// -----------------------------------------------------

DumirrorRobo mochi(8, 9);


// -----------------------------------------------------
// SCENE STRUCTURE
// -----------------------------------------------------

struct SceneStep {
  const char* content;
  bool isText;
  uint8_t textSize;
  const char* description;
};


// -----------------------------------------------------
// STORY - 13 SCENES
// -----------------------------------------------------

const SceneStep STORY[] = {

  // 1. Vida:
  // Mochi! Guess what?
  // I might be going to Finland!

  {
    "blink", false, 1,
    "Vida: Mochi! Guess what? I might be going to Finland!"
  },

  // 2. Mochi reacts to the news.

  {
    "huh", false, 1,
    "Mochi reacts with surprise."
  },

  // 3. Vida:
  // Yeah! A dinner date!
  // What should I serve?

  {
    "look_left", false, 1,
    "Vida: Yeah! A dinner date! What should I serve?"
  },

  // 4. Mochi:
  // KEBAB?

  {
    "KEBAB?", true, 3,
    "Mochi: KEBAB?"
  },

  // 5. Vida:
  // Kebab? Actually, that's a pretty good idea!
  // See? You're useful sometimes!

  {
    "heartbeat", false, 1,
    "Vida: Kebab? Actually, that's a pretty good idea! See? You're useful sometimes!"
  },

  // 6. Vida:
  // So, are you coming with me?

  {
    "sneeze", false, 1,
    "Vida: So, are you coming with me?"
  },

  // 7. Mochi:
  // I'M TAKEN.

  {
    "I'M TAKEN", true, 2,
    "Mochi: I'M TAKEN."
  },

  // 8. Vida:
  // WHAT?!
  // So... I'm the only single one here?

  {
    "laugh", false, 1,
    "Vida: WHAT?! So... I'm the only single one here?"
  },

  // 9. Vida:
  // OH, REALLY?!
  // Pretend finger poke.

  {
    "dizzy", false, 1,
    "Vida: OH, REALLY?! (Pretend finger poke)"
  },

  // 10. Vida:
  // Okay. I'm not talking to you anymore.

  {
    "cry", false, 1,
    "Vida: Okay. I'm not talking to you anymore."
  },

  // 11. Vida:
  // Don't be sad, Mochi!

  {
  "love", false, 1,
  "Vida: Don't be sad, Mochi!"
  },

  // 12. Mochi sends Vida a kiss.

  {
    "kiss", false, 1,
    "Mochi sends Vida a kiss."
  },

  // 13. Vida:
  // Anyway! I'd serve Persian Kabab Tabei,
  // with grilled tomatoes and saffron rice.
  //
  // It's one of my favourite homemade dishes.
  // I'd love to share a little piece of my culture,
  // and of course, try something Finnish too!
  //
  // He's still upset.

  {
    "love", false, 1,
    "Vida: I'd serve Persian Kabab Tabei with grilled tomatoes and saffron rice. It's one of my favourite homemade dishes. I'd love to share a little piece of my culture, and try something Finnish too! He's still upset."
  }

};


const uint8_t STORY_LENGTH =
  sizeof(STORY) / sizeof(STORY[0]);


// -----------------------------------------------------
// STORY STATE
// -----------------------------------------------------

int currentStep = -1;

bool storyRunning = false;
bool storyFinished = false;


// -----------------------------------------------------
// SHOW SCENE
// -----------------------------------------------------

void showScene(int index) {

  if (index < 0 || index >= STORY_LENGTH) {
    return;
  }

  const SceneStep& scene = STORY[index];

  Serial.println();
  Serial.println("================================");

  Serial.print("SCENE ");
  Serial.print(index + 1);
  Serial.print(" / ");
  Serial.println(STORY_LENGTH);

  Serial.print("ACTION: ");
  Serial.println(scene.description);

  Serial.println("================================");

  if (scene.isText) {

    mochi.showCenteredText(
      scene.content,
      scene.textSize
    );

  } else {

    mochi.showFaceByName(scene.content);

  }
}


// -----------------------------------------------------
// START / RESTART STORY
// -----------------------------------------------------

void startStory() {

  currentStep = 0;

  storyRunning = true;
  storyFinished = false;

  Serial.println();
  Serial.println("================================");
  Serial.println(" MOCHI FINNISH DINNER DATE");
  Serial.println(" STORY STARTED");
  Serial.println("================================");

  showScene(currentStep);
}


// -----------------------------------------------------
// NEXT SCENE
// -----------------------------------------------------

void nextScene() {

  if (!storyRunning) {

    if (storyFinished) {
      Serial.println("Story already completed. Press 1 to replay.");
    } else {
      Serial.println("Story is not running. Press 1 to start.");
    }

    return;
  }

  if (currentStep < STORY_LENGTH - 1) {

    currentStep++;

    showScene(currentStep);

  } else {

    storyRunning = false;
    storyFinished = true;

    Serial.println();
    Serial.println("================================");
    Serial.println(" STORY COMPLETED!");
    Serial.println(" MOCHI SENT A KISS!");
    Serial.println("================================");
    Serial.println("Press 1 to replay.");

  }
}


// -----------------------------------------------------
// PREVIOUS SCENE
// -----------------------------------------------------

void previousScene() {

  if (!storyRunning) {

    Serial.println("Start the story first with 1.");
    return;

  }

  if (currentStep > 0) {

    currentStep--;

    showScene(currentStep);

  } else {

    Serial.println("Already at the first scene.");

  }
}


// -----------------------------------------------------
// MANUAL EMOTION CONTROL
// -----------------------------------------------------

void manualFace(const char* name) {

  storyRunning = false;
  storyFinished = false;

  Serial.print("MANUAL MODE: ");
  Serial.println(name);

  mochi.showFaceByName(name);
}


// -----------------------------------------------------
// HELP
// -----------------------------------------------------

void showHelp() {

  Serial.println();
  Serial.println("========== MOCHI REMOTE ==========");

  Serial.println();
  Serial.println("STORY CONTROL:");
  Serial.println("1 = Start / Restart");
  Serial.println("2 = Next scene");
  Serial.println("3 = Previous scene");
  Serial.println("0 = Stop story");

  Serial.println();
  Serial.println("MANUAL EMOTIONS:");
  Serial.println("b = blink");
  Serial.println("h = huh");
  Serial.println("l = look left");
  Serial.println("r = look right");
  Serial.println("q = heartbeat");
  Serial.println("v = love");
  Serial.println("x = sneeze");
  Serial.println("w = wink");
  Serial.println("j = laugh");
  Serial.println("d = dizzy");
  Serial.println("c = cry");
  Serial.println("k = kiss");

  Serial.println();
  Serial.println("TEXT:");
  Serial.println("4 = KEBAB?");
  Serial.println("5 = I'M TAKEN");

  Serial.println();
  Serial.println("==================================");

}


// -----------------------------------------------------
// SERIAL COMMAND HANDLER
// -----------------------------------------------------

void handleSerial() {

  if (!Serial.available()) {
    return;
  }

  char command = Serial.read();

  if (command == '\n' || command == '\r') {
    return;
  }

  switch (command) {

    // STORY CONTROL

    case '1':
      startStory();
      break;

    case '2':
      nextScene();
      break;

    case '3':
      previousScene();
      break;

    case '0':

      storyRunning = false;
      storyFinished = false;
      currentStep = -1;

      mochi.clearText();

      Serial.println("Story stopped.");
      break;


    // MANUAL EMOTIONS

    case 'b':
      manualFace("blink");
      break;

    case 'h':
      manualFace("huh");
      break;

    case 'l':
      manualFace("look_left");
      break;

    case 'r':
      manualFace("look_right");
      break;

    case 'q':
      manualFace("heartbeat");
      break;

    case 'v':
      manualFace("love");
      break;

    case 'x':
      manualFace("sneeze");
      break;

    case 'w':
      manualFace("wink");
      break;

    case 'j':
      manualFace("laugh");
      break;

    case 'd':
      manualFace("dizzy");
      break;

    case 'c':
      manualFace("cry");
      break;

    case 'k':
      manualFace("kiss");
      break;


    // TEXT CONTROL

    case '4':

      storyRunning = false;
      storyFinished = false;

      mochi.showCenteredText("KEBAB?", 3);
      break;

    case '5':

      storyRunning = false;
      storyFinished = false;

      mochi.showCenteredText("I'M TAKEN", 2);
      break;


    // HELP

    case '?':
      showHelp();
      break;


    default:
      Serial.println("Unknown command. Send ? for help.");
      break;

  }
}


// -----------------------------------------------------
// SETUP
// -----------------------------------------------------

void setup() {

  Serial.begin(115200);

  delay(500);

  Serial.println();
  Serial.println("================================");
  Serial.println(" DUMIRROR MOCHI");
  Serial.println(" FINNISH DINNER DATE");
  Serial.println("================================");

  if (!mochi.begin()) {

    Serial.println("OLED initialization failed!");

    while (true) {
      delay(1000);
    }

  }

  Serial.println("OLED initialized successfully.");

  mochi.showFaceByName("blink");

  Serial.println();
  Serial.println("MOCHI IS READY!");
  Serial.println("Send 1 to start.");
  Serial.println("Send ? for commands.");

}


// -----------------------------------------------------
// MAIN LOOP
// -----------------------------------------------------

void loop() {

  handleSerial();

  // Keep animations running.
  mochi.updateAnimation();

}
