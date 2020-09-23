# SHA256 in three different ways
You can run SHA256 algortithm in three different ways on Arduino Portenta H7: 
- using [ArduinoBearSSL](https://github.com/arduino-libraries/ArduinoBearSSL), based on [BearlSSL](https://bearssl.org/)
- using [ArduinoECCX08](https://github.com/arduino-libraries/ArduinoECCX08), so the secure element [Microchip ATECC608A](https://www.microchip.com/wwwproducts/en/ATECC608A) can run SHA256
- using [ARM Mbed TLS](https://github.com/ARMmbed/mbedtls)

# Usage

## Requirements
- [Arduino Portenta H7](https://store.arduino.cc/portenta-h7)
- Arduino IDE or [Arduino CLI](https://github.com/arduino/arduino-cli)

1. Download the *ArduinoBearlSSL*'s branch [`luigigubello:adding-runHash-method`](https://github.com/luigigubello/ArduinoBearSSL/tree/adding-runHash-method)
2. Put it in your Arduino libraries folder
3. Upload the sketch to the Arduino Portenta H7 and open the serial monitor
