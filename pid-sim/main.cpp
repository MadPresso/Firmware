#include <stdio.h>
#include <fstream>

#include "pid-controller.h"
#include "Boiler.h"
#include "Arduino.h"

unsigned long _millis;

int main(void) {
  _millis = 0;

  PIDController *pid = new PIDController(0, 255);
  //pid->setParams(25, 3, 128);
  pid->setParams(25, 3, 12);
  pid->setTarget(100.0);

  // Values for Ranchilio Siliva

  struct BoilerParameters param = {
    .waterVolume = 0.30,
    .boilerWeight = 1.0,
    .boilerMaterialHeatCapacity = 0.377,
    .heaterPower = 1150, //1150,
  };

  Boiler *b = new Boiler(&param, 22.18, 19.2);

  std::ifstream infile("../data-1.csv");

  int dHeater;
  float dTemp;
  int t = 0;

  // int skip = 180;

  // b->setTemperature(101.3);

  while (infile >> dHeater >> dTemp) {
    // if (--skip > 0)
    //   continue;

    b->tick();

//    int v = pid->compute(t);
//    b->setHeater((float) v);
    b->setHeaterPowerPercentage(float(dHeater)/2.55);
   float sTemp = b->getTemperature();
    printf("%d %.2f %.2f %d %.2f\n", t, sTemp, dTemp, dHeater, b->getWaterTemperature());
    // printf("%d %.2f %d\n", t, dTemp, dHeater);
    // if (t % 1000 == 0)
    //   printf("%d %.2f\n", t, dTemp);

    millis_tick(100);
    t++;

    if (t > 10000)
      break;
  }

  return 0;
}
