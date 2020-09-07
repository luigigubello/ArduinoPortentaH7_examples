/*
  SensorTag Button

  This example scans for BLE peripherals until a TI SensorTag is discovered.
  It then connects to it, discovers the attributes of the 0xffe0 service,
  subscribes to the Simple Key Characteristic (UUID 0xffe1). When a button is
  pressed on the SensorTag a notification is received and the button state is
  outputted to the Serial Monitor when one is pressed.

  The circuit:
  - Arduino MKR WiFi 1010, Arduino Uno WiFi Rev2 board, Arduino Nano 33 IoT,
    Arduino Nano 33 BLE, or Arduino Nano 33 BLE Sense board.
  - TI SensorTag

  This example code is in the public domain.
*/

#include <ArduinoBLE.h>
#include <Arduino_APDS9960.h>

const int ledPin2 = 23;
int ledStatus = 0;
int gestureStatus = 0;

void setup() {
  Serial.begin(9600);

  // begin initialization
  if (!BLE.begin()) {
    Serial.println("starting BLE failed!");

    while (1);
  }

  if (!APDS.begin()) {
    Serial.println("Error initializing APDS9960 sensor!");
  }
  
  pinMode(23, OUTPUT);

  Serial.println("BLE Central - SensorTag button");
  Serial.println("Make sure to turn on the device.");

  // start scanning for peripheral
  BLE.scan();
}

void loop() {
  // check if a peripheral has been discovered
  BLEDevice peripheral = BLE.available();

  if (peripheral) {
    // discovered a peripheral, print out address, local name, and advertised service
    Serial.print("Found ");
    Serial.print(peripheral.address());
    Serial.print(" '");
    Serial.print(peripheral.localName());
    Serial.print("' ");
    Serial.print(peripheral.advertisedServiceUuid());
    Serial.println();


    if (peripheral.localName() == "LED") {
      // stop scanning
      BLE.stopScan();

      ArduinoBLECloud(peripheral);

      // peripheral disconnected, start scanning again
      BLE.scan();
    }
  }
}

void ArduinoBLECloud(BLEDevice peripheral) {
  // connect to the peripheral
  Serial.println("Connecting ...");
  if (peripheral.connect()) {
    Serial.println("Connected");
  } else {
    Serial.println("Failed to connect!");
    return;
  }

  // discover peripheral attributes
  Serial.println("Discovering service 0x1802 ...");
  if (peripheral.discoverService("1802")) {
    Serial.println("Service discovered");
  } else {
    Serial.println("Attribute discovery failed.");
    peripheral.disconnect();

    while (1);
    return;
  }

  // retrieve the characteristic
  BLECharacteristic simpleKeyCharacteristic = peripheral.characteristic("1802");

  // subscribe to the characteristic
  Serial.println("Subscribing to simple key characteristic ...");
  if (!simpleKeyCharacteristic) {
    Serial.println("no simple key characteristic found!");
    peripheral.disconnect();
    return;
  } else if (!simpleKeyCharacteristic.canSubscribe()) {
    Serial.println("simple key characteristic is not subscribable!");
    peripheral.disconnect();
    return;
  } else if (!simpleKeyCharacteristic.subscribe()) {
    Serial.println("subscription failed!");
    peripheral.disconnect();
    return;
  } else {
    Serial.println("Subscribed");
  }



  while (peripheral.connected()) {
    // while the peripheral is connected
     gestureStatus = 0;

     if (APDS.gestureAvailable()) {
        int gesture = APDS.readGesture();
        if (gesture == -1) {
          break;
        }
        if (ledStatus == 0) {
         simpleKeyCharacteristic.writeValue((byte)0x01);
         Serial.println("ON");
         digitalWrite(ledPin2, LOW);
         ledStatus = 1;
         gestureStatus = 1;
        }
        else {
         simpleKeyCharacteristic.writeValue((byte)0x00);
         Serial.println("OFF");
         digitalWrite(ledPin2, HIGH);
         ledStatus = 0;
         gestureStatus = 1;

        }
     }
    // check if the value of the simple key characteristic has been updated
    if (simpleKeyCharacteristic.valueUpdated() && gestureStatus == 0) {
      // yes, get the value, characteristic is 1 byte so use byte value
      byte value = 0;
      
      simpleKeyCharacteristic.readValue(value);

      if (value == 0x00) {
        // first bit corresponds to the right button
        Serial.println("OFF");
        digitalWrite(ledPin2, HIGH);
        ledStatus = 0;
      }

      if (value == 0x01) {
        // second bit corresponds to the left button
        Serial.println("ON");
        digitalWrite(ledPin2, LOW);
        ledStatus = 1;
      }
    }
  }
  Serial.println("Disconnected!");
}
