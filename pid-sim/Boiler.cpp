#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Boiler.h"
#include <cmath>

Boiler::Boiler(BoilerParameters *params, float startTemperature, float ambientTemperature) :
  params(params),
  ambientTemperature(ambientTemperature),
  boilerTemperature(startTemperature),
  waterTemperature(startTemperature),
  machineTemperature(startTemperature) {
    heaterAccumulator = new Accumulator(30, 0);
    boilerAccumulator = new Accumulator(2, startTemperature);
}

//void Boiler::setTemperature(float t) {
//  temperature = t;
//  temperature1 = t;
//  temperature2 = t;
//}

void Boiler::tick() {
  const float waterHeatCapacity = 4.184;
  const float boilerConstant = .015;
  const float machineConstant = .0025;

  float waterBoilerDeltaTemperature = waterTemperature - boilerTemperature;
  float boilerMachineDeltaTemperature = boilerTemperature - machineTemperature;

  float heaterPower = ((params->heaterPower / 1000.0) * (heaterPowerPercentage / 100.0));
  heaterAccumulator->push(heaterPower);

  waterTemperature += heaterAccumulator->mean() / (params->waterVolume * waterHeatCapacity);

  // Heat transfer water -> boiler
  float transferPower = waterBoilerDeltaTemperature * boilerConstant;
  waterTemperature -= transferPower / (params->waterVolume * waterHeatCapacity);
  boilerTemperature += transferPower / (params->boilerWeight * params->boilerMaterialHeatCapacity);

  // Heat transfer boiler -> machine
  transferPower = boilerMachineDeltaTemperature * machineConstant;
  boilerTemperature -= transferPower / (params->boilerWeight * params->boilerMaterialHeatCapacity);
  machineTemperature += transferPower * std::exp(-0.3);

  // Cooling
  boilerTemperature = ambientTemperature + (boilerTemperature - ambientTemperature) * std::exp(-0.0018);


  // boilerAccumulator->push(boilerTemperature);

  // temperature1 += sum * 0.005;
  // temperature2 += sum * 0.002;

  // y = 98.96692 - (0.03921549/0.0005393714)*(1 - e^(-0.0005393714*x))
  // temperature -= (0.03921549/0.0005393714)*(1 - std::exp(-0.0005393714));
  //float delta = (temperature - ambientTemperature);
  // temperature = delta * 0.9998 * (std::exp(-delta * 0.0000044));
  // temperature = (101.3-19.2)*std::exp(-0.00032 * time);

  // boilerTemperature = (boilerTemperature - ambientTemperature) * std::exp(-0.0027);
  // boilerTemperature += ambientTemperature;

  // waterTemperature = (waterTemperature - ambientTemperature) * std::exp(-0.00027);
  // waterTemperature += ambientTemperature;

  // temperature = (temperature1 + temperature2 * 2) / 3;
  //  temperature -= (temperature - ambientTemperature) / isolation;
}
