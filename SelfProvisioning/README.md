# Arduino IoT Cloud self-provisioning
Arduino IoT Cloud self-provisioning sketch. Just uploading the sketch to Arduino Portenta and the board will register itself to Arduino IoT Cloud using the Arduino IoT Cloud APIs. 

# Usage

## Requirements
- [Arduino Portenta H7](https://store.arduino.cc/portenta-h7)
- [Arduino IoT APIs credentials](https://create.arduino.cc/iot/)
- Arduino IDE or [Arduino CLI](https://github.com/arduino/arduino-cli)

1. Download the [Arduino Portenta core](https://github.com/arduino/ArduinoCore-mbed) and follow the instractions to install it
2. Upload the example sketch [PortentaWiFiFirmwareUpdater](https://github.com/arduino/ArduinoCore-mbed/tree/master/libraries/WiFi/examples/PortentaWiFiFirmwareUpdater) to Arduino Portenta (if you've ever uploaded this sketch, you can skip this point)
<br/><details><img src="/SelfProvisioning/img/Screenshot_1.png" width="50%" alt="PortentaWiFiFirmwareUpdater in Arduino IDE"></img></details>
3. In `arduino_secrets.h` add your WiFi credentials and your Arduino IoT Cloud APIs credentials
4. Upload the sketch to the board and open the serial monitor