#include <Arduino.h>
#include "io.h"
#include "lib.h"
#include "logic.h"
#include "data.h"

/*
 * PB4-PB7, PD0-PD7, PC0-PC7 - in1-in20 соотвественно
 * PA0-PA7, PF0-PF7, PE2-PE6 - out0-out21 соотвественно
 * PE7 - led ready
 * PG4 - сброс настроек
 */

namespace IO {
  const int PIN_JITTER_TIME = 50;
  const int LED_BLINK_TIME = 1000;
  
  LedState ledBlueState = off;
 
  void setup() {
    // порт A - выход
    DDRA = 0xff;
    PORTA = 0;
    // порт PB4-PB7 - вход
    DDRB =  0b00001111;
    PORTB = 0b11110000;
    // порт C - вход
    DDRC = 0;
    PORTC = 0xff;
    // порт D - вход
    DDRD = 0;
    PORTD = 0xff;
    // порт E - выход
    DDRE = 0xff;
    PORTE = 0;
    // порт F - выход
    DDRF = 0xff;
    PORTF = 0;
    // порт G - вход
    DDRG =  0;
    PORTG = 0xff;
  }

  void tickLedBlue(const uint64_t currTime) {
    static uint64_t ledBlueChangeTime;
    if (ledBlueState == off) {
      bitClear(PORTE, 7);
    } else if (ledBlueState == on) {
      bitSet(PORTE, 7);
    } else if (currTime > ledBlueChangeTime + LED_BLINK_TIME) {
      bitToggle(PORTE, 7);
      ledBlueChangeTime = currTime;
    }
  }

  void tickInputs(const uint64_t currTime) {
    static uint64_t pinsChangeTime[24];
    static uint32_t lastPins = 0x00;
    // PE2-PE7, PB6, PB7, PD0-PD7, PG0, PG1, PG3, PG4 - in1-in20 соотвественно
    //uint32_t pins = uint32_t(PINE) >> 2 | uint32_t(PINB) & 0b11000000 | uint32_t(PIND) << 8 | uint32_t(PING & 0b00000011) << 16 | uint32_t(PING & 0b00011000) << 15;
    // PB4-PB7, PD0-PD7, PC0-PC7 - in1-in20 соотвественно
    uint32_t pins = uint32_t(PINB) >> 4 | uint32_t(PIND) << 4 | uint32_t(PINC) << 12;
    for (uint8_t pinId = 0; pinId < 20; pinId++) {
      LOGIC::In in = DATA::getIn(pinId);
      if (!in.enabled) continue;
      bool pinVal = bitRead(pins, pinId);
      auto changedAgo = pinsChangeTime[pinId] > 0 ? currTime - pinsChangeTime[pinId] : 0;
      // в течении in.throttleTime после изменения состояния кнопки ничего не делаем
      if (changedAgo > 0 && changedAgo < in.throttleTime) continue;
      pinsChangeTime[pinId] = currTime;
      // проверем что бит изменился - кнопка нажата/отжата
      bool lastVal = bitRead(lastPins, pinId);
      if (pinVal != lastVal) {
        Serial.print(F("pin "));Serial.print(pinId);Serial.print(F(" changed "));Serial.print(lastVal);Serial.print(F(" -> "));Serial.println(pinVal);
        LOGIC::inChange(currTime, pinId, pinVal);
        pinVal ? bitSet(lastPins, pinId) : bitClear(lastPins, pinId);
      }
    }
  }

  void tickResetSettingsBtn(const uint64_t currTime) {
    uint64_t btnChangeTime; // время последнего изменения состояния кнопки
    static bool btnState;   // текущее состояние кнопки (true - нажата)
    auto changeTimeAgo = btnChangeTime > 0 ? currTime - btnChangeTime : 0;
    // в течении PIN_JITTER_TIME после изменения состояния ничего не делаем
    if (changeTimeAgo > 0 && changeTimeAgo < PIN_JITTER_TIME) return;
    bool currPinVal = !bitRead(PING, 4); // активный сигнал - false - инвертируем
    if (currPinVal != btnState) {
      btnState = currPinVal;
      btnChangeTime = currTime;
      if (btnState ) {
        LOGIC::resetSettingsBtnPressed(currTime);
      }
    }
  }

  void tick(const uint64_t currTime) {
    tickInputs(currTime);
    tickResetSettingsBtn(currTime);
    tickLedBlue(currTime);
  }

  void setBlueLedState(const LedState state) {
    ledBlueState = state;
  }

  void setOutVal(const uint8_t outId, const bool val) {
    // PA0-PA7, PF0-PF7, PE2-PE6 - out0-out21 соотвественно
    if (outId < 8) {
      bitWrite(PORTA, outId, val);
    } else if (outId < 16) {
      bitWrite(PORTF, outId-8, val);
    } else if (outId < 24) {
      bitWrite(PORTE, outId-16+2, val);
    } else {
      Serial.print(F("IO::setOutVal range error, id= ")); Serial.println(outId);
    }
  }
}
