#include <Arduino.h>

class DigitalSwitch {
public:
  DigitalSwitch(int gpio, int onState) :
    gpio(gpio), onState(onState), first(true), irqPending(false) {}

  void ICACHE_RAM_ATTR interrupt() {
    irqPending = true;
  }

  bool on() {
    bool b;
    changed(&b);
    return b;
  }

  bool changed(bool *state) {
    noInterrupts();
    bool _irqPending = irqPending;
    irqPending = false;
    interrupts();

    *state = digitalRead(gpio) == onState;

    if (!first && (!_irqPending || (*state == lastReportedState)))
        return false;

    lastReportedState = *state;
    first = false;

    return true;
  }

private:
  int gpio;
  int onState;
  bool first;
  bool irqPending;
  bool lastReportedState;
};
