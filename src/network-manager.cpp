#include <Arduino.h>
#include <ESP8266WiFi.h>

#include "credentials.h"
#include "network-manager.h"

NetworkManager::NetworkManager() {}

void ICACHE_RAM_ATTR NetworkManager::begin() {
    Serial.println("Connecting to WiFi");
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    wifiConnecting = true;
}

void ICACHE_RAM_ATTR NetworkManager::tick() {
  if (WiFi.status() == WL_CONNECTED && wifiConnecting) {
    Serial.println("WiFi connection successful");
    Serial.print("The IP Address of ESP8266 Module is: ");
    Serial.print(WiFi.localIP());
    wifiConnecting = false;
  }
}
