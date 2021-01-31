#include <Arduino.h>

#include "temperature.h"

TemperatureReader::TemperatureReader() : kalman(0.1, 10000.0) {}

void TemperatureReader::begin() {
  adcInitialized = adc.begin(&Wire, 0x50);
  
  if (adcInitialized) {
    // adc.setMode(ADC101C_MODE_AUTO_512);
    temperature = kalman.compute(read());
  } else {
    Serial.print("Cannot initialize ADC!");
  }
}

float TemperatureReader::read() {
  if (!adcInitialized)
    return 0;

  uint16_t highest = adc.highest();
  uint16_t lowest = adc.lowest();
  uint16_t value = adc.read(); //(highest + lowest) / 2;
  int mV = map(value, 0, 1023, 0, 3300);

  // LM35DT gives us 10mV/°C
  float f = float(mV) / 10.0;

  return f;
}

// static int t = 0;

void TemperatureReader::tick() { 
  ticker.tick();

  if (ticker.elapsed() >= 50) {
    float v = read();
    temperature = kalman.compute(v);
    // Serial.printf("%d %.2f %.2f\n", t, v, temperature);
    ticker.reset();
    // t++;
  }
}
