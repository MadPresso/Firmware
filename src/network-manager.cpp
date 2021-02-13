#include <Arduino.h>
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>

#include "network-manager.h"

void NetworkManager::printStatus() {
  switch (WiFi.getMode()) {
    case WIFI_OFF:
      Serial.print("Wifi off.");
      break;

    case WIFI_SHUTDOWN:
      Serial.print("Wifi shut down.");
      break;

    case WIFI_RESUME:
      Serial.print("Wifi resuming.");
      break;

    case WIFI_AP:
      Serial.printf("Soft AP IP: %s\n", WiFi.softAPIP().toString().c_str());
      break;

    case WIFI_STA:
    case WIFI_AP_STA:
      Serial.printf("Connected to Wifi SSID %s\n", WiFi.SSID().c_str());
      Serial.printf("Local IP address: %s\n", WiFi.localIP().toString().c_str());
      break;
  }
}

void ICACHE_RAM_ATTR NetworkManager::autoConnect() {
  dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
  dnsServer.start(53, "*", WiFi.softAPIP());

  WiFi.hostname("MadPresso");
  MDNS.begin("MadPresso");
  // configTime(3 * 3600, 0, "pool.ntp.org", "time.nist.gov");

  printStatus();
}

void NetworkManager::forceAPMode(const String &ssid) {
  WiFi.disconnect();
  WiFi.softAP(ssid);
  WiFi.mode(WIFI_AP);
}

bool NetworkManager::isAPMode() {
  return WiFi.getMode() == WIFI_AP;
}

void NetworkManager::configAsJSON(String &output) {
  DynamicJsonDocument doc(1024);

  doc["mode"] = WiFi.getMode() == WIFI_AP ? "AP" : "STA";
  doc["apSSID"] = WiFi.softAPSSID();
  doc["staSSID"] = WiFi.SSID();

  serializeJsonPretty(doc, output);
}

bool NetworkManager::applyFromJSON(String input) {
   DynamicJsonDocument doc(1024);

  auto error = deserializeJson(doc, input);

  if (error != DeserializationError::Ok) {
    Serial.printf("Cannot deserialize JSON: %s\n", error.c_str());
    return false;
  }

  if (doc["mode"] == "STA") {
    connectToWifi(doc["staSSID"], doc["passphrase"]);
  } else {
    WiFi.mode(WIFI_AP);
    printStatus();
  }

  return true;
}

void NetworkManager::scanWifis(String &output) {
  DynamicJsonDocument doc(1024);
  int n = WiFi.scanNetworks();

  for (int i = 0; i < n; i++) {
    JsonObject w = doc.createNestedObject();
    w["ssid"] = WiFi.SSID(i);
    w["rssi"] = WiFi.RSSI(i);
    w["encrypted"] = WiFi.encryptionType(i) != ENC_TYPE_NONE;
  }

  serializeJsonPretty(doc, output);
}

void NetworkManager::connectToWifi(const String &ssid, const String &passphrase) {
  WiFi.mode(WIFI_STA);
  Serial.printf("Attempting to connect to Wifi SSID %s\n", ssid.c_str());
  WiFi.begin(ssid, passphrase);
  wl_status_t status = wl_status_t(WiFi.waitForConnectResult());
  if (status != WL_CONNECTED) {
    Serial.println("Connection failed. Switching to AP mode");
    WiFi.mode(WIFI_AP);
  }

  MDNS.begin("MadPresso");

  printStatus();
}

void NetworkManager::tick() {
  dnsServer.processNextRequest();
  MDNS.update();
}
