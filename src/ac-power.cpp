#include <Arduino.h>
#include <ESP8266TimerInterrupt.h>
#include <ESP8266_ISR_Timer.h>

#include "ac-power.h"
#include "pins.h"
#include "triac-output.h"

ESP8266Timer timer;

static unsigned int timerCounter = 0;
static unsigned int timerCounterPerPhase = 0;
static bool firstRisingEdge = false;

static TriacOutput heaterTriacOutput(PIN_HEATER_TRIAC);
static TriacOutput pumpTriacOutput(PIN_PUMP_TRIAC);

void ICACHE_RAM_ATTR timerHandler(void) {
  timerCounter++;

  if (timerCounterPerPhase < 256)
    return;

  if (timerCounter % (timerCounterPerPhase / 256) == 0) {
    heaterTriacOutput.timerHandler();
    pumpTriacOutput.timerHandler();
  }
}

void ICACHE_RAM_ATTR zeroCrossHandler(void) {
  bool falling = digitalRead(PIN_ZC_DETECT);

  heaterTriacOutput.zeroCross(falling);
  pumpTriacOutput.zeroCross(falling);

  if (falling) {
    if (firstRisingEdge) {
      timerCounterPerPhase = timerCounter;
    }
  } else {
    firstRisingEdge = true;
    timerCounter = 0;
  }
}

void initACPower() {
    // Run timer as fast as possible
  timer.attachInterruptInterval(1, timerHandler);
  attachInterrupt(digitalPinToInterrupt(PIN_ZC_DETECT), zeroCrossHandler, CHANGE);
  pinMode(PIN_VALVE_TRIAC, OUTPUT);
  digitalWrite(PIN_VALVE_TRIAC, LOW);
}

void setHeaterPower(float value) {
  heaterTriacOutput.setPower(value);
}

float getHeaterPower() {
  return heaterTriacOutput.getPower();
}

void setPumpPower(float value) {
  pumpTriacOutput.setPower(value);
}

float getPumpPower() {
  return pumpTriacOutput.getPower();
}

void setValve(bool on) {
  digitalWrite(PIN_VALVE_TRIAC, on);
}
