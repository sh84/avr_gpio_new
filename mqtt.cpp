#include "mqtt.h"
#include "lib.h"
#include "ethernet.h"
#include "data.h"
#include "time.h"

/*
 * Топики
 * -> <topic>/info                 - информация по состоянию, json: {uptime: <uptime>, ip: "<ip>", ins: [<in>,<in>...], outs: [<out>,<out>...]}
 *                                    in: {name: "имя входа", enabled: 0|1, value: 0|1, throttle_time: <0-65535>}
 *                                    out: {name: "имя выхода", enabled: 0|1, value: 0|1}
 * -> <topic>/in/<имя входа> [R]   - состояние входа, 0 или 1
 * -> <topic>/out/<имя выхода> [R] - состояние выхода, 0 или 1
 * <- <topic>/out/<имя выхода>/set - изменить состояние выхода, 0 или 1
 */

namespace MQTT {
  const uint16_t SEND_INFO_PERIOD = 30000;           // интервал отправки информации по настройкам и состоянию, мс

  EthernetClient net;
  PubSubClient client(net);
  bool subscribed;
  uint32_t lastConnectTime;
  uint32_t lastSubscribeTime;
  uint32_t lastSendInfoTime;

  void callback(char* inTopic, uint8_t* payloadRaw, unsigned int payloadLength) {
    if (payloadLength > 64) payloadLength = 64; // больше нам не потребуется, предотвращаем переполнение памяти для String((char*)payloadRaw)
    payloadRaw[payloadLength] = 0; // гарантия корректности строки в payload
    Serial.print(F("MQTT incoming: "));
    Serial.print(inTopic);
    Serial.print(F(" >> "));
    Serial.println((char*)payloadRaw);
    const String topic = DATA::getMqttTopic();
    uint8_t topicLen = topic.length();
    if (topicLen >= strlen(inTopic)) {
      return;
    }
    const String payload = String((char*)payloadRaw);
    char *significantTopic = inTopic + topicLen;
    // /out/<имя выхода>/set
    if (strstr(significantTopic, "/out/") != NULL) {
      char *setSubstr = strstr(significantTopic, "/set");
      if (setSubstr != NULL) {
        char *outName = significantTopic + 5; // "/out/" - 5 символов
        *setSubstr = 0; // вместо "/set" конец строки
        auto outId = DATA::getOutIdByName(outName);
        if (outId == -1) {
          return;
        }
        auto out = DATA::getOut(outId);
        bool val = payload.toInt() != 0;
        Serial.print(F("Set out ")); Serial.print(outName); Serial.print(F(" to ")); Serial.println(val);
        out.setValue(outId, val);
        DATA::setOutValue(outId, val);
        MQTT::sendOutState(outId);
      }
    }
  }

  void connect(const uint32_t currMillis) {
    static uint8_t connectsTryCount;
    // реконект раз в 10 секунд
    if (currMillis < lastConnectTime + 10000 && currMillis > lastConnectTime) {
      return;
    }
    if (!ETH::connected) {
      return;
    }
    // если 12 раз (2 минуты) коннект сфейлился - переподключаем ethernet
    if (connectsTryCount > 12) {
      connectsTryCount = 0;
      Serial.println(F("MQTT to many connect attempts, reinit ethernet"));
      ETH::setup();
      delay(10);
      return;
    }
    lastConnectTime = currMillis;
    const String server = DATA::getMqttServer();
    if (server.length() == 0) {
      return;
    }
    client.disconnect();
    client.setServer(server.c_str(), 1883);
    connectsTryCount++;
    Serial.print(F("MQTT connecting to ")); Serial.print(server); Serial.print(F(", attempt ")); Serial.println(connectsTryCount);
    if (!client.connect("") || !client.connected()) {
      Serial.println(F("MQTT connect error, state: ")); Serial.println(client.state());
      return;
    }
    connectsTryCount = 0;
    Serial.print(F("MQTT connected ")); Serial.println(client.connected());
    delay(10);
    subscribe(currMillis);
    return;
  }

  void subscribe(const uint32_t currMillis) {
    static uint8_t subscribeTryCount;
    // попытка подписки раз в 5 секунд
    if (currMillis < lastSubscribeTime + 5000 && currMillis > lastSubscribeTime) {
      return;
    }
    if (!ETH::connected) {
      return;
    }
    if (!client.connected()) {
      Serial.println(F("MQTT subscribe, but not connected "));
      return;
    }
    // если 24  раз (2 минуты) коннект сфейлился - переподключаемся к серверу
    if (subscribeTryCount > 24) {
      subscribeTryCount = 0;
      Serial.println(F("MQTT to many subscribe attempts, reconnect"));
      reconnect();
      return;
    }
    lastSubscribeTime = currMillis;
    const String topic = DATA::getMqttTopic();
    if (topic.length() == 0) {
      return;
    }
    subscribeTryCount++;
    String subscrTopic = String(topic + "/#");
    Serial.print(F("MQTT subscribe to ")); Serial.print(subscrTopic.c_str()); Serial.print(F(", attempt ")); Serial.println(subscribeTryCount);
    if (!client.subscribe(subscrTopic.c_str())) {
      Serial.println(F("MQTT subscribe error, state: ")); Serial.println(client.state());
      return;
    }
    subscribeTryCount = 0;
    subscribed = true;
  }

  void setup() {
    client.setCallback(callback);
    client.setSocketTimeout(1);
    subscribed = false;
    // максимальные значения для немедленного срабатывания условия по времени
    lastConnectTime = 4294967295;
    lastSubscribeTime = 4294967295;
    connect(millis());
  }
  
  void loop(const uint32_t currMillis) {
    if (!ETH::connected) {
      return;
    }
    if (!client.connected()) {
      subscribed = false;
      connect(currMillis);
      return;
    }
    client.loop();
    if (!subscribed) {
      subscribe(currMillis);
      return;
    }
    if (currMillis > lastSendInfoTime + SEND_INFO_PERIOD || currMillis < lastSendInfoTime) {
      sendInfo();
      lastSendInfoTime = currMillis;
    }
  }

  void reconnect() {
    Serial.println(F("MQTT reconnect"));
    lastConnectTime = 4294967295;
    lastSubscribeTime = 4294967295;
    connect(millis());
  }

  void printF(const __FlashStringHelper *pstr) {
    PGM_P p = reinterpret_cast<PGM_P>(pstr);
    static uint8_t buff[64];
    uint16_t pLen = strlen_P(p);
    uint16_t i = 0;
    while (i < pLen) {
      uint8_t len = 64;
      if (i + len > pLen) {
        len = pLen - i;
      }
      memcpy_P(buff, p + i, len);
      i += len;
      client.write(buff, len);
    }
  }

  // <topic>/info
  // {uptime: <uptime>, ip: "<ip>", ins: [<in>,<in>...], outs: [<out>,<out>...]}
  // in: {name: "имя входа", enabled: 0|1, value: 0|1, throttle_time: <0-65535>}
  // out: {name: "имя выхода", enabled: 0|1, value: 0|1}
  void sendInfo() {
    const String server = DATA::getMqttServer();
    const String topic = DATA::getMqttTopic();
    if (!client.connected() || server.length() == 0 || topic.length() == 0) {
      return;
    }
    uint16_t len = 10 + 7 + 9 + 10 + 2;
    String ip = ipToString(Ethernet.localIP());
    String uptime = String(TIME::uptime);
    len += ip.length() + uptime.length();
    for (uint8_t first = 1, i = 0; i < 24; i++) {
      if (first) {
        first = false;
      } else {
        len += 1; // запятая
      }
      len += 50;
      // {"name":"имя входа","enabled":0|1,"value":0|1,"throttle_time":<0-65535>}
      auto in = DATA::getIn(i);
      len += String(in.name).length();
      len += String(in.throttleTime).length();
    }
    for (uint8_t first = 1, i = 0; i < 24; i++) {
      if (first) {
        first = false;
      } else {
        len += 1; // запятая
      }
      len += 33;
      // {"name":"имя выхода","enabled":0|1,"value":0|1}
      auto out = DATA::getOut(i);
      len += String(out.name).length();
    }

    String publishTopic = topic + F("/info");
    if (!client.beginPublish(publishTopic.c_str(), len, false)) {
      Serial.println(F("MQTT sendInfo begin publish error"));
      return;
    }
    
    printF(F("{\"uptime\":"));            // 10
    client.print(uptime);
    printF(F(",\"ip\":\""));              // 7
    client.print(ip);
    printF(F("\",\"ins\":["));            // 9
    for (uint8_t first = 1, i = 0; i < 24; i++) {
      if (first) {
        first = false;
      } else {
        printF(F(","));
      }
      auto in = DATA::getIn(i);
      // {"name":"имя входа","enabled":0|1,"value":0|1,"throttle_time":<0-65535>}
      printF(F("{\"name\":\""));
      client.print(in.name);
      printF(F("\",\"enabled\":"));
      client.print(in.enabled);
      printF(F(",\"value\":"));
      client.print(in.getValue(i));
      printF(F(",\"throttle_time\":"));
      client.print(in.throttleTime);
      printF(F("}"));
    }
    printF(F("],\"outs\":["));              // 10
    for (uint8_t first = 1, i = 0; i < 24; i++) {
      if (first) {
        first = false;
      } else {
        printF(F(","));
      }
      auto out = DATA::getOut(i);
      // {name: "имя выхода", enabled: 0|1, value: 0|1}
      printF(F("{\"name\":\""));
      client.print(out.name);
      printF(F("\",\"enabled\":"));
      client.print(out.enabled);
      printF(F(",\"value\":"));
      client.print(out.value);
      printF(F("}"));
    }
    
    printF(F("]}"));                      // 2
    if (!client.endPublish()) {
      Serial.println(F("MQTT sendInfo end publish error"));
      return;
    }
    Serial.print(F("MQTT sendInfo succ to ")); Serial.println(publishTopic);
  }

  void sendInfoOnNextLoop() {
    lastSendInfoTime = 0;
  }

  // <topic>/in/<имя входа> [R]
  void sendInState(const uint8_t inId) {
    const String server = DATA::getMqttServer();
    const String topic = DATA::getMqttTopic();
    if (!client.connected() || server.length() == 0 || topic.length() == 0) {
      return;
    }
    auto in = DATA::getIn(inId);
    String publishTopic = topic + F("/in/") + in.name;
    const char *msg = in.getValue(inId) ? "1": "0";
    if (!client.publish(publishTopic.c_str(), (const uint8_t*)msg, 2, true)) {
      Serial.println(F("MQTT sendInState publish error"));
      return;
    }
    Serial.print(F("MQTT sendInState succ to ")); Serial.println(publishTopic);
  }

  // <topic>/out/<имя выхода> [R]
  void sendOutState(const uint8_t outId) {
    const String server = DATA::getMqttServer();
    const String topic = DATA::getMqttTopic();
    if (!client.connected() || server.length() == 0 || topic.length() == 0) {
      return;
    }
    auto out = DATA::getOut(outId);
    String publishTopic = topic + F("/out/") + out.name;
    const char *msg = out.value ? "1": "0";
    if (!client.publish(publishTopic.c_str(), (const uint8_t*)msg, 2, true)) {
      Serial.println(F("MQTT sendOutState publish error"));
      return;
    }
    Serial.print(F("MQTT sendOutState succ to ")); Serial.println(publishTopic);
  }
}
