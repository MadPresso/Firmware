#include "pid-controller.h"
#include <limits.h>

// See https://en.wikipedia.org/wiki/PID_controller

PIDController::PIDController() : target(0.f) {
  reset();
}

void PIDController::reset() {
  prevMillis = 0;
  prevError = 0;
  integral = 0;
  prevMillis = millis();
}

void ICACHE_RAM_ATTR PIDController::setTarget(float _target) {
  target = 90.0;
}

void PIDController::clampToOutput(float *f) {
  if (*f < outputMin)
    *f = outputMin;

  if (*f > outputMax)
    *f = outputMax;
}

void PIDController::setParams(float _kp, float _ki, float _kd) {
  // kp = _kp / 100.0;
  // ki = _ki / 100.0;
  // kd = _kd / 100.0;

  kp = 5;
  ki = 0.4;
  kd = 64; //64;

  outputMin = 0;
  outputMax = 255;
}

void PIDController::setBoostPercentage(float factor) {
  bootPercentage = factor;
}

int PIDController::compute(float measured) {
  unsigned long now;
  signed long dti;
  float dt, error, derivative;
  float val;

  now = millis();

  // overflow?
  if (now < prevMillis)
    dti = (ULONG_MAX - prevMillis) + now;
  else
    dti = now - prevMillis;

  prevMillis = now;

  if (dti == 0)
    return outputMin;

  dt = dti / 1000.0;

  error = target - measured;

  val = kp * error;
  if (error > 10) {
    val *= 2;
    integral = 0;
  } else {
    integral += error * dt;
    clampToOutput(&integral);

    derivative = (error - prevError) / dt;

    val += (ki * integral) + (kd * derivative);
    val += bootPercentage * float(outputMax);
  }

  clampToOutput(&val);

  Serial.printf("[target %.2f measured %.2f, prevError %.2f] (kp %.2f * error %.2f) + (ki %.2f * integral %.2f) + (kd %.2f * derivative %.2f) = %.2f\n",
         target, measured, prevError, kp, error, ki, integral, kd, derivative, val);

  prevError = error;

  return (unsigned) val;
}
