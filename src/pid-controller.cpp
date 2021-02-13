#include "pid-controller.h"
#include <limits.h>

// See https://en.wikipedia.org/wiki/PID_controller

PIDController::PIDController(int min, int max) {
  target = 0;
  outputMin = min;
  outputMax = max;
  reset();
}

void PIDController::reset() {
  ticker.reset();
  prevError = 0;
  integral = 0;
}

void ICACHE_RAM_ATTR PIDController::setTarget(float celcius) {
  target = celcius;
}

void PIDController::clamp(float *f, float max) {
  if (*f < outputMin)
    *f = outputMin;

  if (*f > max)
    *f = max;
}

void PIDController::setParams(float _kp, float _ki, float _kd, float _integralWindupLimit) {
  kp = _kp;
  ki = _ki;
  kd = _kd;
  integralWindupLimit = _integralWindupLimit;
}

int PIDController::compute(float measured) {
  float dt, error, derivative;
  float val;

  ticker.tick();
  dt = ticker.elapsed() / 1000.0;
  ticker.reset();

  if (dt == 0)
    return outputMin;

  error = target - measured;

  val = kp * error;

  integral += error * dt;
  clamp(&integral, outputMax * 0.6);

  derivative = (error - prevError) / dt;

  val += (ki * integral) + (kd * derivative);
  clamp(&val, outputMax);

//  Serial.printf("[target %.2f measured %.2f, prevError %.2f, integralInit %d] (kp %.2f * error %.2f) + (ki %.2f * integral %.2f) + (kd %.2f * derivative %.2f) = %.2f\n",
//         target, measured, prevError, integralInit, kp, error, ki, integral, kd, derivative, val);

  prevError = error;

  return (unsigned) val;
}
