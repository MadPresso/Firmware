#pragma once

#define TRIAC_TICKS 256

class TriacOutput {
private:
  int gpio;
  unsigned int counter;
  unsigned int value;
  uint8_t currentPower;

public:
  TriacOutput(int gpio);

  // zeroCross() is expected to be called on both edges of the AC zero-crossing detection.
  // The rising edge is expected shortly before the crossing happens, and the rising one shortly after.
  void zeroCross(bool rising);

  // timerHandler() must be called roughly 256 times between the falling and the rising edge of the zero detection
  void timerHandler();

  // setValue() sets the triac output as float value between 0.f and 1.f
  void setPower(uint8_t power);
  uint8_t getPower() { return currentPower; }
};
