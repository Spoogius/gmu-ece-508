#include <ArduinoBLE.h>

void setup() {
  Serial.begin(115200);
  while (!Serial);
  pinMode(LED_BUILTIN, OUTPUT);
  // begin initialization
  if (!BLE.begin()) {
    Serial.println("starting BLE failed!");
    while (1);
  }
  BLE.setLocalName("HelloBLE");
  BLE.setAdvertisedServiceUuid("A3A2FDA5-1581-44CA-B62F-7F99F485EC37");
  // start advertising
  BLE.advertise();
  Serial.println("Bluetooth device active, waiting for connections...");
}

void loop() {
  // wait for a BLE central
  BLEDevice central = BLE.central();
  if (central) {
    Serial.print("Connected to central: ");
    Serial.println(central.address());
    digitalWrite(LED_BUILTIN, HIGH);
    while (central.connected()) {
      // do nothing
    }
    digitalWrite(LED_BUILTIN, LOW);
    Serial.print("Disconnected from central: ");
    Serial.println(central.address());
  }
}