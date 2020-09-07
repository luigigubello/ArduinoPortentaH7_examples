# LED sync between Arduino Portenta H7 and Arduino Nano 33 BLE Sense
This example shows how to sync an Arduino Portenta H7 and an Arduino Nano 33 BLE Sense using Arduino IoT Cloud and BLE.

# Usage

## Requirements
- [Arduino Portenta H7](https://store.arduino.cc/portenta-h7)
- [Arduino Nano 33 BLE Sense](https://store.arduino.cc/arduino-nano-33-ble-sense)
- [Arduino IoT Cloud](https://create.arduino.cc)

1. Create a new thing on [Arduino IoT Cloud](https://create.arduino.cc/iot/) by associating the Arduino Portenta H7
<br/><details><img src="/ArduinoIoTCloud_Gesture_Control/img/Screenshot_1.png" width="70%" alt="Arduino IoT Cloud thing example"></img></details>
2. Add a boolean property to the thing, by calling it `ledSwitch`
3. Create a new [dashboard](https://create.arduino.cc/iot/dashboards/) by inserting a switch button linked to the previous boolean property
4. In `arduino_secret.h` add your WiFi credentials and your thing UUID
5. Upload the Arduino Portenta example to Arduino Portenta and the Arduino Nano 33 BLE Sense example to Arduino Nano 33 BLE Sense
6. Download _Arduino IoT Cloud Remote_ app for [Android](https://play.google.com/store/apps/details?id=cc.arduino.cloudiot) or [iOS](https://apps.apple.com/us/app/id1514358431) or use gestures on Arduino BLE Sense for blinking the LEDs on both the boards
