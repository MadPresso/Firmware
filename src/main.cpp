#include <Arduino.h>
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <LittleFS.h>

#include "ac-power.h"
#include "config.h"
#include "credentials.h"
#include "pid-controller.h"
#include "pins.h"
#include "time-series.h"

ESP8266WebServer httpServer(80);

PIDController pid(.05f);
TimeSeries temperatureHistory;
Config config;

float readTemperature() {
  int value = analogRead(PIN_TEMPERATURE_SENSOR);
  int mV = map(value, 0, 1023, 0, 3300);
  // LM35DT has 10mV/°C
  return float(mV) / 10.0;
}

void ICACHE_RAM_ATTR shotSwitchHandler(void) {
  bool on = digitalRead(PIN_SHOT_SENSOR);

  setValve(on);
  setPumpPower(on ? 255 : 0);
}

void ICACHE_RAM_ATTR setHeaterTargetTemperature(void) {
  bool steam = digitalRead(PIN_STEAM_SENSOR);

  if (steam)
    pid.setTarget(config.steamTemperature);
  else
    pid.setTarget(config.brewTemperature);
}

void ICACHE_RAM_ATTR steamSwitchHandler(void) {
  setHeaterTargetTemperature();
}

// HTTP callbacks

void httpGetConfig(void) {
  String output;
  config.toJson(output);
  httpServer.send(200, "application/json", output);
}

void httpPostConfig(void) {
}

void httpGetStatus(void) {
  const size_t CAPACITY = JSON_OBJECT_SIZE(3);
  StaticJsonDocument<CAPACITY> doc;

  JsonObject object = doc.to<JsonObject>();
  object["temperature"] = readTemperature();
  object["targetTemperature"] = pid.currentTarget();
  object["heaterPowerPercentage"] = getHeaterPower() * 100.f;

  String output;
  serializeJsonPretty(doc, output);
  httpServer.send(200, "application/json", output);
}

void httpGetTemperatures(void) {
  String *output = new String;
  temperatureHistory.toJson(*output);
  httpServer.send(200, "application/json", *output);
  delete output;
}

void setup() {
  Serial.begin(115200);
  
  delay(2000);

  Serial.println("CPU Frequency = " + String(F_CPU / 1000000) + " MHz");

  attachInterrupt(digitalPinToInterrupt(PIN_SHOT_SENSOR), shotSwitchHandler, CHANGE);
  attachInterrupt(digitalPinToInterrupt(PIN_STEAM_SENSOR), steamSwitchHandler, CHANGE);

  initACPower();

  setValve(1);
  setHeaterPower(0.5);

  LittleFS.begin();

  config.read(LittleFS);

  pid.setParams(config.pidP, config.pidI, config.pidD);
  setHeaterTargetTemperature();

  httpServer.on("/v1/config", HTTP_GET, httpGetConfig);
  httpServer.on("/v1/config", HTTP_POST, httpPostConfig);
  httpServer.on("/v1/status", HTTP_GET, httpGetStatus);
  httpServer.on("/v1/temperatures", HTTP_GET, httpGetTemperatures);
  httpServer.serveStatic("/", LittleFS, "/webroot/", "");
  httpServer.begin();

  Serial.println("Init completed");
}

static bool wifiConnecting = false;
static int lastMeasurement = 0;

static float dimStep = 0.01;
static float dimValue = 0.0;
static int lastDim = 0;

void loop() {
  // if (WiFi.status() != WL_CONNECTED && !wifiConnecting) {
  //   Serial.println("Connecting to WiFi");
  //   WiFi.begin(WIFI_SSID, WIFI_PASS);
  //   wifiConnecting = true;
  // }

  // if (WiFi.status() == WL_CONNECTED && wifiConnecting) {
  //   Serial.println("WiFi connection successful");
  //   Serial.print("The IP Address of ESP8266 Module is: ");
  //   Serial.print(WiFi.localIP());
  //   wifiConnecting = false;
  // }

  // httpServer.handleClient();

  unsigned int now = millis();
  // if (now - lastMeasurement > config.pollingIntervalMs) {
  //   float temperature = readTemperature();
  //   float heaterValue = pid.compute(temperature);
  //   setHeaterPower(heaterValue);
  //   temperatureHistory.push(temperature);
  //   lastMeasurement = now;
  // }

  if (now - lastDim > 20) {
    setHeaterPower(dimValue);
    dimValue += dimStep;
    if (dimValue >= 1.0 || dimValue <= 0) {
      dimStep *= -1;
    }
    lastDim = now;
  }
}
