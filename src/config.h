#pragma once

#include <FS.h>

class Config {
public:
  // Machine config
  std::string deviceName;
  unsigned int pollingIntervalSeconds;

  // Shot config
  float brewTemperature;
  float steamTemperature;

  bool shotTimerEnabled;
  float shotTimerSeconds;

  bool preInfusionEnabled;
  bool preInfusionValveClosed;
  float preInfusionPumpSeconds;
  float preInfusionPauseSeconds;

  bool pumpControlEnabled;
  float pumpControlPercentageStart;
  float pumpControlPercentageEnd;
  float pumpControlSeconds;

  float pidP, pidI, pidD;
  float heaterPercentageDuringShot;

  Config();
  bool read(FS &fs);
  bool write(FS &fs);
  bool fromJson(const String &s);
  void toJson(String &s);
};
