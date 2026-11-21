#pragma once

// Copy this file to Secrets.h and fill in your network details.
// Secrets.h is gitignored so credentials never get committed.
namespace Secrets {
  // Existing network the ESP32 joins (use "" as the password for an open network).
  constexpr const char* WIFI_SSID     = "your-network-name";
  constexpr const char* WIFI_PASSWORD = "your-network-password";
}
