#include <ArduinoBearSSL.h>
#include <ArduinoECCX08.h>
#include "mbedtls/sha256.h"

void setup() {
  Serial.begin(9600);
  while (!Serial);
  
  if (!ECCX08.begin()) {
    Serial.println("Waiting ATECC608...");
    delay(100);
  }

  uint8_t input[8] = "Arduino"; // {0x41,0x72,0x64,0x75,0x69,0x6E,0x6F,0x00}
  
  // BearSSL
  uint8_t output_brssl[32];
  SHA256.runHash(input, 8, output_brssl); // expect 0x9838B14C64DF2C9036A664C2CBE5BB5398A6996DE0CB3E1C8300F4371AFBB6E2
  Serial.print("Output SHA256 BearSSL: ");
  printHex(output_brssl, 32);
  Serial.println(" ");
  delay(100);
  
  // Microchip ATECCX08
  byte output_ateccx08[32];
  ECCX08.beginSHA256();
  ECCX08.endSHA256(input, 8, output_ateccx08);
  Serial.print("Output SHA256 ATECC608A: ");
  printHex(output_ateccx08, 32);
  Serial.println(" ");
  delay(100);

  // Mbed TLS
  byte output_mbedtls[32];
  mbedtls_sha256_context ctx;
  mbedtls_sha256_starts(&ctx, 0);
  mbedtls_sha256_update_ret(&ctx, input, 8);
  mbedtls_sha256_finish(&ctx, output_mbedtls);
  Serial.print("Output SHA256 Mbed TLS: ");
  printHex(output_mbedtls, 32);
  Serial.println(" ");
  delay(100);

}

void loop() {
}

void printHex(uint8_t *text, size_t size) {
  for (byte i = 0; i < size; i = i + 1) {
    if (text[i] < 16) {
      Serial.print("0");
    }
    Serial.print(text[i], HEX);
  }
}
