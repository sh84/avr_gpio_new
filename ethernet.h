#include <Ethernet.h>

#ifndef ETH_H
#define ETH_H

namespace ETH {
  void setup();
  bool checkAndPrintErrors(bool checkDHCP = true);
  extern bool connected;
  void setStaticIp();
  void setDHCP();
  IPAddress getIP();
  void loop(const uint32_t currMillis);
}

#endif
