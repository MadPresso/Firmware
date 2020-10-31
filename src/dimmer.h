#pragma once

#include <Arduino.h>

class Dimmer {
private:
  float value;
  unsigned int counter;
  unsigned long prevMicros;
  unsigned long timings[10];
  unsigned int timingsTaken;

public:
  int gpio;
  unsigned int avgPeriod;

  Dimmer(int gpio);

  void zeroCross(bool falling);
  void setValue(float value);
  float getValue() { return value; }
  void timerHandler();
};
