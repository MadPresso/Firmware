#include <Arduino.h>

class DigitalSwitch {
public:
  DigitalSwitch(int gpio) :
    gpio(gpio), first(true), irqPending(false) {}

  void ICACHE_RAM_ATTR interrupt() {
    irqPending = true;
  }

  bool state() {
    return digitalRead(gpio) == HIGH;
  }

  bool changed(bool *state) {
    noInterrupts();
    bool _irqPending = irqPending;
    irqPending = false;
    interrupts();

    if (!_irqPending && !first)
      return false;

    *state = digitalRead(gpio) == HIGH;

    if (*state == lastReportedState && !first)
      return false;

    lastReportedState = *state;
    first = false;

    return true;
  }

private:
  int gpio;
  bool first;
  bool irqPending;
  bool lastReportedState;
};
