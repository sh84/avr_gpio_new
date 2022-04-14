#include <Ethernet.h>
#include <PubSubClient.h>

#ifndef MQTT_H
#define MQTT_H

namespace MQTT {
  void connect(const uint32_t currMillis);
  void subscribe(const uint32_t currMillis);
  extern bool subscribed;
  void setup();
  void loop(const uint32_t currMillis);
  void reconnect();
  void sendInfo();
  void sendInfoOnNextLoop();
  void sendOutState(const uint8_t OutId);
  void sendInState(const uint8_t InId);
}

#endif
