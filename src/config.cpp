#include <ArduinoJson.h>

#include "config.h"

static const char *filename = "config.json";
static const size_t CAPACITY = JSON_OBJECT_SIZE(16);

Config::Config() {
  // default config
  pidP = 23;
  pidI = 5;
  pidD = 128;
  pollingIntervalMs = 1000;

  brewTemperature = 42.0;
  steamTemperature = 130.0;

  pumpControlEnabled = true;
}

bool ICACHE_RAM_ATTR Config::read(FS &fs) {
  File f = fs.open(filename, "r");

  if (!f.isFile())
    return false;

  String s = f.readString();
  bool ret = fromJson(s);

  f.close();

  return ret;
}

ICACHE_RAM_ATTR bool Config::write(FS &fs) {
  File f = fs.open(filename, "w");

  if (!f.isFile())
    return false;

  String output;
  toJson(output);
  f.write(output.c_str());

  f.close();

  return true;
}

bool Config::fromJson(const String &s) {
  DynamicJsonDocument doc(1024);

  auto error = deserializeJson(doc, s);

  if (error != DeserializationError::Ok) {
    Serial.printf("Cannot deserialize JSON: %s\n", error.c_str());
    return false;
  }

  // pollingIntervalMs = doc["pollingIntervalMs"];

  brewTemperature = doc["brewTemperature"];
  steamTemperature = doc["steamTemperature"];

  shotTimerEnabled = doc["shotTimerEnabled"];
  shotTimerSeconds = doc["shotTimerSeconds"];

  preInfusionEnabled = doc["preInfusionEnabled"];
  preInfusionValveClosed = doc["preInfusionValveClosed"];
  preInfusionPumpSeconds = doc["preInfusionPumpSeconds"];
  preInfusionPauseSeconds = doc["preInfusionPauseSeconds"];

  pumpControlEnabled = doc["pumpControlEnabled"];
  pumpControlPercentageStart = doc["pumpControlPercentageStart"];
  pumpControlPercentageEnd = doc["pumpControlPercentageEnd"];
  pumpControlSeconds = doc["pumpControlSeconds"];

  pidP = doc["pidP"];
  pidI = doc["pidI"];
  pidD = doc["pidD"];
  pidShotBoostPercentage = doc["pidShotBoostPercentage"];

  return true;
}

void Config::toJson(String &s) {
  StaticJsonDocument<CAPACITY> doc;

  JsonObject object = doc.to<JsonObject>();
  // object["pollingIntervalMs"] = pollingIntervalMs;

  object["brewTemperature"] = brewTemperature;
  object["steamTemperature"] = steamTemperature;

  object["shotTimerEnabled"] = shotTimerEnabled;
  object["shotTimerSeconds"] = shotTimerSeconds;

  object["preInfusionEnabled"] = preInfusionEnabled;
  object["preInfusionValveClosed"] = preInfusionValveClosed;
  object["preInfusionPumpSeconds"] = preInfusionPumpSeconds;
  object["preInfusionPauseSeconds"] = preInfusionPauseSeconds;

  object["pumpControlEnabled"] = pumpControlEnabled;
  object["pumpControlPercentageStart"] = pumpControlPercentageStart;
  object["pumpControlPercentageEnd"] = pumpControlPercentageEnd;
  object["pumpControlSeconds"] = pumpControlSeconds;

  object["pidP"] = pidP;
  object["pidI"] = pidI;
  object["pidD"] = pidD;
  object["pidShotBoostPercentage"] = pidShotBoostPercentage;

  serializeJsonPretty(doc, s);
}
