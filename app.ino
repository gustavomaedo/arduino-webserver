#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <ESP8266WebServer.h>
#include <EEPROM.h>

void setup() {
  Serial.begin(9600);
  setPins();
  setServer();
}

void loop() {
  // put your main code here, to run repeatedly:
  serverLoop();
}
