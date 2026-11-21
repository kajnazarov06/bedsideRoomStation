#include "WifiConnection.h"

WifiConnection::WifiConnection(const char* ssid, const char* password, unsigned long timeoutMs)
  : _ssid(ssid), _password(password), _timeoutMs(timeoutMs) {}

bool WifiConnection::begin() {
  WiFi.mode(WIFI_STA);
  WiFi.setAutoReconnect(true);
  WiFi.begin(_ssid, _password);

  Serial.printf("[WiFi] Connecting to \"%s\"", _ssid);
  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED) {
    if (millis() - start >= _timeoutMs) {
      Serial.printf("\n[WiFi] Failed to connect to \"%s\" (status %d)\n", _ssid, WiFi.status());
      return false;
    }
    delay(250);
    Serial.print('.');
  }

  Serial.printf("\n[WiFi] Connected to \"%s\", IP %s, RSSI %d dBm\n",
                _ssid, localIP().toString().c_str(), WiFi.RSSI());
  return true;
}
