#pragma once

#include <stdio.h>
#include <stdlib.h>

class Accumulator {
public:
  Accumulator(int depth, float start) : depth(depth) {
    values = (float *) malloc(sizeof(float) * depth);
    for (int i = 0; i < depth; i++)
      values[i] = start;

    index = 0;
  }

  void push(float v) {
    values[index] = v;
    index++;
    index %= depth;
  }

  float mean() {
    float f = 0;

    for (int i = 0; i < depth; i++)
      f += values[i];

    f /= float(depth);
    return f;
  }

private:
  int depth;
  int index;
  float *values;
};
