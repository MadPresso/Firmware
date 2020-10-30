#pragma once

#define DEPTH 3600

class TimeSeries {
private:
  int *timestamps;
  float *values;
  int writePos;

public:
  TimeSeries();
  ~TimeSeries();

  void push(float value);
  void toJson(String &s);
};
