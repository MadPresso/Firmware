#pragma once

#include <Arduino.h>

class PIDController {
private:
  unsigned long prevMillis;
  float integral;
  float prevError;
  float outputScale;
  float target;
  float kp, ki, kd;

public:
  PIDController(float outputScale);

  void reset();
  void setTarget(float target);
  float currentTarget() { return target; }
  int compute(float measured);
  void setParams(float _kp, float _ki, float _kd);
};
