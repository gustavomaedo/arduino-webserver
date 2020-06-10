//Sonoff
int led = 13;
int relay = 12;

//LCTech
uint8_t relayOn[] = {0xA0, 0x01, 0x00, 0xA1};
uint8_t relayOff[] = {0xA0, 0x01, 0x01, 0xA2};

bool isRelay = true;

void setPins() {
  pinMode(led, OUTPUT);
  pinMode(relay, OUTPUT);
  turnOn();
}

void turnOff() {
  digitalWrite(led, LOW);
  digitalWrite(relay, HIGH);
  Serial.write(relayOff, 4);
  isRelay = true;
  delay(300);
}

void turnOn() {
  digitalWrite(led, HIGH);
  digitalWrite(relay, LOW);
  Serial.write(relayOn, 4);
  isRelay = false;
  delay(300);
}

bool getRelay() {
  return isRelay;
}
