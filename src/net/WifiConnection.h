#pragma once
#include "../core/IComponent.h"
#include <WiFi.h>

// Connects the ESP32 to a WiFi access point in station mode.
// begin() blocks until connected or the timeout expires; afterwards the
// ESP32 WiFi stack reconnects automatically if the link drops.
class WifiConnection : public IComponent {
public:
  WifiConnection(const char* ssid, const char* password, unsigned long timeoutMs = 10000);

  bool begin() override;
  const char* name() const override { return "WiFi"; }

  bool isConnected() const { return WiFi.status() == WL_CONNECTED; }
  IPAddress localIP() const { return WiFi.localIP(); }

private:
  const char*   _ssid;
  const char*   _password;
  unsigned long _timeoutMs;
};
