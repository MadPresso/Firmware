#pragma once

class NetworkManager {
public:
  NetworkManager();
  void tick();
  void begin();

private:
  bool wifiConnecting;
};
