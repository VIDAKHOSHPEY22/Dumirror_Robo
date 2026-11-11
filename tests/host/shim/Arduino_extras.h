#pragma once
// Extra Arduino API surface used only by the example syntax check.
#include <stdlib.h>
#include "Arduino.h"
#define HIGH 1
#define LOW 0
#define INPUT 0
#define INPUT_PULLUP 2
#define A0 14
inline int analogRead(int) { return 0; }
inline void pinMode(int, int) {}
inline int digitalRead(int) { return 0; }
inline long random(long m) { return m ? rand() % m : 0; }
inline void randomSeed(unsigned long) {}
struct SerialStub { void begin(long) {} template<class T> void print(T) {} template<class T> void println(T) {} void println() {} int available() { return 0; } int read() { return -1; } };
extern SerialStub Serial;
