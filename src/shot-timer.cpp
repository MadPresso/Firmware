#include <Arduino.h>
#include <limits.h>

#include "shot-timer.h"

ShotTimer::ShotTimer(Config *config) : config(config) {
  stop();
}

void ShotTimer:: start() {
  prevMillis = millis();
  durationSeconds = 0;

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
  active = true;
}

void ShotTimer:: stop() {
  setValve(false);
  setPumpPower(0);
  active = false;
}

void ShotTimer:: tick() {
  if (!active)
    return;

  unsigned long now = millis();
  signed long dti;

  // overflow?
  if (now < prevMillis)
    dti = (ULONG_MAX - prevMillis) + now;
  else
    dti = now - prevMillis;

  if (dti < 100)
    return;

  prevMillis = now;
  float dt = dti / 1000.0;
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
