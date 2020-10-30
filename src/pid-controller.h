#pragma once

#include <Arduino.h>

class PIDController {
private:
  unsigned long prevMillis;
  float integral;
  float prevError;
  float target;
  float kp, ki, kd;
  float outputMin, outputMax, outputScale;

public:
  PIDController(int outputMin, int outputMax, int outputScale);

  void reset();
  void setTarget(float target);
  float currentTarget() { return target; }
  int compute(float measured);
  void setParams(float _kp, float _ki, float _kd);
};
