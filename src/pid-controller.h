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
  void setParams(float _kp, float _ki, float _kd);
  void setBoostPercentage(float factor);

private:
  Ticker ticker;
  float integral;
  float prevError;
  float target;
  float kp, ki, kd;
  float boostPercentage;
  int outputScale;
  int outputMin, outputMax;

  bool integralInit;

  void clampToOutput(float *f, float max);
};
