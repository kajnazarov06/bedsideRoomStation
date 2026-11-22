#include "StateController.h"
#include <WiFi.h>
#include "../resources/DateTimeFormat.h"
#include "../../../config/Settings.h"

void StateController::show(HttpRequest&, HttpResponse& response) {
  JsonDocument body;

  const RtcDateTime now = _clock.now();
  if (now.IsValid()) body["time"] = DateTimeFormat::iso(now);
  else               body["time"] = nullptr;
  body["tz"] = Settings::TIME_ZONE;

  const ClimateReading reading = _climate.read();  // the DHT library caches reads for 2 s
  if (reading.valid) {
    body["temperature_c"] = reading.temperatureC;
    body["humidity_pct"]  = reading.humidity;
  } else {
    body["temperature_c"] = nullptr;
    body["humidity_pct"]  = nullptr;
  }

  body["wifi_rssi"] = WiFi.RSSI();
  body["uptime_s"]  = millis() / 1000;

  if (_alarm.settings().enabled && now.IsValid()) body["next_alarm"] = DateTimeFormat::iso(_alarm.nextRing(now));
  else                                            body["next_alarm"] = nullptr;

  response.json(200, body);
}
