#include <Arduino.h>
#include <ESP8266TimerInterrupt.h>
#include <ESP8266_ISR_Timer.h>

#include "ac-power.h"
#include "pins.h"

static ESP8266Timer timer;
static int timerWaitZC = 5;
static unsigned long lastRisingZC = 0;

static TriacOutput heaterTriacOutput(PIN_HEATER_TRIAC);
static TriacOutput pumpTriacOutput(PIN_PUMP_TRIAC);

void ICACHE_RAM_ATTR timerHandler(void) {
  heaterTriacOutput.timerHandler();
  pumpTriacOutput.timerHandler();
}

void ICACHE_RAM_ATTR zeroCrossHandler(void) {
  bool rising = digitalRead(PIN_ZC_DETECT) == HIGH;

  heaterTriacOutput.zeroCross(rising);
  pumpTriacOutput.zeroCross(rising);

  if (timerWaitZC > 0) {
    unsigned long now = micros();

    if (rising) {
      lastRisingZC = now;
    } else if (--timerWaitZC == 0) {
      unsigned long phaseLength = now - lastRisingZC;
      unsigned int frequency = (TRIAC_TICKS*1000000) / phaseLength;

      timer.setFrequency(frequency, timerHandler);
      Serial.printf("Set timer frequency to %d Hz\n", frequency);
    }
  }
}

void initACPower() {
  pinMode(PIN_ZC_DETECT, INPUT);
  attachInterrupt(digitalPinToInterrupt(PIN_ZC_DETECT), zeroCrossHandler, CHANGE);

  pinMode(PIN_VALVE_TRIAC, OUTPUT);

  setValve(0);
  setHeaterPower(0);
}

void setHeaterPower(uint8_t value) {
  heaterTriacOutput.setPower(value);
}

uint8_t getHeaterPower() {
  return heaterTriacOutput.getPower();
}

void setPumpPower(uint8_t value) {
  pumpTriacOutput.setPower(value);
}

uint8_t getPumpPower() {
  return pumpTriacOutput.getPower();
}

void setValve(bool on) {
  digitalWrite(PIN_VALVE_TRIAC, on ? HIGH : LOW);
}
