#include <avr/wdt.h>
#include "ethernet.h"
#include "lib.h"
#include "data.h"

namespace ETH {
  bool connected = false;
  bool inited = false;
  const int8_t outPinLanSS = 8;
  // DEEDBAFE0303 - протечки ванная
  // DEEDBAFE0404 - протечки кухня
  // DEEDBAFE0505 - свет
  // DEEDBAFE0606 - gpio
  byte mac[] = {0xDE, 0xED, 0xBA, 0xFE, 0x06, 0x06};
  const uint16_t DHCP_TIMEOUT = 20000; //мс

  bool checkAndPrintErrors(bool checkDHCP) {
    if (Ethernet.hardwareStatus() == EthernetNoHardware) {
      Serial.println(F("Ethernet shield was not found."));
      connected = false;
      return true;
    }
    if (Ethernet.linkStatus() == LinkOFF) {
      Serial.println(F("Ethernet cable is not connected."));
      connected = false;
      return true;
    }
    if (checkDHCP && DATA::isUseDHCP()) {
      IPAddress ip = Ethernet.localIP();
      if (ip[0] == 0 && ip[1] == 0 && ip[2] == 0 && ip[3] == 0) {
        Serial.println(F("DHCP error."));
        connected = false;
        return true;
      }
    }
    connected = true;
    return false;
  }
  
  void setup() {
    connected = false;
    pinMode(outPinLanSS, OUTPUT);
    Ethernet.init(outPinLanSS);
    Ethernet.setRetransmissionCount(5);
    if (DATA::isUseDHCP()) {
      ETH::setDHCP();
    } else {
      ETH::setStaticIp(); 
    }
    delay(1);
    inited = true;
  }

  void setStaticIp() {
    Serial.print(F("Ethernet initialize with static IP "));
    Serial.print(DATA::getStaticIP());
    Serial.print(F(", gateway IP "));
    Serial.print(DATA::getStaticGatewayIP());
    Serial.print(F(", subnet mask "));
    Serial.println(DATA::getStaticSubnetMask());
    Ethernet.begin(mac, DATA::getStaticIP(), {8,8,8,8}, DATA::getStaticGatewayIP(), DATA::getStaticSubnetMask());
    if (checkAndPrintErrors(false)) {
      return;
    }
    String ip = ipToString(Ethernet.localIP());
    Serial.print(F("Ethernet IP address: ")); Serial.println(ip);
  }

  void setDHCP() {
    wdt_disable();
    Serial.println(F("Ethernet initialize with DHCP"));
    if (inited && checkAndPrintErrors(false) || Ethernet.begin(mac, DHCP_TIMEOUT, 2000) == 0 || checkAndPrintErrors()) {
      Serial.println(F("Ethernet failed to configure using DHCP"));
      wdt_reset();
      wdt_enable(WDTO_2S);
      return;
    }
    String ip = ipToString(Ethernet.localIP());
    Serial.print(F("Ethernet IP address: ")); Serial.println(ip);
    wdt_reset();
    wdt_enable(WDTO_2S);
  }

  IPAddress getIP() {
    return Ethernet.localIP();
  }

  void loop(const uint32_t currMillis) {
    static uint32_t checkTime = 0;
    // раз в 30 секунд делаем проверку статуса соединения и dhcp renew
    if (currMillis > checkTime + 30000 || currMillis < checkTime) {
      checkTime = currMillis;
      if (checkAndPrintErrors()) {
        if (DATA::isUseDHCP()) {
          setDHCP();
        }
        return;
      }
      if (!DATA::isUseDHCP()) {
        return;
      }
      IPAddress oldIp = Ethernet.localIP();
      auto maintainResult = Ethernet.maintain();
      IPAddress ip = Ethernet.localIP();
      switch (maintainResult) {
        case 1:
          // renewed fail
          Serial.println(F("Ethernet DHCP renewed fail"));
          break;
        case 2:
          //renewed success
          Serial.println(F("Ethernet DHCP renewed success"));
          if (ip != oldIp) {
            Serial.print(F("Ethernet IP address: ")); Serial.println(ip);
          }
          break;
        case 3:
          //rebind fail
          Serial.println(F("Ethernet DHCP rebind fail"));
          break;
        case 4:
          //rebind success
          Serial.println(F("Ethernet DHCP rebind success"));
          if (ip != oldIp) {
            Serial.print(F("Ethernet IP address: ")); Serial.println(ip);
          }
          break;
      }
    }
  }
}
