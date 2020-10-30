#pragma once

#include <FS.h>

class Config {
public:
  // Machine config
  float pidP, pidI, pidD;
  unsigned int pollingIntervalMs;

  // Shot config
  float brewTemperature;
  float steamTemperature;

  Config();
  bool read(FS &fs);
  bool write(FS &fs);
  bool fromJson(String &s);
  void toJson(String &s);
};
