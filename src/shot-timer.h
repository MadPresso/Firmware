#pragma once

#include "config.h"
#include "ac-power.h"
#include "ticker.h"
class ShotTimer {
public:
  ShotTimer(Config *config);
  void start();
  void stop();
  void tick();
  bool isActive() { return active; }

private:
  Config *config;
  Ticker ticker;
  bool active;
  float pumpDeltaPerSecond;
  float pumpCurrent;
  float durationSeconds;
  float preInfusionPumpSeconds;
  float preInfusionPauseSeconds;
};
