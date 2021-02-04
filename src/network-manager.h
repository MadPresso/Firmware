#pragma once

#include <DNSServer.h>

class NetworkManager {
public:
  void tick();
  void autoConnect(const String &ssid);
  void connectToWifi(const String &ssid, const String &passphrase);
  void forceAPMode();
  bool isAPMode();
  void scanWifis(String &output);
  void configAsJSON(String &output);
  bool applyFromJSON(String input);

private:
  void printStatus();
  DNSServer dnsServer;
};
