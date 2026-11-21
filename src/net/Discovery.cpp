#include "Discovery.h"
#include <ESPmDNS.h>
#include "DeviceIdentity.h"
#include "../config/Settings.h"

bool Discovery::begin() {
  String id = DeviceIdentity::id();
  if (!MDNS.begin(id.c_str())) return false;

  MDNS.addService("smartclock", "tcp", _port);
  MDNS.addServiceTxt("smartclock", "tcp", "id", id.c_str());
  MDNS.addServiceTxt("smartclock", "tcp", "type", Settings::DEVICE_TYPE);
  MDNS.addServiceTxt("smartclock", "tcp", "fw", Settings::FIRMWARE_VERSION);
  Serial.printf("[mDNS] http://%s.local:%u\n", id.c_str(), _port);
  return true;
}
