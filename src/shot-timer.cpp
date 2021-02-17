#include <Arduino.h>
#include <limits.h>

#include "shot-timer.h"

ShotTimer::ShotTimer(Config *config, PIDController *pid) : config(config), pid(pid) {
  stop();
}

void ShotTimer:: start() {
  if (config->pumpControlEnabled && config->pumpControlSeconds > 0) {
    float start = (config->pumpControlPercentageStart * 255.0) / 100.0;
    float end = (config->pumpControlPercentageEnd * 255.0) / 100.0;

    pumpDeltaPerSecond = (end - start) / config->pumpControlSeconds;
    pumpCurrent = start;
  } else {
    pumpCurrent = 255;
  }

  if (config->preInfusionEnabled) {
    preInfusionPumpSeconds = config->preInfusionPumpSeconds;
    preInfusionPauseSeconds = config->preInfusionPauseSeconds;
    setValve(config->preInfusionValveClosed);
  } else {
    setValve(true);
  }

  setPumpPower(pumpCurrent);

  ticker.tick();
  ticker.reset();
  durationSeconds = 0;
  active = true;
}

void ShotTimer:: stop() {
  setValve(false);
  setPumpPower(0);
  pid->reset();
  active = false;
}

void ShotTimer:: tick() {
  if (!active)
    return;

  ticker.tick();
  float dt = ticker.elapsed() / 1000.0;

  if (dt < 0.1)
    return;

  ticker.reset();
  durationSeconds += dt;

  Serial.printf("duration %.2f\n", durationSeconds);

  if (config->shotTimerEnabled && durationSeconds > config->shotTimerSeconds) {
    stop();
    return;
  }

  if (config->preInfusionEnabled) {
    if (preInfusionPumpSeconds > 0) {
      preInfusionPumpSeconds -= dt;
      return;
    }

    if (preInfusionPauseSeconds > 0) {
      setPumpPower(0);
      preInfusionPauseSeconds -= dt;
      return;
    }

    setValve(true);
  }

  if (config->pumpControlEnabled && durationSeconds < config->shotTimerSeconds)
    pumpCurrent += pumpDeltaPerSecond * dt;

  setPumpPower(pumpCurrent);
}
