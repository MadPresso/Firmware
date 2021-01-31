#include <stdio.h>
#include <fstream>

#include "pid-controller.h"
#include "Boiler.h"
#include "Arduino.h"

unsigned long _millis;

  // Values for Ranchilio Siliva
static struct BoilerParameters param = {
  .waterVolume = 0.3,
  .boilerWeight = 1.0,
  .boilerMaterialHeatCapacity = 0.377, // brass
  .heaterPower = 1150,
  .boilerConstant = 0.019,
  .machineConstant = 0.0025,
  .boilerCoolingFactor = std::exp(-0.0021),
  .machineCoolingFactor = std::exp(-0.00014),
};

void plotMeasurementFile(const char *fileName, float startTemperature, float ambientTemperature) {
  std::ifstream infile(fileName);

  int dHeater;
  float dTemp;
  int t = 0;

  Boiler *b = new Boiler(&param, startTemperature, ambientTemperature);

  // int skip = 180;

  while (infile >> dHeater >> dTemp) {
    // if (--skip > 0)
    //   continue;

    b->tick();
    b->setHeaterPowerPercentage(float(dHeater)/2.55);
    float sTemp = b->getTemperature();
    printf("%d %.2f %.2f %d\n", t, sTemp, dTemp, dHeater);
    // printf("%d %.2f %d\n", t, dTemp, dHeater);
    // if (t % 1000 == 0)
    //   printf("%d %.2f\n", t, dTemp);

    millis_tick(100);
    t++;

    if (t > 2000)
      break;
  }
}

void plotPID() {
  PIDController *pid = new PIDController(0, 255);
  pid->setParams(25, 3, 12);
  pid->setTarget(100.0);

  Boiler *b = new Boiler(&param, 18.0, 18.0);

  for (int t = 0; t < 10000; t++) {
    b->tick();

    if (t == 2000)
      pid->setTarget(110);

    if (t == 4000)
      pid->setTarget(90);

    if (t == 6000)
      pid->setTarget(120);

    if (t == 8000)
      pid->setTarget(80);

    float sTemp = b->getTemperature();
    int v = pid->compute(sTemp);

    b->setHeaterPowerPercentage(((float) v)/2.55);

    printf("%d %.2f %.2f %d\n", t, sTemp, pid->currentTarget(), v);

    millis_tick(1000);
  }
}

int main(void) {
  _millis = 0;

  //plotMeasurementFile("data-1.csv", 22.18, 19.2);
  //plotMeasurementFile("data-on-off-heating.csv", 17.4, 17.4);
  plotPID();

  return 0;
}
