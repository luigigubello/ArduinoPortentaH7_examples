#include "arduino_secrets.h"
#include "ECCX08TLSConfig.h"
#include <ArduinoECCX08.h>
#include <Arduino_JSON.h>
#include <ArduinoIoTCloud.h>
#include <WiFiSSLClient.h>

// pag 3496 manuale
#define SERIAL_NUMBER_WORD_0  *(unsigned long*)(0x1FF1E800)
#define SERIAL_NUMBER_WORD_1  *(unsigned long*)(0x1FF1E804)
#define SERIAL_NUMBER_WORD_2  *(unsigned long*)(0x1FF1E808)

int MIN_NTP_PORT = 49152;
int MAX_NTP_PORT = 65535;

const bool DEBUG = true;
const int keySlot                                   = 0;
const int compressedCertSlot                        = 10;
const int serialNumberAndAuthorityKeyIdentifierSlot = 11;
const int deviceIdSlot                              = 12;

char ssid[] = SECRET_SSID;
char pass[] = SECRET_PASS;
char client_id[] = SECRET_CLIENT_ID;
char secret_id[] = SECRET_SECRET_ID;

/// Board Info
String board_name = "NewDevice_";
char board_type[] = "envie_m7"; // Portenta H7 M7
char board_fqbn[] = "arduino-beta:mbed:envie_m7"; // Portenta H7 M7
String ArduinoID;

String Arduino_Token;
String deviceId;

String not_before;
String serialNumber;
String authorityKeyIdentifier;
String signature;

char server[] = "api2.arduino.cc";  // server address
int port = 443;

//WiFiSSLClient client;
WiFiSSLClient client;
int status = WL_IDLE_STATUS;

ECCX08CertClass ECCX08Cert;

void setup() {
  Serial.begin(9600);
  while (!Serial);

  while ( status != WL_CONNECTED) {
    Serial.print("Attempting to connect to Network named: ");
    Serial.println(ssid);

    // Connect to WPA/WPA2 network:
    status = WiFi.begin(ssid, pass);
    delay(10000);
  }

  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  while (!ECCX08.begin()) {
    Serial.println("No ECCX08 present!");
    delay(100);
  }

  if (!ECCX08.locked()) {

    if (!ECCX08.writeConfiguration(DEFAULT_ECCX08_TLS_CONFIG)) {
      Serial.println("Writing ECCX08 configuration failed!");
      Serial.println("Stopping Provisioning");
      while (1);
    }

    if (!ECCX08.lock()) {
      Serial.println("Locking ECCX08 configuration failed!");
      Serial.println("Stopping Provisioning");
      while (1);
    }

    Serial.println("ECCX08 locked successfully");
    Serial.println();
  }

  //Random number for device name
  board_name += String(ECCX08.random(65535));

  uint32_t BoardUniqueID[3];
  BoardUniqueID[0] = SERIAL_NUMBER_WORD_0;
  BoardUniqueID[1] = SERIAL_NUMBER_WORD_1;
  BoardUniqueID[2] = SERIAL_NUMBER_WORD_2;
  uint8_t bid[32];
  for (int i = 0; i < 3; i++)
  {
    bid[i*4+0] = (uint8_t)(BoardUniqueID[i] >> 24);
    bid[i*4+1] = (uint8_t)(BoardUniqueID[i] >> 16);
    bid[i*4+2] = (uint8_t)(BoardUniqueID[i] >> 8);
    bid[i*4+3] = (uint8_t)(BoardUniqueID[i] >> 0);
  }

  for (size_t i = 0; i < 12; i++) {
    if (bid[i] < 12) {
      ArduinoID += String(0, HEX);
    }
    ArduinoID += String(bid[i], HEX);
  }
  ArduinoID.toUpperCase();

  Serial.print("Device Name: ");
  Serial.println(board_name);
  // Create Arduino Token
  ArduinoToken(client_id, secret_id);
  Serial.print("Bearer Token: ");
  Serial.println(Arduino_Token);
  // Generating Device UUID
  BoardUuid(board_name, board_type, board_fqbn, ArduinoID, Arduino_Token);
  Serial.print("Device UUID:");
  Serial.println(deviceId);
  
  delay(2000);

  while (!ECCX08Cert.beginCSR(keySlot, true)) {
    Serial.println("Error starting CSR generation!");
    delay(2000);
  }

  ECCX08Cert.setSubjectCommonName(deviceId);

  String csr = ECCX08Cert.endCSR();

  while (!csr) {
    Serial.println("Error generating CSR!");
    delay(2000);
  }
  
  Serial.println("Generated CSR is:");
  Serial.println();
  Serial.println(csr);

  // Downloading Arduino cert
  ArduinoCertificate(Arduino_Token, deviceId, csr);
  
  String issueYear              = not_before.substring(0,4);
  Serial.print("Year: ");
  Serial.println(issueYear);
  String issueMonth             = not_before.substring(5,7);
  Serial.print("Month: ");
  Serial.println(issueMonth);
  String issueDay               = not_before.substring(8,10);
  Serial.print("Day: ");
  Serial.println(issueDay);
  String issueHour              = not_before.substring(11,13);
  Serial.print("Hour: ");
  Serial.println(issueHour);
  String expireYears            = "31";
  Serial.print("Certificate Serial: ");
  Serial.println(serialNumber);
  Serial.print("Certificate Authority Key: ");
  Serial.println(authorityKeyIdentifier);
  Serial.print("Certificate Signature: ");
  Serial.println(signature);

  byte deviceIdBytes[72];
  byte serialNumberBytes[16];
  byte authorityKeyIdentifierBytes[20];
  byte signatureBytes[64];

  deviceId.getBytes(deviceIdBytes, sizeof(deviceIdBytes));
  hexStringToBytes(serialNumber, serialNumberBytes, sizeof(serialNumberBytes));
  hexStringToBytes(authorityKeyIdentifier, authorityKeyIdentifierBytes, sizeof(authorityKeyIdentifierBytes));
  hexStringToBytes(signature, signatureBytes, sizeof(signatureBytes));

  if (!ECCX08.writeSlot(deviceIdSlot, deviceIdBytes, sizeof(deviceIdBytes))) {
    Serial.println("Error storing device id!");
    while (1);
  }

  if (!ECCX08Cert.beginStorage(compressedCertSlot, serialNumberAndAuthorityKeyIdentifierSlot)) {
    Serial.println("Error starting ECCX08 storage!");
    while (1);
  }

  ECCX08Cert.setSignature(signatureBytes);
  ECCX08Cert.setAuthorityKeyIdentifier(authorityKeyIdentifierBytes);
  ECCX08Cert.setSerialNumber(serialNumberBytes);
  ECCX08Cert.setIssueYear(issueYear.toInt());
  ECCX08Cert.setIssueMonth(issueMonth.toInt());
  ECCX08Cert.setIssueDay(issueDay.toInt());
  ECCX08Cert.setIssueHour(issueHour.toInt());
  ECCX08Cert.setExpireYears(expireYears.toInt());

  if (!ECCX08Cert.endStorage()) {
    Serial.println("Error storing ECCX08 compressed cert!");
    while (1);
  }

  if (!ECCX08Cert.beginReconstruction(keySlot, compressedCertSlot, serialNumberAndAuthorityKeyIdentifierSlot)) {
    Serial.println("Error starting ECCX08 cert reconstruction!");
    while (1);
  }

  ECCX08Cert.setIssuerCountryName("US");
  ECCX08Cert.setIssuerOrganizationName("Arduino LLC US");
  ECCX08Cert.setIssuerOrganizationalUnitName("IT");
  ECCX08Cert.setIssuerCommonName("Arduino");

  if (!ECCX08Cert.endReconstruction()) {
    Serial.println("Error reconstructing ECCX08 compressed cert!");
    while (1);
  }

  if (!DEBUG) {
    return;
  }

  Serial.println("Compressed cert = ");

  const byte* certData = ECCX08Cert.bytes();
  int certLength = ECCX08Cert.length();

  for (int i = 0; i < certLength; i++) {
    byte b = certData[i];

    if (b < 16) {
      Serial.print('0');
    }
    Serial.print(b, HEX);
  }
  Serial.println();
}

void loop() {
}

String promptAndReadLine(const char* prompt) {
  Serial.print(prompt);
  String s = readLine();
  Serial.println(s);

  return s;
}

String readLine() {
  String line;

  while (1) {
    if (Serial.available()) {
      char c = Serial.read();

      if (c == '\r') {
        // ignore
      } else if (c == '\n') {
        break;
      }

      line += c;
    }
  }

  line.trim();

  return line;
}

void hexStringToBytes(String& in, byte out[], int length) {
  int inLength = in.length();
  in.toUpperCase();
  int outLength = 0;

  for (int i = 0; i < inLength && outLength < length; i += 2) {
    char highChar = in[i];
    char lowChar = in[i + 1];

    byte highByte = (highChar <= '9') ? (highChar - '0') : (highChar + 10 - 'A');
    byte lowByte = (lowChar <= '9') ? (lowChar - '0') : (lowChar + 10 - 'A');

    out[outLength++] = (highByte << 4) | (lowByte & 0xF);
  }
}

void ArduinoToken(String client_id, String client_secret) {
  Serial.println("Creating Bearer Token...");
  String PostData = "grant_type=client_credentials&client_id=";
  PostData += client_id;
  PostData += "&client_secret=";
  PostData += secret_id;
  PostData += "&audience=https://api2.arduino.cc/iot";
  
  if (client.connect(server, port)) {
    client.println("POST /iot/v1/clients/token HTTP/1.1");
    client.println("Host: api2.arduino.cc");
    client.println("Connection: close");
    client.println("Content-Type: application/x-www-form-urlencoded");
    client.print("Content-Length: ");
    client.println(PostData.length());
    client.println();
    client.println(PostData);
  }

  while (!client.available()) {
    Serial.println("No client");
    delay(2000);
  }

  char endOfHeaders[] = "\r\n\r\n";
  if (!client.find(endOfHeaders)) {
    Serial.println("Invalid response");
    Serial.println("Error during token generation");
    while (1) {
      ;
    }
  }

  char tokenResponse[1024];
  int intIndex = 0;
  while (client.available()) {
    tokenResponse[intIndex] = client.read();
    if (tokenResponse[intIndex] == -1) {
      break;
    }
    intIndex++;
  }
  JSONVar myObject = JSON.parse(tokenResponse);
  if (myObject.hasOwnProperty("access_token")) {
    Arduino_Token += (const char*) myObject["access_token"];
  }
  client.stop();
}

void BoardUuid(String board_name, String board_type, String board_fqbn, String board_serial, String user_token) {
  Serial.println("Adding board to IoT Cloud...");
  String PostData = "{\"fqbn\":\"";
  PostData += board_fqbn;
  PostData += "\",\"name\":\"";
  PostData += board_name;
  PostData += "\",\"type\":\"";
  PostData += board_type;
  PostData += "\",\"serial\":\"";
  PostData += board_serial;
  PostData += "\"}";
  
  if (client.connect(server, port)) {
    client.println("PUT /iot/v2/devices HTTP/1.1");
    client.println("Host: api2.arduino.cc");
    client.println("Connection: close");
    client.println("Content-Type: application/json;charset=UTF-8");
    client.print("Authorization: Bearer ");
    client.println(user_token);
    client.print("Content-Length: ");
    client.println(PostData.length());
    client.println();
    client.println(PostData);
  }

  while (!client.available()) {
    Serial.println("No client");
    delay(2000);
  }

  char endOfHeaders[] = "\r\n\r\n";
  if (!client.find(endOfHeaders)) {
    Serial.println("Invalid response");
    Serial.println("Error during Device UUID generation");
    while (1) {
      ;
    }
  }
  
  char deviceResponse[512];
  int intIndex = 0;
  while (client.available()) {
    deviceResponse[intIndex] = client.read();
    if (deviceResponse[intIndex] == -1) {
      break;
    }
    intIndex++;
  }
  JSONVar myObject = JSON.parse(deviceResponse);
  if (myObject.hasOwnProperty("id")) {
    deviceId += (const char*) myObject["id"];
  }
  client.stop();
}

void ArduinoCertificate(String user_token, String DeviceUuid, String csr) {
  Serial.println("Downloading certificate...");
  String url = "/iot/v2/devices/";
  url += DeviceUuid;
  url += "/certs";

  csr.replace("\n", "\\n");
  String PostData = "{\"ca\":\"Arduino\",\"csr\":\"";
  PostData += csr;
  PostData += "\",\"enabled\":true}";

  if (client.connect(server, port)) {
    client.print("PUT ");
    client.print(url);
    client.println(" HTTP/1.1");
    client.println("Host: api2.arduino.cc");
    client.println("Connection: close");
    client.println("Content-Type: application/json;charset=UTF-8");
    client.print("Authorization: Bearer ");
    client.println(user_token);
    client.print("Content-Length: ");
    client.println(PostData.length());
    client.println();
    client.println(PostData);
  }

  while (!client.available()) {
    Serial.println("No client");
    delay(2000);
  }
//  while (client.available()) {
//    char c = client.read();
//    Serial.write(c);
//  }
  char endOfHeaders[] = "\r\n\r\n";
  if (!client.find(endOfHeaders)) {
    Serial.println("Invalid response");
    Serial.println("Error during certificate generation");
    while (1) {
      ;
    }
  }
  
  char certResponse[4096];
  int intIndex = 0;
  while (client.available()) {
    certResponse[intIndex] = client.read();
    if (certResponse[intIndex] == -1) {
      break;
    }
    intIndex++;
  }
  JSONVar myObject = JSON.parse(certResponse);
  String certZip = JSON.stringify(myObject["compressed"]);
  JSONVar myCert = JSON.parse(certZip);
  if (myCert.hasOwnProperty("not_before")) {
    not_before += (const char*) myCert["not_before"];
  }
  if (myCert.hasOwnProperty("serial")) {
    serialNumber += (const char*) myCert["serial"];
  }
  if (myCert.hasOwnProperty("authority_key_identifier")) {
    authorityKeyIdentifier += (const char*) myCert["authority_key_identifier"];
  }
  if (myCert.hasOwnProperty("signature_asn1_x")) {
    signature += (const char*) myCert["signature_asn1_x"];
  }
  if (myCert.hasOwnProperty("signature_asn1_y")) {
    signature += (const char*) myCert["signature_asn1_y"];
  }
  client.stop();
}
