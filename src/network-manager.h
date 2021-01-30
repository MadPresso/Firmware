#pragma once

#include "config.h"

class NetworkManager {
public:
  NetworkManager(Config *config);
  void tick();
  void begin();

private:
  Config *config;
  bool wifiConnecting;
};
