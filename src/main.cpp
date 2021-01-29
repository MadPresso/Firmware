#include <Arduino.h>
#include <ArduinoJson.h>
#include <ESP8266WebServer.h>
#include <LittleFS.h>
#include <ADC101C.h>

#include "ac-power.h"
#include "config.h"
#include "network-manager.h"
#include "pid-controller.h"
#include "pins.h"
#include "shot-timer.h"
#include "time-series.h"

static ESP8266WebServer httpServer(80);

static NetworkManager networkManager;
static PIDController pid;
static TimeSeries temperatureHistory;
static Config config;
static ADC101C adc;
static bool adcInitialized;
static ShotTimer shotTimer(&config);

float readTemperature() {
  if (!adcInitialized)
    return 0;

  uint16_t highest = adc.highest();
  uint16_t lowest = adc.lowest();
  uint16_t value = (highest + lowest) / 2;
  int mV = map(value, 0, 1023, 0, 3300);

  // LM35DT has 10mV/°C
  float f = float(mV) / 10.0;

  return f;
}

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
  object["temperature"] = readTemperature();
  object["targetTemperature"] = pid.currentTarget();
  object["heaterPowerPercentage"] = int(float(getHeaterPower()) / 2.55);

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

void setLED(bool on) {
  digitalWrite(PIN_LED, on ? LOW : HIGH);
}

void setup() {
  Serial.begin(115200);
  
  delay(2000);

  Serial.println("CPU Frequency = " + String(F_CPU / 1000000) + " MHz");

  Wire.begin(D2, D1);
  adcInitialized = adc.begin(&Wire, 0x50);
  if (adcInitialized)
    adc.setMode(ADC101C_MODE_AUTO_512);

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

static int lastMeasurement = 0;

#define TEMP_MEASUREMENTS 20
static float temperatures[TEMP_MEASUREMENTS];
unsigned int temperatureIndex = 0;

void loop() {
  networkManager.tick();
  httpServer.handleClient();
  shotTimer.tick();

  unsigned int now = millis();

  if (now - lastMeasurement > 50) {
    temperatures[temperatureIndex] = readTemperature();
    temperatureIndex++;
    temperatureIndex %= TEMP_MEASUREMENTS;
    lastMeasurement = now;

    if (temperatureIndex == 0) {
      float temperature = 0;

      for (int n = 0; n < TEMP_MEASUREMENTS; n++)
        temperature += temperatures[temperatureIndex];

      temperature /= TEMP_MEASUREMENTS;

      float heaterValue = pid.compute(temperature);
      setHeaterPower(heaterValue);
      Serial.printf("Temperature %.2f  heaterValue %.2f\n", temperature, heaterValue);
      // temperatureHistory.push(temperature);
    }
  }
}
