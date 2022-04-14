#include <Ethernet.h>

#ifndef TIME_H
#define TIME_H

namespace TIME {
  void loop(const uint32_t currMillis);
  extern uint32_t uptime;
}

#endif
