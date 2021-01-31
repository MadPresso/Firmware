#pragma once

#include "Arduino.h"
#include "accumulator.h"
#include <cmath>

struct BoilerParameters {
  // Volume of the water reservoir in litres
  float waterVolume;
  // Weight of the boiler, in kg
  float boilerWeight;
  // Specfic heat capacity coefficient of the boiler material, in KJ/(kg*K)
  float boilerMaterialHeatCapacity;
  // Power of the heating element, in Watts
  float heaterPower;
  // Numeric constants that define the rates at which heat dissipates from the
  // water reservoir to the boiler, and from the boiler to the rest of the
  // machine, respectively.
  float boilerConstant;
  float machineConstant;
  // Numeric constants that define the cool-down effect of the boiler and the
  // machine, respectively.
  float boilerCoolingFactor;
  float machineCoolingFactor;
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

public:
  Boiler(BoilerParameters *params, float startTemperature, float ambientTemperature);
  void tick();
  void setambientTemperature(float celcius) { ambientTemperature = celcius; }
  void setHeaterPowerPercentage(float perc) { heaterPowerPercentage = perc; }

  float getTemperature() { return boilerTemperature; }
  float getWaterTemperature() { return waterTemperature; }
};
