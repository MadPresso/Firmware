#include <Arduino.h>
#include <ArduinoJson.h>
#include <ESP8266WebServer.h>
#include <LittleFS.h>
#include <Wire.h>

#include "ac-power.h"
#include "config.h"
#include "network-manager.h"
#include "pid-controller.h"
#include "pins.h"
#include "shot-timer.h"
#include "temperature.h"
#include "time-series.h"

static ESP8266WebServer httpServer(80);

static PIDController pid(0, 255);
static TemperatureReader temperatureReader;
//static TimeSeries temperatureHistory;
static Config config;
static NetworkManager networkManager(&config);
static ShotTimer shotTimer(&config);
static Ticker measurementTicker;

void ICACHE_RAM_ATTR shotSwitchHandler(void) {
  bool on = !digitalRead(PIN_SHOT_SENSOR);
  Serial.printf("Shot switch: %d\n", on);

  if (on) {
    pid.setBoostPercentage(config.pidShotBoostPercentage / 100.0);
    shotTimer.start();
  } else {
    pid.setBoostPercentage(0);
    shotTimer.stop();
  }
}

void ICACHE_RAM_ATTR setHeaterTargetTemperature(void) {
  bool steam = digitalRead(PIN_STEAM_SENSOR);

  if (steam)
    pid.setTarget(config.steamTemperature);
  else
    pid.setTarget(config.brewTemperature);
}

void ICACHE_RAM_ATTR steamSwitchHandler(void) {
  Serial.printf("Steam switch: %d\n", digitalRead(PIN_STEAM_SENSOR));
  setHeaterTargetTemperature();
}

// HTTP callbacks

void httpGetConfig(void) {
  String output;
  config.toJson(output);
  httpServer.send(200, "application/json", output);
}

void httpPostConfig(void) {
  const String &body = httpServer.arg("plain");
  bool result = config.fromJson(body);
  
  if (result)
    result = config.write(LittleFS);

  httpServer.send(result ? 200 : 500);
}

void httpGetStatus(void) {
  const size_t CAPACITY = JSON_OBJECT_SIZE(3);
  StaticJsonDocument<CAPACITY> doc;

  JsonObject object = doc.to<JsonObject>();
  object["temperature"] = temperatureReader.current();
  object["targetTemperature"] = pid.currentTarget();
  object["heaterPowerPercentage"] = int(float(getHeaterPower()) / 2.55);

  String output;
  serializeJsonPretty(doc, output);
  httpServer.send(200, "application/json", output);
}

void httpGetTemperatures(void) {
  String *output = new String;
  // temperatureHistory.toJson(*output);
  // httpServer.send(200, "application/json", *output);
  delete output;
}

void setLED(bool on) {
  digitalWrite(PIN_LED, on ? LOW : HIGH);
}

void setup() {
  Serial.begin(115200);
  
  delay(2000);

  Serial.println("CPU Frequency = " + String(F_CPU / 1000000) + " MHz");

  Wire.begin(D2, D1);

  temperatureReader.begin();
  measurementTicker.reset();

  pinMode(PIN_LED, OUTPUT);
  setLED(false);

  setValve(0);
  setHeaterPower(0);

  LittleFS.begin();

  config.read(LittleFS);

  pid.setParams(config.pidP, config.pidI, config.pidD);
  setHeaterTargetTemperature();

  httpServer.on("/api/v1/config", HTTP_GET, httpGetConfig);
  httpServer.on("/api/v1/config", HTTP_POST, httpPostConfig);
  httpServer.on("/api/v1/status", HTTP_GET, httpGetStatus);
  httpServer.on("/api/v1/temperatures", HTTP_GET, httpGetTemperatures);
  httpServer.serveStatic("/", LittleFS, "/webroot/", "");
  httpServer.begin();

  networkManager.begin();

  attachInterrupt(digitalPinToInterrupt(PIN_SHOT_SENSOR), shotSwitchHandler, CHANGE);
  attachInterrupt(digitalPinToInterrupt(PIN_STEAM_SENSOR), steamSwitchHandler, CHANGE);

  initACPower();

  Serial.println("Initialization completed");
}

void loop() {
  networkManager.tick();
  httpServer.handleClient();
  shotTimer.tick();
  temperatureReader.tick();
  measurementTicker.tick();

  if (measurementTicker.elapsed() > 1000) {
//    float heaterValue = pid.compute(temperature);
    float temperature = temperatureReader.current();
    setHeaterPower(0);
    Serial.printf("Temperature %.2f\n", temperature);
    // temperatureHistory.push(temperature);
    measurementTicker.reset();
  }
}
