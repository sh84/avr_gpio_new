#include <avr/wdt.h>
#include "lib.h"
#include "data.h"
#include "io.h"
#include "ethernet.h"
#include "time.h"
#include "http_server.h"
#include "logic.h"
#include "mqtt.h"

volatile bool timerTickRuning = false;
uint64_t lastTickTime = 0;
uint64_t lastLoopTime = 0;

void setup() {
  IO::setup();
  Serial.begin(115200);
  memTest();
  delay(10);
  Serial.print(F("MCUCSR: ")); Serial.println(MCUCSR, HEX);
  MCUCSR = 0;
  DATA::setup();
  LOGIC::setup();
  timerOneInit(10000); // период 10мс
  ETH::setup();
  HTTP_SERVER::setup();
  MQTT::setup();

  Serial.print(F("Free memory: ")); Serial.println(getFreeMemory());
  Serial.flush();
  wdt_disable();
  wdt_enable(WDTO_2S);
}

ISR(TIMER1_OVF_vect) {
  cli();
  if (timerTickRuning) {
    sei();
    return;
  }
  timerTickRuning = true;
  lastTickTime = millis64();
  sei();
  wdt_reset();
  IO::tick(lastTickTime);
  static uint64_t printTime;
  if (lastLoopTime != 0 && lastTickTime > lastLoopTime + 2000 && lastTickTime > printTime + 100) {
    printTime = lastTickTime;
    Serial.print(F("Loop too long! Last loop ago:")); Serial.println((uint32_t)(lastTickTime - lastLoopTime));
    if (lastTickTime > lastLoopTime + 10000) {
      Serial.println(F("10s loop timeout, restart"));
      Serial.flush();
      delay(1);
      restart();
    }
  }  
  timerTickRuning = false;
}

void loop() {
  if (millis64() > lastTickTime + 1000) {
    Serial.print(F("Timer loop too long! Last tick ago:")); Serial.println((uint32_t)(millis64() - lastTickTime));
  }
  lastLoopTime = lastTickTime;
  uint32_t currMillis = millis();
  //LOGIC::loop(currMillis);
  DATA::loop(currMillis);
  ETH::loop(currMillis);
  TIME::loop(currMillis);
  MQTT::loop(currMillis);
  HTTP_SERVER::loop(currMillis);
  int freeMemory = getFreeMemory();
  if (freeMemory < TOO_LOW_FREE_MEMORY) {
    Serial.print(F("Free memory: ")); Serial.print(freeMemory); Serial.println(F(" - too low!!!"));
  }
  delay(1);
}
