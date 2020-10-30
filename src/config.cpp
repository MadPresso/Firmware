#include <ArduinoJson.h>

#include "config.h"

static const char *filename = "config.json";
static const size_t CAPACITY = JSON_OBJECT_SIZE(6);

Config::Config() {
  // default config
  pidP = 23;
  pidI = 5;
  pidD = 128;
  pollingIntervalMs = 1000;

  brewTemperature = 100.0;
  steamTemperature = 130.0;
}

bool Config::read(FS &fs) {
  File f = fs.open(filename, "r");

  if (!f.isFile())
    return false;

  String s = f.readString();
  bool ret = fromJson(s);

  f.close();

  return ret;
}

bool Config::write(FS &fs) {
//  File f = fs.open(filename, "w");
//
//  if (!f.isFile())
//    return false;
//
//  String output;
//  toJson(output);
//  f.write(output);
//  f.close();
//
  return true;
}

bool Config::fromJson(String &s) {
  StaticJsonDocument<CAPACITY> doc;

  if (deserializeJson(doc, s) != DeserializationError::Ok)
    return false;

  pidP = doc["pidP"];
  pidI = doc["pidI"];
  pidD = doc["pidD"];
  pollingIntervalMs = doc["pollingIntervalMs"];
  brewTemperature = doc["brewTemperature"];
  steamTemperature = doc["steamTemperature"];

  return true;
}

void Config::toJson(String &s) {
  StaticJsonDocument<CAPACITY> doc;

  JsonObject object = doc.to<JsonObject>();
  object["pidP"] = pidP;
  object["pidI"] = pidI;
  object["pidD"] = pidD;
  object["pollingIntervalMs"] = pollingIntervalMs;
  object["brewTemperature"] = brewTemperature;
  object["steamTemperature"] = steamTemperature;

  serializeJsonPretty(doc, s);
}

