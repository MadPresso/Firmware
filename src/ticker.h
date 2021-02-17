#pragma once

#include <Arduino.h>
#include <limits.h>

class Ticker {
public:
  Ticker() {
    tickMillis = resetMillis = 0;
  }

  void tick() {
    tickMillis = millis();
  }

  void reset() {
    resetMillis = tickMillis;
  }

  unsigned long elapsed() {
    // overflow?
    if (tickMillis < resetMillis)
     return (ULONG_MAX - resetMillis) + tickMillis;

    return tickMillis - resetMillis;
  }

private:
  unsigned long resetMillis;
  unsigned long tickMillis;
};
