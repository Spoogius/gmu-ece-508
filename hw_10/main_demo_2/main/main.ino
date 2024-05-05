#include <ArduinoBLE.h>
BLEService ledService("19B10000-E8F2-537E-4F6C-D104768A1214");
BLEByteCharacteristic switchCharacteristic("A3A2FDA5-1581-44CA-B62F-7F99F485EC37", BLERead | BLEWrite);
const int ledPin = LED_BUILTIN;

void setup() {
  Serial.begin(9600);
  while (!Serial);
  // set LED pin to output mode
  pinMode(ledPin, OUTPUT);
  // begin initialization
  if (!BLE.begin()) {
    Serial.println("starting BLE failed!");
    while (1);
  }
  // set advertised local name and service UUID:
  BLE.setLocalName("LED");
  BLE.setAdvertisedService(ledService);
  // add the characteristic to the service
  ledService.addCharacteristic(switchCharacteristic);
  // add service
  BLE.addService(ledService);
  // set the initial value for the characeristic:
  switchCharacteristic.writeValue(0);
  // start advertising
  BLE.advertise();
  Serial.println("BLE LED Peripheral");
}

void loop() {
  BLEDevice central = BLE.central();
   if (central) {
    Serial.print("Connected to central: ");
    // print the central's MAC address:
    Serial.println(central.address());
    // while the central is still connected to peripheral:
    while (central.connected()) {
      // if the remote device wrote to the characteristic,
      // use the value to control the LED:
      if (switchCharacteristic.written()) {
        int val = switchCharacteristic.value();
        Serial.println(val);
        if (val>0) {   // any value other than 0
          Serial.println("LED on");
          digitalWrite(ledPin, HIGH);         // will turn the LED on
        } else {                              // a 0 value
          Serial.println(F("LED off"));
          digitalWrite(ledPin, LOW);          // will turn the LED off
        }
      }
    }
    Serial.print(F("Disconnected from central: "));
    Serial.println(central.address());
  }
}