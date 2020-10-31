#include "dimmer.h"

#define ARRAY_SIZE(a) sizeof(a) / sizeof((a)[0])

// Generated with acos.rb

static const uint8_t acosLUT[256] = {
  0xff, 0xfe, 0xfd, 0xfd, 0xfc, 0xfb, 0xfb, 0xfa,
  0xf9, 0xf9, 0xf8, 0xf7, 0xf7, 0xf6, 0xf6, 0xf5,
  0xf4, 0xf4, 0xf3, 0xf2, 0xf2, 0xf1, 0xf0, 0xf0,
  0xef, 0xef, 0xee, 0xed, 0xed, 0xec, 0xeb, 0xeb,
  0xea, 0xe9, 0xe9, 0xe8, 0xe8, 0xe7, 0xe6, 0xe6,
  0xe5, 0xe4, 0xe4, 0xe3, 0xe2, 0xe2, 0xe1, 0xe0,
  0xe0, 0xdf, 0xde, 0xde, 0xdd, 0xdd, 0xdc, 0xdb,
  0xdb, 0xda, 0xd9, 0xd9, 0xd8, 0xd7, 0xd7, 0xd6,
  0xd5, 0xd5, 0xd4, 0xd3, 0xd3, 0xd2, 0xd1, 0xd1,
  0xd0, 0xcf, 0xcf, 0xce, 0xcd, 0xcd, 0xcc, 0xcb,
  0xcb, 0xca, 0xc9, 0xc9, 0xc8, 0xc7, 0xc7, 0xc6,
  0xc5, 0xc5, 0xc4, 0xc3, 0xc3, 0xc2, 0xc1, 0xc1,
  0xc0, 0xbf, 0xbe, 0xbe, 0xbd, 0xbc, 0xbc, 0xbb,
  0xba, 0xba, 0xb9, 0xb8, 0xb8, 0xb7, 0xb6, 0xb5,
  0xb5, 0xb4, 0xb3, 0xb3, 0xb2, 0xb1, 0xb0, 0xb0,
  0xaf, 0xae, 0xae, 0xad, 0xac, 0xab, 0xab, 0xaa,
  0xa9, 0xa8, 0xa8, 0xa7, 0xa6, 0xa5, 0xa5, 0xa4,
  0xa3, 0xa2, 0xa2, 0xa1, 0xa0, 0x9f, 0x9f, 0x9e,
  0x9d, 0x9c, 0x9c, 0x9b, 0x9a, 0x99, 0x98, 0x98,
  0x97, 0x96, 0x95, 0x95, 0x94, 0x93, 0x92, 0x91,
  0x90, 0x90, 0x8f, 0x8e, 0x8d, 0x8c, 0x8c, 0x8b,
  0x8a, 0x89, 0x88, 0x87, 0x86, 0x86, 0x85, 0x84,
  0x83, 0x82, 0x81, 0x80, 0x7f, 0x7f, 0x7e, 0x7d,
  0x7c, 0x7b, 0x7a, 0x79, 0x78, 0x77, 0x76, 0x75,
  0x74, 0x73, 0x72, 0x71, 0x70, 0x6f, 0x6e, 0x6d,
  0x6c, 0x6b, 0x6a, 0x69, 0x68, 0x67, 0x66, 0x65,
  0x64, 0x63, 0x62, 0x61, 0x5f, 0x5e, 0x5d, 0x5c,
  0x5b, 0x5a, 0x58, 0x57, 0x56, 0x55, 0x53, 0x52,
  0x51, 0x50, 0x4e, 0x4d, 0x4b, 0x4a, 0x49, 0x47,
  0x46, 0x44, 0x43, 0x41, 0x3f, 0x3e, 0x3c, 0x3a,
  0x38, 0x37, 0x35, 0x33, 0x31, 0x2e, 0x2c, 0x2a,
  0x27, 0x25, 0x22, 0x1f, 0x1b, 0x17, 0x13, 0x00,
};

Dimmer::Dimmer(int _gpio) {
  gpio = _gpio;
  avgPeriod = 0;
  timingsTaken = 0;
  prevMicros = 0;

  pinMode(gpio, OUTPUT);
  digitalWrite(gpio, LOW);

  value = 0.f;
  counter = 0;
}

void Dimmer::zeroCross(bool falling) {
  digitalWrite(gpio, LOW);

  //
  // Collect a number of measurements for the period from the rising to the falling
  // slope of the zero detect signal. Once we have enough information, calculate the
  // average mean value to see what the window is in which we may schedule the Triac
  // ignition event.
  //

  if (timingsTaken < ARRAY_SIZE(timings)) {
    if (falling) {
      if (prevMicros)
        timings[timingsTaken++] = micros() - prevMicros;
    } else {
      prevMicros = micros();
    }

    return;
  }

  if (avgPeriod == 0) {
    for (unsigned i = 0; i < ARRAY_SIZE(timings); i++)
      avgPeriod += timings[i];

    avgPeriod /= ARRAY_SIZE(timings);
  }

  if (falling)
    counter = 0;
  else
    counter = acosLUT[uint8_t(value * 256.0)];
}

void Dimmer::setValue(float _value)
{
  if (_value < 0.f)
    _value = 0.f;

  if (_value > 0.f)
    _value = 1.f;

  noInterrupts();
  value = _value;
  interrupts();
}

void ICACHE_RAM_ATTR Dimmer::timerHandler()
{
  if (counter > 0 && counter-- == 0)
    digitalWrite(gpio, HIGH);
}