#include <EEPROM.h>
#include <Ethernet.h>
#include "logic.h"

#ifndef DATA_H
#define DATA_H

namespace DATA {
  void setup();
  void loop(const uint32_t currMillis);
  void initEmptyData();
  bool isUseDHCP();
  void saveUseDHCP(const bool val);
  const IPAddress getStaticIP();
  void setStaticIP(const IPAddress ip);
  const IPAddress getStaticSubnetMask();
  void setStaticSubnetMask(const IPAddress ip);
  const IPAddress getStaticGatewayIP();
  void setStaticGatewayIP(const IPAddress ip);
  const String getMqttServer();
  void setMqttServer(const String &str);
  const String getMqttTopic();
  void setMqttTopic(const String &str);

  const LOGIC::In getIn(const uint8_t id);
  void setIn(const uint8_t id, const LOGIC::In val);
  
  LOGIC::Out getOut(const uint8_t id);
  const int8_t getOutIdByName(const char *name);
  void setOut(const uint8_t id, const LOGIC::Out val);
  void setOutValue(const uint8_t id, const uint32_t val);
}

#endif
