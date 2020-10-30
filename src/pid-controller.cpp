#include "pid-controller.h"
#include <limits.h>

// See https://en.wikipedia.org/wiki/PID_controller

PIDController::PIDController(int min, int max, int scale) {
  outputMin = min;
  outputMax = max;
  outputScale = scale;
  target = 0.f;
  reset();
}

void PIDController::reset() {
  prevMillis = 0;
  prevError = 0;
  prevMillis = millis();
  integral = 0;
}

void ICACHE_RAM_ATTR PIDController::setTarget(float _target) {
  target = _target;
}

void PIDController::setParams(float _kp, float _ki, float _kd) {
  kp = _kp;
  ki = _ki;
  kd = _kd;
}

#define CLAMP(_v,_min,_max) \
  { if ((_v) > (_max))      \
      (_v) = (_max);        \
    else if ((_v) < (_min)) \
      (_v) = (_min);        \
  }

int PIDController::compute(float measured) {
  unsigned long now;
  signed long dt;
  float error, derivative, val;

  now = millis();

  // overflow?
  if (now < prevMillis)
    dt = (ULONG_MAX - prevMillis) + now;
  else
    dt = now - prevMillis;

  prevMillis = now;

  if (dt == 0)
    return 0.f;

  error = target - measured;
  integral += error * dt;
  CLAMP(integral, outputMin, outputMax);

  derivative = (error - prevError) / dt;
  val = (kp * error) + (ki * integral) + (kd * derivative);

//  printf("[measured %f, prevError %f] (kp (%f) * error (%f)) + (ki (%f) * integral (%f)) + (kd (%f) * derivative (%f)) = %f\n",
//         measured, prevError, kp, error, ki, integral, kd, derivative, val);

  val /= outputScale;
  CLAMP(val, outputMin, outputMax);
  prevError = error;

  return (unsigned) val;
}
