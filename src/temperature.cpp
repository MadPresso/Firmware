#include <Arduino.h>

#include "temperature.h"

void TemperatureReader::begin() {
  adcInitialized = adc.begin(&Wire, 0x50);
  
  if (!adcInitialized)
    Serial.print("Cannot initialize ADC!");
}

float TemperatureReader::read() {
  if (!adcInitialized)
    return 0;

  uint16_t highest = adc.highest();
  uint16_t lowest = adc.lowest();
  uint16_t value = (highest + lowest) / 2;
  int mV = map(value, 0, 1023, 0, 3300);

  // LM35DT has 10mV/°C
  float f = float(mV) / 10.0;

  return f;
}

float TemperatureReader::current() {
  if (!primed)
    return 0;

  float value = 0;

  for (unsigned int n = 0; n < sizeof(values) / sizeof(values[0]); n++)
    value += values[n];

  value /= sizeof(values) / sizeof(values[0]);

  return value;
}

void TemperatureReader::tick() { 
  ticker.tick();

  if (ticker.elapsed() >= 50) {
    values[valueIndex] = read();
    valueIndex++;
    valueIndex %= sizeof(values) / sizeof(values[0]);

    if (valueIndex == 0)
      primed = true;

    ticker.reset();
  }
}
