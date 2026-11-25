#include "StateController.h"
#include <WiFi.h>
#include "../resources/DateTimeFormat.h"

void StateController::show(HttpRequest&, HttpResponse& response) {
  JsonDocument body;

  const RtcDateTime now = _clock.now();
  if (now.IsValid()) body["time"] = DateTimeFormat::iso(now);
  else               body["time"] = nullptr;
  body["tz"] = _settings.settings().timeZone;

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

  RtcDateTime nextAlarm;
  if (now.IsValid() && _alarm.nextRing(now, nextAlarm)) body["next_alarm"] = DateTimeFormat::iso(nextAlarm);
  else                                                  body["next_alarm"] = nullptr;

  response.json(200, body);
}
