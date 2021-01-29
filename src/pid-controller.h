#pragma once

#include <Arduino.h>

class PIDController {
private:
  unsigned long prevMillis;
  float integral;
  float prevError;
  float target;
  float kp, ki, kd;
  float bootPercentage;
  int outputScale;
  int outputMin, outputMax;

  void clampToOutput(float *f);

public:
  PIDController();

  void reset();
  void setTarget(float target);
  float currentTarget() { return target; }
  int compute(float measured);
  void setParams(float _kp, float _ki, float _kd);
  void setBoostPercentage(float factor);
};
