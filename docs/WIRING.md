# Wiring guide

All examples use an **SSD1306 128x64 I2C OLED** (address `0x3C`, some modules
`0x3D`) and, for TouchPet, a **TTP223 capacitive touch module**.

> **Source of the pin data.** The default I2C pins below are the defaults of
> the respective Arduino cores/variants (and the ATmega hardware TWI pins on
> AVR). They were written from the core definitions as known when this
> release was prepared; the build environment had **no internet access**, so
> they were **not re-checked against live board documentation**. Always
> compare with the pinout of *your exact board* (clone boards differ,
> especially ESP32-C3/S2/S3 "mini" boards).
> Nothing in this guide has been verified on physical hardware.

## 1. I2C pin table

"Default" = what `DumirrorRobo robo;` uses. "Configurable" = you may pass other
pins as `DumirrorRobo robo(sda, scl);`.

| Board | OLED SDA | OLED SCL | Pins configurable? | Logic voltage |
|---|---|---|---|---|
| ESP32 (classic) | GPIO 21 | GPIO 22 | Yes | 3.3 V |
| ESP32-S2 | GPIO 8 | GPIO 9 | Yes | 3.3 V |
| ESP32-S3 | GPIO 8 | GPIO 9 | Yes | 3.3 V |
| ESP32-C3 | GPIO 8 | GPIO 9 | Yes | 3.3 V |
| Arduino Uno R3 | A4 | A5 | **No - fixed hardware pins** | 5 V |
| Arduino Nano Classic | A4 | A5 | **No - fixed** | 5 V |
| Arduino Mega 2560 | D20 | D21 | **No - fixed** | 5 V |
| Arduino Leonardo | D2 | D3 | **No - fixed** | 5 V |
| ESP8266 (NodeMCU) | GPIO 4 (D2) | GPIO 5 (D1) | Yes | 3.3 V |
| RP2040 (Pico) | GP4 | GP5 | Yes, arduino-pico core (valid I2C0 pairs) | 3.3 V |

Board-specific cautions:

* **Classic ESP32:** do not use GPIO 8/9 (connected to the flash chip). The
  constructor default now selects GPIO 21/22 automatically.
* **ESP32-C3:** GPIO 8 and 9 are strapping pins. I2C modules with ordinary
  pull-ups work, but do not add external pull-downs on them.
* **Leonardo:** D2/D3 are the I2C pins, so TouchPet uses **D4** for the touch
  sensor on all AVR boards.
* **RP2040:** only the earlephilhower *arduino-pico* core is covered by the
  pin-selection code. The Arduino *mbed* RP2040 core uses plain `Wire.begin()`.

## 2. TouchPet connections

| Signal | ESP32 family | Uno / Nano | Mega 2560 | Leonardo | ESP8266 NodeMCU | RP2040 Pico |
|---|---|---|---|---|---|---|
| OLED VCC | 3V3 | 5 V* | 5 V* | 5 V* | 3V3 | 3V3(OUT) |
| OLED GND | GND | GND | GND | GND | GND | GND |
| OLED SDA | see table 1 | A4 | D20 | D2 | D2 (GPIO 4) | GP4 |
| OLED SCL | see table 1 | A5 | D21 | D3 | D1 (GPIO 5) | GP5 |
| TTP223 VCC | 3V3 | 5 V | 5 V | 5 V | 3V3 | 3V3(OUT) |
| TTP223 GND | GND | GND | GND | GND | GND | GND |
| TTP223 OUT | GPIO 4 | D4 | D4 | D4 | D5 (GPIO 14) | GP15 |

\* see section 3.

These touch pins are exactly the `TOUCH_PIN` values in `TouchPet.ino`
(`4` on ESP32 and AVR, `14` on ESP8266, `15` on RP2040).

## 3. Electrical safety

* **Do not assume your OLED tolerates 5 V.** The SSD1306 controller itself is a
  3.3 V part. Many 4-pin modules add a regulator and accept 3.3-5 V on VCC;
  some do not. Read the module's description.
* **Pull-ups follow VCC.** The module's SDA/SCL pull-up resistors connect to the
  module's VCC. If you power the OLED from 5 V, SDA/SCL idle at 5 V - fine for a
  5 V board, **dangerous for a 3.3 V board** (ESP32/ESP8266/RP2040 GPIOs are not
  5 V tolerant). On 3.3 V boards power the OLED from 3.3 V.
* **5 V board + 3.3 V-only OLED:** use a bidirectional I2C level shifter, or a
  module designed for 5 V.
* **TTP223:** supply 2.0-5.5 V. Its OUT pin swings to *its supply voltage*, so
  on 3.3 V boards power the TTP223 from 3.3 V as well.
* **Polarity:** the default TTP223 output is active HIGH. If you changed its
  solder jumpers to active LOW, set `TOUCH_ACTIVE_HIGH = false` in the sketch.
* All modules must share a **common GND** with the board.
* The Uno's 3.3 V pin is limited (about 50 mA); the OLED alone is fine, but do
  not power additional loads from it.

## 4. Diagrams

* `images/wiring/wiring_esp32s3_ssd1306_ttp223.svg` - ESP32-S3 (GPIO 8/9, touch GPIO 4)
* `images/wiring/wiring_uno_ssd1306_ttp223.svg` - Arduino Uno R3 (A4/A5, touch D4)

They show exactly the pins used by `examples/TouchPet/TouchPet.ino`.
The SVGs were generated and validated as XML but could not be rendered to an
image in the build environment.
