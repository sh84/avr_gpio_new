#include <stdint.h>

#ifndef IO_H
#define IO_H

namespace IO {
  enum LedState: uint8_t {off, on, blinking};

  void setup();
  void tick(const uint64_t currTime);
  void setBlueLedState(const LedState state);
  void setOutVal(const uint8_t outId, const bool val);
}

#endif
