#include <Arduino.h>
#include "lib.h"
#include "logic.h"
#include "io.h"
#include "data.h"
#include "time.h"
#include "ethernet.h"
#include "mqtt.h"

namespace LOGIC {
  bool insValues[24];
  
  void In::setValue(const uint8_t id, const bool val) {
    insValues[id] = val;
  }
  
  bool In::getValue(const uint8_t id) {
    return insValues[id];
  }
  
  void Out::setValue(const uint8_t id, const bool val) {
    value = val;
    IO::setOutVal(id, value);
    Serial.print(F("Out ")); Serial.print(name); Serial.print(F(" change val to "));Serial.println(val);
  }
  
  void inChange(const uint64_t currTime, const uint8_t pinId, const bool pinVal) {
    auto in = DATA::getIn(pinId);
    in.setValue(pinId, pinVal);
    MQTT::sendInState(pinId);
  }

  void setup() {
    for (uint8_t i = 0; i < 24; i++) {
      auto out = DATA::getOut(i);
      if (out.enabled) {
        out.setValue(i, out.value);
      }
    }
  }
  
  void loop(const uint32_t currMillis) {
    static uint32_t lastCheckTime;
    if (currMillis > lastCheckTime + 300 || currMillis < lastCheckTime) {
      lastCheckTime = currMillis;
      // логика светодиода готовности
      if (!ETH::connected) {
        IO::setBlueLedState(IO::blinking);
      } else {
        if (DATA::getMqttServer().length() == 0) {
          IO::setBlueLedState(IO::on);
        } else {
          if (MQTT::subscribed) {
            IO::setBlueLedState(IO::on);
          } else {
            IO::setBlueLedState(IO::blinking);
          }
        }
      }
    }
  }
  
  void resetSettingsBtnPressed(const uint64_t currTime) {
    Serial.println(F("Reset settings and restart"));
    DATA::initEmptyData();
    delay(100);
    restart();
  }
}
