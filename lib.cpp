#include "lib.h"
#include <Arduino.h>
#include "ethernet.h"
#include <avr/wdt.h>

uint64_t millis64() {
  static uint32_t low32, high32;
  uint8_t oldSREG = SREG;
  cli();
  uint32_t newLow32 = millis();
  if (newLow32 < low32) high32++;
  low32 = newLow32;
  uint64_t result = (uint64_t)high32 << 32 | low32;
  SREG = oldSREG;
  return result;
}

char* intToStr(uint64_t value) {
  const uint8_t numDigts = value > 0 ? log10(value) + 1 : 1;
  static char result[21];
  result[numDigts] =  0;
  for (uint8_t i = numDigts; i--; value /= 10) {
    result[i] = '0' + (value % 10);
  }
  return result;
}

String ipToString(const IPAddress &ip) {
  return String(ip[0]) + '.' + String(ip[1]) + '.' + String(ip[2]) + '.' + String(ip[3]);
}

void restart() {
  wdt_disable();
  wdt_enable(WDTO_15MS);
  while (true) {}
}

void memTest() {
  Serial.print(F("Mem test... "));
  Serial.flush();
  uint16_t memErrors PROGMEM;
  volatile char c;
  const char p1 = 0x55;
  const char p2 = 0xAA;
  for (uintptr_t i PROGMEM = RAMSTART; i<=RAMEND; i++) {
    c = *reinterpret_cast<char *>(i);
    *reinterpret_cast<char *>(i) = p1;
    if (*reinterpret_cast<char *>(i) != p1) memErrors++;
    *reinterpret_cast<char *>(i) = p2;
    if (*reinterpret_cast<char *>(i) != p2) memErrors++;
    *reinterpret_cast<char *>(i) = c;
  }
  Serial.print(memErrors);
  Serial.println(F(" errors"));
  Serial.flush();
}

extern unsigned int __bss_end;
extern unsigned int __heap_start;
extern void *__brkval;
int getFreeMemory() {
  int freeMemory;
  if((int)__brkval == 0) {
    freeMemory = ((int)&freeMemory) - ((int)&__bss_end);
  } else {
    freeMemory = ((int)&freeMemory) - ((int)__brkval);
  }
  return freeMemory;
}

void timerOneInit(const uint32_t microseconds) {
  TCCR1B = _BV(WGM13);        // set mode as phase and frequency correct pwm, stop the timer
  TCCR1A = 0;                 // clear control register A 
  const uint32_t cycles = (F_CPU/100000 * microseconds) / 20;
  uint16_t pwmPeriod;
  uint8_t clockSelectBits;
  if (cycles < 65536UL) {
    clockSelectBits = _BV(CS10);
    pwmPeriod = cycles;
  } else if (cycles < 65536UL * 8) {
    clockSelectBits = _BV(CS11);
    pwmPeriod = cycles / 8;
  } else if (cycles < 65536UL * 64) {
    clockSelectBits = _BV(CS11) | _BV(CS10);
    pwmPeriod = cycles / 64;
  } else if (cycles < 65536UL * 256) {
    clockSelectBits = _BV(CS12);
    pwmPeriod = cycles / 256;
  } else if (cycles < 65536UL * 1024) {
    clockSelectBits = _BV(CS12) | _BV(CS10);
    pwmPeriod = cycles / 1024;
  } else {
    clockSelectBits = _BV(CS12) | _BV(CS10);
    pwmPeriod = 65536UL - 1;
  }
  ICR1 = pwmPeriod;
  TCCR1B = _BV(WGM13) | clockSelectBits;
  TIMSK |= _BV(TOIE1);  // enable  interrupts
}
