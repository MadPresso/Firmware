#include "Arduino.h"
#include "accumulator.h"

#pragma once

struct BoilerParameters {
  float waterVolume; // Liters
  float boilerWeight; // kg
  float boilerMaterialHeatCapacity; // KJ/(kg*K)
  float heaterPower; // W
};

class Boiler
{
private:
  BoilerParameters *params;
  float heaterPowerPercentage;
  float ambientTemperature;

  float boilerTemperature;
  float waterTemperature;
  float machineTemperature;

  Accumulator *heaterAccumulator;
  Accumulator *boilerAccumulator;

public:
  Boiler(BoilerParameters *params, float startTemperature, float ambientTemperature);
  void tick();
  void setambientTemperature(float celcius) { ambientTemperature = celcius; }
  void setHeaterPowerPercentage(float perc) { heaterPowerPercentage = perc; }

  float getTemperature() { return boilerTemperature; } // boilerAccumulator->mean();
  float getWaterTemperature() { return waterTemperature; }
};
