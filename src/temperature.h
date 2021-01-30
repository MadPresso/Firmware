#pragma once

#include <ADC101C.h>
#include "ticker.h"

class TemperatureReader {
public:
  TemperatureReader() {}
  void begin();
  void tick();
  float current();

private:
  float read();
  float values[20];
  unsigned int valueIndex = 0;
  ADC101C adc;
  bool adcInitialized;
  bool primed;
  Ticker ticker;
};
