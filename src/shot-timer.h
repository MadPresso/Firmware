#pragma once

#include "config.h"
#include "ac-power.h"

class ShotTimer {
public:
  ShotTimer(Config *config);
  void start();
  void stop();
  void tick();
  bool isActive() { return active; }

private:
  Config *config;
  bool active;
  unsigned long prevMillis;
  float pumpDeltaPerSecond;
  float pumpCurrent;
  float durationSeconds;
  float preInfusionPumpSeconds;
  float preInfusionPauseSeconds;
};
