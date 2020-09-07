#include "arduino_secrets.h"
#include "thingProperties.h"
#include <ArduinoBLE.h>

BLEService ledService("1802"); // BLE LED Service

// BLE LED Switch Characteristic - custom 128-bit UUID, read and writable by central
BLEByteCharacteristic switchCharacteristic("1802", BLERead | BLENotify | BLEWrite);

int ledPin = LED_BUILTIN;
int ledConst;

void setup() {
  // Initialize serial and wait for port to open:
  Serial.begin(9600);
  // This delay gives the chance to wait for a Serial Monitor without blocking if none is found
  delay(1500); 
  
  // Defined in thingProperties.h
  initProperties();

  // Connect to Arduino IoT Cloud
  ArduinoCloud.begin(ArduinoIoTPreferredConnection);
  
  setDebugMessageLevel(2);
  ArduinoCloud.printDebugInfo();
  
  ledConst = ledSwitch;
  
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

  switchCharacteristic.writeValue(ledSwitch);

  BLE.advertise();

  
  Serial.println("BLE LED Peripheral");
}

void loop() {
  ArduinoCloud.update();
  // Your code here 
  
  delay(100);
  // listen for BLE peripherals to connect:
  BLEDevice central = BLE.central();

  // if a central is connected to peripheral:
  if (central) {
    Serial.print("Connected to central: ");
    // print the central's MAC address:
    Serial.println(central.address());

     if (switchCharacteristic.written()) {
        if (ledSwitch == 1) {
          digitalWrite(ledPin, HIGH); // LED OFF
          ledSwitch = 0;
          ledConst = ledSwitch;
          BLE.advertise();
        }
        else {
          digitalWrite(ledPin, LOW); // LED ON
          ledSwitch = 1;
          ledConst = ledSwitch;
          BLE.advertise();
        } 
      }

    // when the central disconnects, print it out:
    Serial.print(F("Disconnected from central: "));
    Serial.println(central.address());
  }
}


void onLedSwitchChange() {
  if (ledSwitch != ledConst) {
    if (ledSwitch == 1) {
    digitalWrite(ledPin, LOW); // LED ON
    // set the initial value for the characeristic:
    switchCharacteristic.writeValue(1);
    BLE.advertise();
    ledConst = ledSwitch;
    }
    else {
      digitalWrite(ledPin, HIGH); // LED OFF
      // set the initial value for the characeristic:
      switchCharacteristic.writeValue(0);  
      BLE.advertise();
      ledConst = ledSwitch;
    }
  }
}
