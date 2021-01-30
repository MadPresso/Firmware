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
  ki = 0.6;
  kd = 64; //64;
}

void PIDController::setBoostPercentage(float factor) {
  boostPercentage = factor;
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
  if (error > 10) {
    val *= 2;
    integral = 0;
    integralInit = false;
  } else {
    if (integralInit) {
      integral += error * dt;
      clampToOutput(&integral);
    }

    if (error <= 0)
      integralInit = true;

    derivative = (error - prevError) / dt;

    val += (ki * integral) + (kd * derivative);
    val += boostPercentage * float(outputMax);
  }

  clampToOutput(&val);

//  Serial.printf("[target %.2f measured %.2f, prevError %.2f, integralInit %d] (kp %.2f * error %.2f) + (ki %.2f * integral %.2f) + (kd %.2f * derivative %.2f) = %.2f\n",
//         target, measured, prevError, integralInit, kp, error, ki, integral, kd, derivative, val);

  prevError = error;

  return (unsigned) val;
}
