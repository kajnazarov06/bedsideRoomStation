#pragma once
#include <WiFi.h>

// Stable per-device ID derived from the WiFi MAC, e.g. "clock-a1b2c3".
// Shared by the HTTP API and mDNS so both advertise the same identity.
namespace DeviceIdentity {
  inline String id() {
    String mac = WiFi.macAddress();  // "AA:BB:CC:DD:EE:FF"
    mac.replace(":", "");
    mac.toLowerCase();
    return "clock-" + mac.substring(6);
  }
}
