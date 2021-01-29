#pragma once

void initACPower();

void setHeaterPower(uint8_t value);
uint8_t getHeaterPower();

void setPumpPower(uint8_t value);
uint8_t getPumpPower();

void setValve(bool on);
