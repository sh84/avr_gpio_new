#include "http_server.h"
#include "lib.h"
#include "data.h"
#include "logic.h"
#include "ethernet.h"
#include "time.h"
#include "mqtt.h"

/* Роуты:
 *  /                                  - html страница
 *  /info                              - json, основные данные 
 *  /net_set_ip?ip=<ip>                - установка статического IP
 *  /net_set_subnet?ip=<ip>            - установка маски подсети
 *  /net_set_gateway_ip?ip=<ip>        - установка IP шлюза
 *  /net_set_dhcp?dhcp=<dhcp>          - включение/выключение DHCP, значение 0 или 1
 *  /mqtt_set_server?server=<server>   - изменение MQTT сервера
 *  /mqtt_set_topic?topic=<topic>      - изменение MQTT топика
 *  /in_change?id=<id>&enabled=<0|1>&name=<name>&throttle_time=<0..65535> - изменение in с ид <id>
 *  /out_change?id=<id>&enabled=<0|1>&name=<name> - изменение out с ид <id>
 *  /out_set_value?id=<id>&val=<0|1>   - изменение значения out-а <id> на <val>
 */

 namespace HTTP_SERVER {
  const uint8_t VERSION = 1;
  const uint8_t PORT = 80;
  const uint16_t REQ_TIMEOUT = 1900; // мс
  EthernetServer server(PORT);
  static char buff[255];

  const PROGMEM char pageHtml[] =
#include "page.html.h"
;

  void write(EthernetClient &client, PGM_P p) {
    uint16_t i = 0;
    uint16_t pLen = strlen_P(p);
    while (i < pLen) {
      uint8_t len = 255;
      if (i + len > pLen) {
        len = pLen - i;
      }
      memcpy_P(buff, p + i, len);
      i += len;
      client.write(buff, len);
    }
  }
  
  void printF(EthernetClient &client, const __FlashStringHelper *pstr) {
    PGM_P p = reinterpret_cast<PGM_P>(pstr);
    write(client, p);
  }
  
  void print404(EthernetClient &client) {
    printF(client, F("HTTP/1.0 404 Not found\nAccess-Control-Allow-Origin: *\nConnection: close\n\n"));
  }

  void printOK(EthernetClient &client) {
    printF(client, F("HTTP/1.0 200 OK\nContent-Type: application/json; charset=utf-8\nContent-Length: 2\nAccess-Control-Allow-Origin: *\nConnection: close\n\n{}"));
  }

  void printError(EthernetClient &client, const char* err) {
    uint16_t len = strlen(err) + 10;
    printF(client, F("HTTP/1.0 200 OK\nContent-Type: application/json; charset=utf-8\nContent-Length: "));
    client.print(len);
    printF(client, F("\nAccess-Control-Allow-Origin: *\nConnection: close\n\n{\"error\":\""));
    client.print(err);
    printF(client, F("\"}"));
  }

  void printErrorF(EthernetClient &client, const __FlashStringHelper *err) {
    PGM_P p = reinterpret_cast<PGM_P>(err);
    uint16_t len =  strlen_P(p) + 12;
    printF(client, F("HTTP/1.0 200 OK\nContent-Type: application/json; charset=utf-8\nContent-Length: "));
    client.print(len);
    printF(client, F("\nAccess-Control-Allow-Origin: *\nConnection: close\n\n{\"error\":\""));
    write(client, p);
    printF(client, F("\"}"));
  }

  void serialFinishUrlRoute(const uint16_t code, const uint32_t startMillis) {
    Serial.print(F("HTTP send "));
    Serial.print(code);
    Serial.print(F(" by "));
    Serial.print(millis() - startMillis);
    Serial.println(F("ms"));
  }

  unsigned char h2int(char c) {
    if (c >= '0' && c <= '9') {
      return (unsigned char)c - '0';
    }
    if (c >= 'a' && c <= 'f') {
      return (unsigned char)c - 'a' + 10;
    }
    if (c >= 'A' && c <='F') {
        return (unsigned char)c - 'A' + 10;
    }
    return 0;
  }

  String urlDecode(String str) {
    String encodedString;
    char c, code0, code1;
    for (uint8_t i = 0; i < str.length(); i++) {
      c = str.charAt(i);
      if (c == '+') {
        encodedString += ' ';
      } else if (c == '%') {
        i++;
        code0 = str.charAt(i);
        i++;
        code1 = str.charAt(i);
        c = (h2int(code0) << 4) | h2int(code1);
        encodedString += c;
      } else{
        encodedString += c;  
      }
    }
    return encodedString;
  }

  // ищет параметр paramName в урле (paramName должен быть в формате "param=")
  // если не находит - возврашает false
  // если находит - возвращает true и модифицирует paramVal на значение найденного параметра
  bool getParamFromUrl(const String &url, const char *paramName, String &paramVal) {
    uint16_t paramPos = url.lastIndexOf(paramName);
    if (paramPos == -1) {
      return false;
    }
    uint16_t paramEnd = url.indexOf("&", paramPos + 1); 
    if (paramEnd != -1) {
      paramVal = url.substring(paramPos + strlen(paramName), paramEnd);
    } else {
      paramVal = url.substring(paramPos + strlen(paramName));
    }
    paramVal = urlDecode(paramVal);
    return true;
  }

  // ROUTES

  // /
  void routeMainPage(EthernetClient &client, const uint32_t startMillis) {
    printF(client, F("HTTP/1.0 200 OK\nContent-Type: text/html; charset=utf-8\nContent-Length: "));
    client.println(strlen_P(pageHtml));
    printF(client, F("Access-Control-Allow-Origin: *\nConnection: close\n\n"));
    write(client, pageHtml);
    serialFinishUrlRoute(200, startMillis);
  }

  // /info
  void routeInfo(EthernetClient &client, const uint32_t startMillis) {
    printF(client, F("HTTP/1.0 200 OK\nContent-Type: application/json; charset=utf-8\nAccess-Control-Allow-Origin: *\nConnection: close\n\n"));
    printF(client, F("{\"version\":"));
    client.print(VERSION);
    printF(client, F(",\"ip\":\""));
    client.print(ETH::getIP());
    printF(client, F("\",\"dhcp\":"));
    printF(client, DATA::isUseDHCP() ? F("true") : F("false"));
    printF(client, F(",\"static_ip\":\""));
    client.print(DATA::getStaticIP());
    printF(client, F("\",\"subnet_mask\":\""));
    client.print(DATA::getStaticSubnetMask());
    printF(client, F("\",\"gateway_ip\":\""));
    client.print(DATA::getStaticGatewayIP());
    printF(client, F("\",\"mqtt_server\":\""));
    client.print(DATA::getMqttServer());
    printF(client, F("\",\"mqtt_topic\":\""));
    client.print(DATA::getMqttTopic());
    printF(client, F("\",\"mqtt_connected\":"));
    client.print(MQTT::subscribed);
    printF(client, F(",\"uptime\":"));
    client.print(TIME::uptime);

    printF(client, F(",\"ins\":["));
    for (uint8_t i = 0; i < 24; i++) {
      if (i > 0) printF(client, F(","));
      auto in = DATA::getIn(i);
      printF(client, F("{\"enabled\":"));
      client.print(in.enabled);
      printF(client, F(",\"name\":\""));
      client.print(in.name);
      printF(client, F("\",\"value\":"));
      client.print(in.getValue(i));
      printF(client, F(",\"throttle_time\":"));
      client.print(in.throttleTime);
      printF(client, F("}"));
    }
    
    printF(client, F("],\"outs\":["));
    for (uint8_t i = 0; i < 24; i++) {
      if (i > 0) printF(client, F(","));
      auto out = DATA::getOut(i);
      printF(client, F("{\"enabled\":"));
      client.print(out.enabled);
      printF(client, F(",\"name\":\""));
      client.print(out.name);
      printF(client, F("\",\"value\":"));
      client.print(out.value);
      printF(client, F("}"));
    }    
    printF(client, F("]}"));
    serialFinishUrlRoute(200, startMillis);
  }

  bool getIpParam(EthernetClient &client, const String &url, const uint32_t startMillis, IPAddress &ip) {
    String paramVal;
    if (!getParamFromUrl(url, "ip=", paramVal)) {
      print404(client);
      Serial.println(F("Incorrect url: ip not found"));
      serialFinishUrlRoute(404, startMillis);
      return false;
    }
    if (!ip.fromString(paramVal.c_str())) {
      printErrorF(client, F("wrong ip"));
      Serial.println(F("Wrong ip"));
      serialFinishUrlRoute(200, startMillis);
      return false;
    } else {
      return true;
    }
  }

  // /net_set_ip?ip=<ip>
  void routeSetIp(EthernetClient &client, const String &url, const uint32_t startMillis) {
    IPAddress ip;
    if (getIpParam(client, url, startMillis, ip)) {
      DATA::setStaticIP(ip);
      printOK(client);
      serialFinishUrlRoute(200, startMillis);
      if (!DATA::isUseDHCP()) {
        ETH::setStaticIp();
        MQTT::reconnect();
      }
    }
  }

  // /net_set_subnet?ip=<ip>
  void routeSetSubNet(EthernetClient &client, const String &url, const uint32_t startMillis) {
    IPAddress ip;
    if (getIpParam(client, url, startMillis, ip)) {
      DATA::setStaticSubnetMask(ip);
      printOK(client);
      serialFinishUrlRoute(200, startMillis);
      if (!DATA::isUseDHCP()) {
        ETH::setStaticIp();
        MQTT::reconnect();
      }
    }
  }

  // /net_set_gateway_ip?ip=<ip>
  void routeSetGatewayIp(EthernetClient &client, const String &url, const uint32_t startMillis) {
    IPAddress ip;
    if (getIpParam(client, url, startMillis, ip)) {
      DATA::setStaticGatewayIP(ip);
      printOK(client);
      serialFinishUrlRoute(200, startMillis);
      if (!DATA::isUseDHCP()) {
        ETH::setStaticIp();
        MQTT::reconnect();
      }
    }
  }

  // /net_set_dhcp?dhcp=<dhcp>
  void routeSetDHCP(EthernetClient &client, const String &url, const uint32_t startMillis) {
    String paramVal;
    if (!getParamFromUrl(url, "dhcp=", paramVal)) {
      print404(client);
      Serial.println(F("Incorrect url: dhcp not found"));
      serialFinishUrlRoute(404, startMillis);
      return;
    }
    if (paramVal[0] == '0') {
      DATA::saveUseDHCP(false);
      printOK(client);
      serialFinishUrlRoute(200, startMillis);
      ETH::setStaticIp();
    } else {
      DATA::saveUseDHCP(true);
      printOK(client);
      serialFinishUrlRoute(200, startMillis);
      ETH::setDHCP();
    }
    MQTT::reconnect();
  }
  
  // /mqtt_set_server?server=<server>
  void routeMqttSetServer(EthernetClient &client, const String &url, const uint32_t startMillis) {
    String paramVal;
    if (!getParamFromUrl(url, "server=", paramVal)) {
      print404(client);
      Serial.println(F("Incorrect url: name not found"));
      serialFinishUrlRoute(404, startMillis);
      return;
    }
    DATA::setMqttServer(paramVal.c_str());
    printOK(client);
    serialFinishUrlRoute(200, startMillis);
    MQTT::reconnect();
  }
  
  // /mqtt_set_topic?topic=<topic>
  void routeMqttSetTopic(EthernetClient &client, const String &url, const uint32_t startMillis) {
    String paramVal;
    if (!getParamFromUrl(url, "topic=", paramVal)) {
      print404(client);
      Serial.println(F("Incorrect url: name not found"));
      serialFinishUrlRoute(404, startMillis);
      return;
    }
    DATA::setMqttTopic(paramVal.c_str());
    printOK(client);
    serialFinishUrlRoute(200, startMillis);
    MQTT::reconnect();
  }

  // /in_change?id=<id>&enabled=<0|1>&name=<name>&throttle_time=<0..65535>
  void routeInChange(EthernetClient &client, const String &url, const uint32_t startMillis) {
    String paramVal;
    if (!getParamFromUrl(url, "id=", paramVal)) {
      print404(client);
      Serial.println(F("Incorrect url: id not found"));
      serialFinishUrlRoute(404, startMillis);
      return;
    }
    auto id = paramVal.toInt();
    if (id < 0 || id >= 24) {
      print404(client);
      Serial.println(F("Incorrect url: wrong id"));
      serialFinishUrlRoute(404, startMillis);
      return;
    }
    if (!getParamFromUrl(url, "enabled=", paramVal)) {
      print404(client);
      Serial.println(F("Incorrect url: enabled not found"));
      serialFinishUrlRoute(404, startMillis);
      return;
    }
    LOGIC::In in;
    in.enabled = paramVal.toInt() != 0;
    if (!getParamFromUrl(url, "name=", paramVal)) {
      print404(client);
      Serial.println(F("Incorrect url: name not found"));
      serialFinishUrlRoute(404, startMillis);
      return;
    }
    strncpy(in.name, paramVal.c_str(), sizeof(in.name)-1);
    in.name[sizeof(in.name) - 1] = 0;
    if (!getParamFromUrl(url, "throttle_time=", paramVal)) {
      print404(client);
      Serial.println(F("Incorrect url: throttle_time not found"));
      serialFinishUrlRoute(404, startMillis);
      return;
    }
    in.throttleTime = paramVal.toInt();
    
    DATA::setIn(id, in);
    MQTT::sendInfoOnNextLoop();
    MQTT::sendInState(id);
    printOK(client);
    serialFinishUrlRoute(200, startMillis);
  }

  // /out_change?id=<id>&enabled=<0|1>&name=<name>
  void routeOutChange(EthernetClient &client, const String &url, const uint32_t startMillis) {
    String paramVal;
    if (!getParamFromUrl(url, "id=", paramVal)) {
      print404(client);
      Serial.println(F("Incorrect url: id not found"));
      serialFinishUrlRoute(404, startMillis);
      return;
    }
    auto id = paramVal.toInt();
    if (id < 0 || id >= 24) {
      print404(client);
      Serial.println(F("Incorrect url: wrong id"));
      serialFinishUrlRoute(404, startMillis);
      return;
    }
    if (!getParamFromUrl(url, "enabled=", paramVal)) {
      print404(client);
      Serial.println(F("Incorrect url: enabled not found"));
      serialFinishUrlRoute(404, startMillis);
      return;
    }
    LOGIC::Out out;
    out.enabled = paramVal.toInt() != 0;
    if (!getParamFromUrl(url, "name=", paramVal)) {
      print404(client);
      Serial.println(F("Incorrect url: name not found"));
      serialFinishUrlRoute(404, startMillis);
      return;
    }
    strncpy(out.name, paramVal.c_str(), sizeof(out.name)-1);
    out.name[sizeof(out.name) - 1] = 0;
    out.value = 0;
    
    DATA::setOut(id, out);
    MQTT::sendInfoOnNextLoop();
    MQTT::sendOutState(id);
    printOK(client);
    serialFinishUrlRoute(200, startMillis);
  }

  // /out_set_value?id=<id>&val=<0|1>
  void routeOutSetValue(EthernetClient &client, const String &url, const uint32_t startMillis) {
    String paramVal;
    if (!getParamFromUrl(url, "id=", paramVal)) {
      print404(client);
      Serial.println(F("Incorrect url: id not found"));
      serialFinishUrlRoute(404, startMillis);
      return;
    }
    uint8_t id = paramVal.toInt();
    if (id < 0 || id >= 24) {
      print404(client);
      Serial.println(F("Incorrect url: wrong id"));
      serialFinishUrlRoute(404, startMillis);
      return;
    }
    if (!getParamFromUrl(url, "val=", paramVal)) {
      print404(client);
      Serial.println(F("Incorrect url: val not found"));
      serialFinishUrlRoute(404, startMillis);
      return;
    }
    auto out = DATA::getOut(id);
    out.setValue(id, paramVal.toInt() != 0);
    DATA::setOutValue(id, out.value);
    MQTT::sendOutState(id);
    printOK(client);
    serialFinishUrlRoute(200, startMillis);
  }
    
  void setup() {
    server.begin();
    Serial.print(F("HTTP Server started at "));
    Serial.print(Ethernet.localIP());
    Serial.print(":");
    Serial.println(PORT);
  }

  void loop(const uint32_t currMillis) {
    EthernetClient client = server.available();
    if (!client || !client.connected()) {
      return;
    }
    String req = "";
    IPAddress remoteIP = client.remoteIP();
    uint16_t reqLen = 0;
    uint32_t timeoutTime = currMillis + REQ_TIMEOUT;
    while (client.connected()) {
      if (millis() > timeoutTime) {
        Serial.print(F("HTTP [")); Serial.print(remoteIP); Serial.print(F("] req timeout, req: "));
        Serial.println(req);
        break;
      }
      if (client.available()) {
        char c = client.read();
        if (reqLen == 500) {
          c = '\n';
          Serial.println(F("HTTP req too long"));
        }
        if (c != '\n' && c != '\r') {
          req += c;
          reqLen++;
          continue;
        }
        if (!req.startsWith("GET ")) {
          print404(client);
          Serial.print(F("HTTP [")); Serial.print(remoteIP); Serial.print(F("] "));
          Serial.print(req);
          Serial.println(F(" - not GET, 404"));
          break;
        }
        uint16_t httpPos = req.lastIndexOf(" HTTP");
        if (httpPos == -1 || httpPos < 5) {
          print404(client);
          Serial.print(F("HTTP [")); Serial.print(remoteIP); Serial.print(F("] "));
          Serial.print(req);
          Serial.println(F(" - bad req, 404"));
          break;
        }
        String url = req.substring(4, httpPos);
        Serial.print(F("HTTP [")); Serial.print(remoteIP); Serial.print(F("] GET "));
        Serial.println(url);
        if (url == "/") {
          routeMainPage(client, currMillis);
        } else if (url == "/info") {
          routeInfo(client, currMillis);
        } else if (url.startsWith(F("/net_set_ip?"))) {
          routeSetIp(client, url, currMillis);
        } else if (url.startsWith(F("/net_set_subnet?"))) {
          routeSetSubNet(client, url, currMillis);
        } else if (url.startsWith(F("/net_set_gateway_ip?"))) {
          routeSetGatewayIp(client, url, currMillis);
        } else if (url.startsWith(F("/net_set_dhcp"))) {
          routeSetDHCP(client, url, currMillis);
        } else if (url.startsWith(F("/mqtt_set_server"))) {
          routeMqttSetServer(client, url, currMillis);
        } else if (url.startsWith(F("/mqtt_set_topic"))) {
          routeMqttSetTopic(client, url, currMillis);
        } else if (url.startsWith(F("/in_change"))) {
          routeInChange(client, url, currMillis);
        } else if (url.startsWith(F("/out_change"))) {
          routeOutChange(client, url, currMillis);
        } else if (url.startsWith(F("/out_set_value"))) {
          routeOutSetValue(client, url, currMillis);
        } else {
          print404(client);
          serialFinishUrlRoute(404, currMillis);
        }
        break;
      }
    }
    delay(1);
    client.stop();
  }
}
