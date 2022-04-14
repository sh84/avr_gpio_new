#include <Arduino.h>
#include "data.h"
#include "time.h"

namespace DATA {
//уникальный токен
// b6 - протечки
// c7 - свет
// d8 - gpio
const uint8_t TOKEN = 0xd8;

const uint8_t crcAddr = 1;
const uint8_t useDHCPAddr = 5;            // bool useDHCP
const uint8_t staticIPAddr = 6;           // uint8_t staticIP[4]
const uint8_t staticSubnetMaskAddr = 10;  // uint8_t staticSubnetMask[4]
const uint8_t staticGatewayIPAddr = 14;   // uint8_t staticGatewayIP[4]
const uint8_t mqttServerAddr = 18;        // char mqttServer[64]
const uint8_t mqttTopicAddr = 82;         // char mqttTopic[64]

const uint16_t insAddr = 146;                                   // LOGIC::In ins[24]
const uint16_t outsAddr = insAddr + sizeof(LOGIC::In[24]);      // LOGIC::Outs outs[24]
const uint16_t lastAddr = outsAddr + sizeof(LOGIC::Out[24]);

const uint16_t defualtThrottleTime = 200; // мс

uint32_t crc;
bool outsValuesBuff[24];
uint32_t outsValuesBuffChangedMap;

#define strcat_(x, y) x ## y
#define strcat(x, y) strcat_(x, y)
#define PRINT_VALUE(x) \
    template <unsigned int> \
    struct strcat(strcat(value_of_, x), _is); \
    static_assert(strcat(strcat(value_of_, x), _is)<x>::x, "");

static_assert(lastAddr < E2END, "Превышение размера flash памяти");
//PRINT_VALUE(lastAddr)

// промежуточная функция вычисления crc очередного байта данных (d), текущее состояие - глобальная переменная crc
void addCRC(const byte d) {
  //Serial.print(' ');Serial.print(d, HEX);
  const uint32_t crcTable[16] = {
    0x00000000, 0x1db71064, 0x3b6e20c8, 0x26d930ac,
    0x76dc4190, 0x6b6b51f4, 0x4db26158, 0x5005713c,
    0xedb88320, 0xf00f9344, 0xd6d6a3e8, 0xcb61b38c,
    0x9b64c2b0, 0x86d3d2d4, 0xa00ae278, 0xbdbdf21c
  };
  crc = crcTable[(crc ^ d) & 0x0f] ^ (crc >> 4);
  crc = crcTable[(crc ^ (d >> 4)) & 0x0f] ^ (crc >> 4);
  crc = ~crc;
}

void addCRC(const void *data, const uint8_t &bytesCount) {
  byte *bytes = (byte*)data;
  for (uint8_t i = 0; i < bytesCount; i++) {
    addCRC(*bytes++);
  }
}

void addCRCUInt16(const uint16_t &val) {
  union {
    uint16_t value;
    uint8_t buffer[2];
  } converter;
  converter.value = val;
  addCRC(converter.buffer[0]);
  addCRC(converter.buffer[1]);
}

void addCRCUInt32(const uint32_t &val) {
  union {
    uint32_t value;
    uint8_t buffer[4];
  } converter;
  converter.value = val;
  addCRC(converter.buffer[0]);
  addCRC(converter.buffer[1]);
  addCRC(converter.buffer[2]);
  addCRC(converter.buffer[3]);
}

// вычисление crc всех текущих данных, результат - глобальная переменная crc
void calcCRC() {
  crc = ~0L;
  addCRC(isUseDHCP());
  addCRCUInt32(uint32_t(getStaticIP()));
  addCRCUInt32(uint32_t(getStaticSubnetMask()));
  addCRCUInt32(uint32_t(getStaticGatewayIP()));
  String str = getMqttServer();
  addCRC(str.c_str(), str.length());
  str = getMqttTopic();
  addCRC(str.c_str(), str.length());

  for (uint8_t i = 0; i < 24; i++) {
    LOGIC::In in = getIn(i);
    addCRC(&in, sizeof(LOGIC::In));
  }
  for (uint8_t i = 0; i < 24; i++) {
    LOGIC::Out out = getOut(i);
    // не считаем crc для value, потому что value сохраняется отдельно и позже
    out.value = 0;
    addCRC(&out, sizeof(LOGIC::Out));
  }
}

void initEmptyData() {
  EEPROM.put(useDHCPAddr, true);
  // default staticIP = 192.168.0.180
  EEPROM.put(staticIPAddr, uint32_t(IPAddress{192, 168, 0, 180}));
  // default staticSubnetMask = 255.255.255.0
  EEPROM.put(staticSubnetMaskAddr, uint32_t(IPAddress{255, 255, 255, 0}));
  // default staticGatewayIP = 192.168.0.1
  EEPROM.put(staticGatewayIPAddr, uint32_t(IPAddress{192, 168, 0, 1}));
  EEPROM.put(mqttServerAddr, "");
  EEPROM.put(mqttTopicAddr, "");
  
  for (uint8_t i = 0; i < 24; i++) {
    auto pin = LOGIC::In{
      throttleTime: defualtThrottleTime,
      enabled: false
    };
    memset(pin.name, 0, sizeof(pin.name));
    strcpy(pin.name, (String("Вход ") + (i + 1)).c_str());
    EEPROM.put(insAddr + i * sizeof(LOGIC::In), pin);
  }
  for (uint8_t i = 0; i < 24; i++) {
    auto out = LOGIC::Out{
      enabled: false
    };
    memset(out.name, 0, sizeof(out.name));
    strcpy(out.name, (String("Выход ") + (i + 1)).c_str());
    out.value = 0;
    EEPROM.put(outsAddr + i * sizeof(LOGIC::Out), out);
  }

  calcCRC();
  EEPROM.put(crcAddr, crc);
  // байт 0 - уникальный токен
  EEPROM.put(0, TOKEN);
}

void load() {
  Serial.println(F("reading eeprom"));

  Serial.print(F("\tuseDHCP: ")); Serial.println(isUseDHCP());
  Serial.print(F("\tstaticIP: ")); Serial.println(getStaticIP());
  Serial.print(F("\tstaticSubnetMask: ")); Serial.println(getStaticSubnetMask());
  Serial.print(F("\tstaticGatewayIP: ")); Serial.println(getStaticGatewayIP());
  Serial.print(F("\tmqttServer: ")); Serial.println(getMqttServer());
  Serial.print(F("\tmqttTopic: ")); Serial.println(getMqttTopic());
  Serial.flush();
  delay(10);

  Serial.println(F("\tins: "));
  for (uint8_t i = 0; i < 24; i++) {
    auto in = getIn(i);
    Serial.print(F("\t\tid=")); Serial.print(i); Serial.print(F(" "));
    Serial.print(F("name: ")); Serial.print(in.name);
    Serial.print(F(", enabled: ")); Serial.print(in.enabled);
    Serial.print(F(", throttleTime: ")); Serial.print(in.throttleTime);
    Serial.println();
    Serial.flush();
    delay(1);
  }
  Serial.println(F("\touts: "));
  for (uint8_t i = 0; i < 24; i++) {
    auto out = getOut(i);
    Serial.print(F("\t\tid=")); Serial.print(i); Serial.print(F(" "));
    Serial.print(F("name: ")); Serial.print(out.name);
    Serial.print(F(", enabled: ")); Serial.print(out.enabled);
    Serial.print(F(", val: ")); Serial.print(out.value);
    Serial.println();
    Serial.flush();
    delay(1);
  }
}

void loadOutsValuesBuff() {
   for (uint8_t i = 0; i < 24; i++) {
     // адрес value вычисляем от конца - он последний
     EEPROM.get(outsAddr + (i + 1) * sizeof(LOGIC::Out) - sizeof(bool), outsValuesBuff[i]);
   }
}

void setup() {
  // байт 0 - уникальный токен
  uint8_t token = EEPROM.read(0);
  if (token == TOKEN) {
    loadOutsValuesBuff();
    load();
    // проверяем целостность данных с помощью crc
    calcCRC();
    uint32_t eepromCrc;
    EEPROM.get(crcAddr, eepromCrc);
    if (eepromCrc == crc) {
      Serial.println(F("eeprom readed"));
    } else {
      Serial.print(F("eeprom crc error, read ")); Serial.print(eepromCrc, HEX); Serial.print(F(", expect ")); Serial.print(crc, HEX); Serial.println(F(", reinit"));
      initEmptyData();
    }
  } else {
    Serial.print(F("eeprom empty (token = ")); Serial.print(token, HEX), Serial.println(F("), init"));
    initEmptyData();
  }
  Serial.flush();
  delay(10);
}

void loop(const uint32_t currMillis) {
  static uint32_t lastCheckTime;
  // сохраняем значение out-ов раз в минуту
  if (currMillis > lastCheckTime + 6000 || currMillis < lastCheckTime) {
    lastCheckTime = currMillis;
    for (uint8_t i = 0; i < 24; i++) {
      if (outsValuesBuffChangedMap & bit(i)) {
        Serial.print("Save out "); Serial.print (i); Serial.print(" value: "); Serial.println(outsValuesBuff[i]);
        // адрес value вычисляем от конца - он последний
        EEPROM.put(outsAddr + (i + 1) * sizeof(LOGIC::Out) - sizeof(bool), outsValuesBuff[i]);
      }
    }
    outsValuesBuffChangedMap = 0;
  }
}

bool isUseDHCP() {
  bool useDHCP;
  EEPROM.get(useDHCPAddr, useDHCP);
  return useDHCP;
}

void saveUseDHCP(const bool val) {
  EEPROM.put(useDHCPAddr, val);
  calcCRC();
  EEPROM.put(crcAddr, crc);
}

const IPAddress getStaticIP() {
  uint32_t ip;
  EEPROM.get(staticIPAddr, ip);
  return IPAddress(ip);
}

void setStaticIP(const IPAddress ip) {
  EEPROM.put(staticIPAddr, uint32_t(ip));
  calcCRC();
  EEPROM.put(crcAddr, crc);
}

const IPAddress getStaticSubnetMask() {
  uint32_t ip;
  EEPROM.get(staticSubnetMaskAddr, ip);
  return IPAddress(ip);
}

void setStaticSubnetMask(const IPAddress ip) {
  EEPROM.put(staticSubnetMaskAddr, uint32_t(ip));
  calcCRC();
  EEPROM.put(crcAddr, crc);
}

const IPAddress getStaticGatewayIP() {
  uint32_t ip;
  EEPROM.get(staticGatewayIPAddr, ip);
  return IPAddress(ip);
}

void setStaticGatewayIP(const IPAddress ip) {
  EEPROM.put(staticGatewayIPAddr, uint32_t(ip));
  calcCRC();
  EEPROM.put(crcAddr, crc);
}

const String getMqttServer() {
  char mqttServer[64] = {};
  EEPROM.get(mqttServerAddr, mqttServer);
  return String(mqttServer);
}

void setMqttServer(const String & str) {
  char mqttServer[64] = {};
  strncpy(mqttServer, str.c_str(), 64);
  mqttServer[63] = 0;
  EEPROM.put(mqttServerAddr, mqttServer);
  calcCRC();
  EEPROM.put(crcAddr, crc);
}

const String getMqttTopic() {
  char mqttTopic[64] = {};
  EEPROM.get(mqttTopicAddr, mqttTopic);
  return String(mqttTopic);
}

void setMqttTopic(const String & str) {
  char mqttTopic[64] = {};
  strncpy(mqttTopic, str.c_str(), 64);
  mqttTopic[63] = 0;
  EEPROM.put(mqttTopicAddr, mqttTopic);
  calcCRC();
  EEPROM.put(crcAddr, crc);
}

const LOGIC::In getIn(const uint8_t id) {
  if (id >= 24) {
    Serial.print(F("ERROR: getIn for wrong id ")); Serial.println(id);
    return LOGIC::In{};
  }
  LOGIC::In in;
  EEPROM.get(insAddr + id * sizeof(LOGIC::In), in);
  return in;
}

void setIn(const uint8_t id, const LOGIC::In in) {
  if (id >= 24) {
    Serial.print(F("ERROR: setIn for wrong id ")); Serial.println(id);
    return;
  }
  EEPROM.put(insAddr + id * sizeof(LOGIC::In), in);
  calcCRC();
  EEPROM.put(crcAddr, crc);
}
 
LOGIC::Out getOut(const uint8_t id) {
  LOGIC::Out out;
  if (id >= 24) {
    Serial.print(F("ERROR: getOut for wrong id ")); Serial.println(id);
    return out;
  }
  EEPROM.get(outsAddr + id * sizeof(LOGIC::Out), out);
  out.value = outsValuesBuff[id];
  return out;
}

// вернет -1, если out не найден
const int8_t getOutIdByName(const char *name) {
  LOGIC::Out out;
  for (uint8_t id = 0; id < 24; id++) {
    EEPROM.get(outsAddr + id * sizeof(LOGIC::Out), out);
    if (strcmp(out.name, name) == 0) {
      return id;
    }
  }
  Serial.print(F("ERROR: getOutIdByName for wrong name ")); Serial.println(name);
  return -1;
}

void setOut(const uint8_t id, const LOGIC::Out val) {
  if (id >= 24) {
    Serial.print(F("ERROR: setOut for wrong id ")); Serial.println(id);
    return;
  }
  EEPROM.put(outsAddr + id * sizeof(LOGIC::Out), val);
  calcCRC();
  EEPROM.put(crcAddr, crc);
  outsValuesBuff[id] = val.value;
}

void setOutValue(const uint8_t id, const uint32_t val) {
  if (id >= 24) {
    Serial.print(F("ERROR: setOutValue for wrong id ")); Serial.println(id);
    return;
  }
  outsValuesBuff[id] = val;
  bitSet(outsValuesBuffChangedMap, id);
}
  
}
