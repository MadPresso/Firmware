#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Boiler.h"

Boiler::Boiler(BoilerParameters *params, float startTemperature, float ambientTemperature) :
  params(params),
  ambientTemperature(ambientTemperature),
  boilerTemperature(startTemperature),
  waterTemperature(startTemperature),
  machineTemperature(startTemperature) {
    heaterAccumulator = new Accumulator(30, 0);
}

void Boiler::tick() {
  const float waterHeatCapacity = 4.184;

  float waterBoilerDeltaTemperature = waterTemperature - boilerTemperature;
  float boilerMachineDeltaTemperature = boilerTemperature - machineTemperature;

  float heaterPower = ((params->heaterPower / 1000.0) * (heaterPowerPercentage / 100.0));
  heaterAccumulator->push(heaterPower);

  waterTemperature += heaterAccumulator->mean() / (params->waterVolume * waterHeatCapacity);

  // Heat transfer water -> boiler
  float transferPower = waterBoilerDeltaTemperature * params->boilerConstant;
  waterTemperature -= transferPower / (params->waterVolume * waterHeatCapacity);
  boilerTemperature += transferPower / (params->boilerWeight * params->boilerMaterialHeatCapacity);

  // Heat transfer boiler -> machine
  transferPower = boilerMachineDeltaTemperature * params->machineConstant;
  boilerTemperature -= transferPower / (params->boilerWeight * params->boilerMaterialHeatCapacity);
  machineTemperature += transferPower * std::exp(-0.1); //?

  // Cooling
  boilerTemperature = ambientTemperature + (boilerTemperature - ambientTemperature) * params->boilerCoolingFactor;
  machineTemperature = ambientTemperature + (machineTemperature - ambientTemperature) * params->machineCoolingFactor;
}
