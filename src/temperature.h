#pragma once

#include <ADC101C.h>
#include "ticker.h"
#include "kalman-filter.h"

class TemperatureReader {
public:
  TemperatureReader();
  void begin();
  void tick();
  float current() { return temperature; }

private:
  float read();
  ADC101C adc;
  bool adcInitialized;
  Ticker ticker;
  KalmanFilter kalman;
  float temperature;
};
