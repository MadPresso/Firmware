#pragma once

#include "config.h"
#include "ac-power.h"
#include "pid-controller.h"
#include "ticker.h"
class ShotTimer {
public:
  ShotTimer(Config *config, PIDController *pid);
  void start();
  void stop();
  void tick();
  bool isActive() { return active; }

private:
  Config *config;
  PIDController *pid;
  Ticker ticker;
  bool active;
  float pumpDeltaPerSecond;
  float pumpCurrent;
  float durationSeconds;
  float preInfusionPumpSeconds;
  float preInfusionPauseSeconds;
};
