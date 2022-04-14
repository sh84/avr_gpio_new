#include <stdint.h>

#ifndef LOGIC_H
#define LOGIC_H

namespace LOGIC {
  struct In {
    uint16_t throttleTime;
    bool enabled;
    char name[64];

    void setValue(const uint8_t id, const bool val);
    bool getValue(const uint8_t id);
  };
  struct Out {
    bool enabled;
    char name[64];
    bool value;

    void setValue(const uint8_t id, const bool val);
  };

  void setup();
  void loop();
  void inChange(const uint64_t currTime, const uint8_t pinId, const bool pinVal);
  void resetSettingsBtnPressed(const uint64_t currTime);
}

#endif
