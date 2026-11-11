#pragma once
#include "Arduino.h"
struct TwoWire {
  int beginCalls = 0, beginPinCalls = 0, sdaSet = -1, sclSet = -1, sda = -1, scl = -1;
  void begin() { ++beginCalls; }
  void begin(int s, int c) { ++beginPinCalls; sda = s; scl = c; }
  void setSDA(int p) { sdaSet = p; }
  void setSCL(int p) { sclSet = p; }
};
extern TwoWire Wire;
