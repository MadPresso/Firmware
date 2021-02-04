#pragma once

#include <Arduino.h>
#include "ticker.h"
class PIDController {
public:
  PIDController(int min, int max);

  void reset();
  void setTarget(float target);
  float currentTarget() { return target; }
  int compute(float measured);
  void setParams(float _kp, float _ki, float _kd, float _integralWindupLimit);

private:
  Ticker ticker;
  float integral;
  float prevError;
  float target;
  float kp, ki, kd;
  float integralWindupLimit;
  int outputScale;
  int outputMin, outputMax;

  bool integralInit;

  void clamp(float *f, float max);
};
