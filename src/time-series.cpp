#include <Arduino.h>
#include <ArduinoJson.h>
#include "time-series.h"

TimeSeries::TimeSeries() {
  timestamps = (int *) malloc(sizeof(int) * DEPTH);
  values = (float *) malloc(sizeof(float) * DEPTH);
  writePos = 0;
}

TimeSeries::~TimeSeries() {
  free(timestamps);
  free(values);
}

void TimeSeries::push(float value) {
  timestamps[writePos] = millis();
  values[writePos] = value;
  writePos++;
  writePos %= DEPTH;
}

void TimeSeries::toJson(String &s) {
  const size_t CAPACITY = JSON_ARRAY_SIZE(DEPTH);
  StaticJsonDocument<CAPACITY> doc;

  JsonArray array = doc.to<JsonArray>();

  for (int i = 0; i < DEPTH; i++) {
    int index = (writePos + i) % DEPTH;

    StaticJsonDocument<JSON_OBJECT_SIZE(2)> entry;
    JsonObject object = entry.to<JsonObject>();
    object["timestamp"] = timestamps[index];
    object["value"] = values[index];

    array.add(object);
  }

  serializeJson(doc, s);
}
