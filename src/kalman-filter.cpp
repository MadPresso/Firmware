#include "kalman-filter.h"

// Code inspired by https://gist.github.com/jannson/9951716

KalmanFilter::KalmanFilter(float q, float r) :
  q(q), r(r), pLast(0), xEstLast(0), primed(false) {}

float KalmanFilter::compute(float input) {
  float k, p, pTemp, xEst;

  if (!primed) {
    xEstLast = input;
    primed = true;
    return input;
  }

  // prediction
  pTemp = pLast + q;

  // calculate the Kalman gain
  k = pTemp / (pTemp + r);

  xEst = xEstLast + k * (input - xEstLast);
  p = (1.0 - k) * pTemp;

  pLast = p;
  xEstLast = xEst;

  return xEst;
}
