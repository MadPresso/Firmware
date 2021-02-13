#include <Arduino.h>
#include <ArduinoJson.h>
#include <ESP8266WebServer.h>
#include <LittleFS.h>
#include <Wire.h>

#define ESP_DRD_USE_LITTLEFS true
#include <ESP_DoubleResetDetector.h>

#include "ac-power.h"
#include "config.h"
#include "digital-switch.h"
#include "network-manager.h"
#include "pid-controller.h"
#include "pins.h"
#include "shot-timer.h"
#include "temperature.h"
#include "time-series.h"

static Config config;
static ESP8266WebServer httpServer(80);
static DoubleResetDetector drd(8, 0);
static PIDController pidController(0, AC_OUTPUT_MAX);
static TemperatureReader temperatureReader;
//static TimeSeries temperatureHistory;
static NetworkManager networkManager;
static ShotTimer shotTimer(&config, &pidController);
static Ticker measurementTicker;

static DigitalSwitch shotSwitch(PIN_SHOT_SENSOR);
static DigitalSwitch steamSwitch(PIN_STEAM_SENSOR);

void ICACHE_RAM_ATTR shotSwitchHandler(void) {
  shotSwitch.interrupt();
}

void ICACHE_RAM_ATTR steamSwitchHandler(void) {
  steamSwitch.interrupt();
}

void setHeaterTargetTemperature(void) {
  bool steam = !digitalRead(PIN_STEAM_SENSOR);
  pidController.setTarget(steam ?
                            config.steamTemperature :
                            config.brewTemperature);
}

// HTTP callbacks

void httpGetStatus(void) {
  const size_t CAPACITY = JSON_OBJECT_SIZE(3);
  StaticJsonDocument<CAPACITY> doc;

  JsonObject object = doc.to<JsonObject>();
  object["temperature"] = int(temperatureReader.current() * 10) / 10.0;
  object["targetTemperature"] = pidController.currentTarget();
  object["heaterPowerPercentage"] = int(float(getHeaterPower()) / (AC_OUTPUT_MAX/100.0));

  String output;
  serializeJsonPretty(doc, output);
  httpServer.send(200, "application/json", output);
}

// void httpGetTemperatures(void) {
//   String *output = new String;
//   temperatureHistory.toJson(*output);
//   httpServer.send(200, "application/json", *output);
//   delete output;
// }

void httpGetMachineConfig(void) {
  String output;
  config.toJson(output);
  httpServer.send(200, "application/json", output);
}

void httpPostMachineConfig(void) {
  const String &body = httpServer.arg("plain");
  bool result = config.fromJson(body);
  
  if (result) {
    shotTimer.stop();
    setHeaterPower(0);
    result = config.write(LittleFS);
    pidController.setParams(config.pidP, config.pidI, config.pidD, config.pidIntegralWindupLimit);
    setHeaterTargetTemperature();
  }

  httpServer.send(result ? 200 : 500);
}

void httpGetNetworkConfig(void) {
  String output;
  networkManager.configAsJSON(output);
  httpServer.send(200, "application/json", output);
}

void httpPostNetworkConfig(void) {
  const String &body = httpServer.arg("plain");
  shotTimer.stop();
  setHeaterPower(0);
  bool result = networkManager.applyFromJSON(body);

  httpServer.send(result ? 200 : 500);
}

void httpGetWifis(void) {
  String output;
  networkManager.scanWifis(output);
  httpServer.send(200, "application/json", output);
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

  LittleFS.begin();

  if (drd.detectDoubleReset()) {
    Serial.println("Double-reset detected. Resetting config.");
    String apName = "MadPresso " + String(ESP.getChipId(), HEX);
    networkManager.forceAPMode(apName);
    config.write(LittleFS);
  } else {
    config.read(LittleFS);
  }

  // networkManager.setHostname(config.deviceName.c_str());
  networkManager.autoConnect();

  pidController.setParams(config.pidP, config.pidI, config.pidD, config.pidIntegralWindupLimit);
  initACPower();

  httpServer.on("/api/v1/status", HTTP_GET, httpGetStatus);
  httpServer.on("/api/v1/config/machine", HTTP_GET, httpGetMachineConfig);
  httpServer.on("/api/v1/config/machine", HTTP_POST, httpPostMachineConfig);
  httpServer.on("/api/v1/config/network", HTTP_GET, httpGetNetworkConfig);
  httpServer.on("/api/v1/config/network", HTTP_POST, httpPostNetworkConfig);
  httpServer.on("/api/v1/wifis", HTTP_GET, httpGetWifis);
  // httpServer.on("/api/v1/temperatures", HTTP_GET, httpGetTemperatures);
  httpServer.serveStatic("/", LittleFS, "/webroot/", "");
  httpServer.begin();

  attachInterrupt(digitalPinToInterrupt(PIN_SHOT_SENSOR), shotSwitchHandler, CHANGE);
  attachInterrupt(digitalPinToInterrupt(PIN_STEAM_SENSOR), steamSwitchHandler, CHANGE);
  setHeaterTargetTemperature();

  Serial.println("Initialization completed");
}

void loop() {
  drd.loop();
  shotTimer.tick();
  networkManager.tick();
  temperatureReader.tick();
  measurementTicker.tick();
  httpServer.handleClient();

  bool state;

  if (shotSwitch.changed(&state)) {
    Serial.printf("Shot switch: %d\n", !state);

    if (!state)
      shotTimer.start();
    else
      shotTimer.stop();
  }

  if (steamSwitch.changed(&state)) {
      setHeaterTargetTemperature();
  }

  if (measurementTicker.elapsed() > 1000) {
    float temperature = temperatureReader.current();
    // temperatureHistory.push(temperature);

    int heaterValue = shotTimer.isActive() ?
      config.heaterPercentageDuringShot * (AC_OUTPUT_MAX/100.0) :
      pidController.compute(temperature);

    setHeaterPower(heaterValue);

    Serial.printf("%.2f %.2f %d\n", temperature, pidController.currentTarget(), heaterValue);
    measurementTicker.reset();
  }
}
