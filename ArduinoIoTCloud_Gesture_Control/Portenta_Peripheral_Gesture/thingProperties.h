#include <ArduinoIoTCloud.h>
#include <Arduino_ConnectionHandler.h>

const char THING_ID[] = SECRET_THING_ID;

int MIN_NTP_PORT = 49152;
int MAX_NTP_PORT = 65535;

const char SSID[]     = SECRET_SSID;    // Network SSID (name)
const char PASS[]     = SECRET_PASS;    // Network password (use for WPA, or use as key for WEP)

void onLedSwitchChange();

bool ledSwitch;

void initProperties(){

  ArduinoCloud.setThingId(THING_ID);
  ArduinoCloud.addProperty(ledSwitch, READWRITE, ON_CHANGE, onLedSwitchChange);

}

WiFiConnectionHandler ArduinoIoTPreferredConnection(SSID, PASS);
