#include <stdio.h>

#pragma once

#define ICACHE_RAM_ATTR

extern unsigned long _millis;

static inline unsigned long millis(void) {
  return _millis;
}

static inline void millis_tick(unsigned int delta = 1) {
  _millis += delta;
}
