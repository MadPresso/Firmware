#pragma once

#include "triac-output.h"

#define AC_OUTPUT_MAX (TRIAC_TICKS-1)

void initACPower();

void setHeaterPower(uint8_t value);
uint8_t getHeaterPower();

void setPumpPower(uint8_t value);
uint8_t getPumpPower();

void setValve(bool on);
