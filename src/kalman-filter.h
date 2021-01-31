#pragma once

class KalmanFilter {
  float q, r;
  float pLast;
  float xEstLast;
  bool primed;

public:
  KalmanFilter(float q, float r);
  float compute(float intput);
};
