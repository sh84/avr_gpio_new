#include <Arduino.h>
#include "ethernet.h"

#ifndef MILLIS64_H
#define MILLIS64_H

const int TOO_LOW_FREE_MEMORY = 256;

uint64_t millis64();
char* intToStr(uint64_t value);
String ipToString(const IPAddress &ip);
void memTest();
void restart();
int getFreeMemory();
void timerOneInit(uint32_t microseconds);

#define bitToggle(value, bit) ((value) ^= (1UL << (bit)))

#endif
