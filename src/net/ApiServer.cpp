#include "ApiServer.h"
#include <WiFi.h>
#include "DeviceIdentity.h"
#include "../config/Settings.h"

namespace {
  constexpr int API_VERSION = 1;

  // RTC time is local wall time, so no "Z" suffix.
  void formatIso(const RtcDateTime& dateTime, char* out, size_t len) {
    snprintf(out, len, "%04u-%02u-%02uT%02u:%02u:%02u",
             dateTime.Year(), dateTime.Month(), dateTime.Day(), dateTime.Hour(), dateTime.Minute(), dateTime.Second());
  }

  // Next time the alarm will ring: today if still ahead, otherwise tomorrow.
  RtcDateTime nextAlarm(const RtcDateTime& now, const AlarmSettings& settings) {
    RtcDateTime today(now.Year(), now.Month(), now.Day(), settings.hour, settings.minute, 0);
    return today > now ? today : today + (uint32_t)(24UL * 60 * 60);
  }
}

ApiServer::ApiServer(DeviceAuth& auth, RtcClock& clock, DhtSensor& climate,
                     const AlarmController& alarm, uint16_t port)
  : _server(port), _auth(auth), _clock(clock), _climate(climate), _alarm(alarm) {}

bool ApiServer::begin() {
  _deviceId = DeviceIdentity::id();

  const char* headers[] = { "Authorization" };
  _server.collectHeaders(headers, 1);

  _server.on("/api/info",         HTTP_GET,  [this] { handleInfo(); });
  _server.on("/api/pair/start",   HTTP_POST, [this] { handlePairStart(); });
  _server.on("/api/pair/confirm", HTTP_POST, [this] { handlePairConfirm(); });
  _server.on("/api/unpair",       HTTP_POST, [this] { handleUnpair(); });
  _server.on("/api/state",        HTTP_GET,  [this] { handleState(); });
  _server.on("/api/alarms",       HTTP_GET,  [this] { handleAlarms(); });
  _server.onNotFound([this] { handleNotFound(); });

  _server.begin();
  return true;
}

void ApiServer::poll() {
  _server.handleClient();
}

// ---------------------------------------------------------------- helpers

void ApiServer::reply(int code, JsonDocument& doc) {
  doc["api_version"] = API_VERSION;
  String body;
  serializeJson(doc, body);
  _server.send(code, "application/json", body);
  Serial.printf("[API] %s %s -> %d\n",
                _server.method() == HTTP_GET ? "GET" : "POST", _server.uri().c_str(), code);
}

void ApiServer::replyLocked() {
  JsonDocument response;
  response["error"] = "locked";
  response["retry_after"] = _auth.retryAfterS();
  reply(429, response);
}

bool ApiServer::requireAuth() {
  const String header = _server.header("Authorization");
  const char* prefix = "Bearer ";
  if (header.startsWith(prefix) && _auth.verifyToken(header.substring(strlen(prefix)))) return true;

  JsonDocument response;
  response["error"] = "unauthorized";
  reply(401, response);
  return false;
}

// ---------------------------------------------------------------- routes

void ApiServer::handleInfo() {
  JsonDocument response;
  response["id"]     = _deviceId;
  response["type"]   = Settings::DEVICE_TYPE;
  response["name"]   = Settings::DEVICE_NAME;
  response["fw"]     = Settings::FIRMWARE_VERSION;
  response["mac"]    = WiFi.macAddress();
  response["paired"] = _auth.isPaired();
  JsonArray capabilities = response["capabilities"].to<JsonArray>();
  capabilities.add("clock");
  capabilities.add("temperature");
  capabilities.add("humidity");
  capabilities.add("alarm");
  reply(200, response);
}

void ApiServer::handlePairStart() {
  if (!_auth.startPairing()) return replyLocked();

  JsonDocument response;
  response["expires_in"] = 120;
  reply(200, response);
}

void ApiServer::handlePairConfirm() {
  JsonDocument request;
  deserializeJson(request, _server.arg("plain"));  // bad JSON -> empty code -> wrong code
  const char* code = request["code"] | "";

  String token;
  switch (_auth.confirmPairing(code, token)) {
    case DeviceAuth::PairResult::Ok: {
      JsonDocument response;
      response["token"] = token;
      return reply(200, response);
    }
    case DeviceAuth::PairResult::WrongCode: {
      JsonDocument response;
      response["error"] = "wrong code";
      response["attempts_left"] = _auth.attemptsLeft();
      return reply(401, response);
    }
    case DeviceAuth::PairResult::LockedOut:
      return replyLocked();
  }
}

void ApiServer::handleUnpair() {
  if (!requireAuth()) return;
  _auth.unpair();

  JsonDocument response;
  response["ok"] = true;
  reply(200, response);
}

void ApiServer::handleState() {
  if (!requireAuth()) return;

  JsonDocument response;
  RtcDateTime now = _clock.now();
  char timestamp[20];
  if (now.IsValid()) {
    formatIso(now, timestamp, sizeof(timestamp));
    response["time"] = timestamp;
  } else {
    response["time"] = nullptr;
  }
  response["tz"] = Settings::TIME_ZONE;

  ClimateReading reading = _climate.read();  // the DHT library caches reads for 2 s
  if (reading.valid) {
    response["temperature_c"] = reading.temperatureC;
    response["humidity_pct"]  = reading.humidity;
  } else {
    response["temperature_c"] = nullptr;
    response["humidity_pct"]  = nullptr;
  }

  response["wifi_rssi"] = WiFi.RSSI();
  response["uptime_s"]  = millis() / 1000;

  const AlarmSettings& alarm = _alarm.settings();
  if (alarm.enabled && now.IsValid()) {
    char next[20];
    formatIso(nextAlarm(now, alarm), next, sizeof(next));
    response["next_alarm"] = next;
  } else {
    response["next_alarm"] = nullptr;
  }

  reply(200, response);
}

// The firmware has a single alarm that rings every day, so the list has one entry.
void ApiServer::handleAlarms() {
  if (!requireAuth()) return;

  const AlarmSettings& settings = _alarm.settings();
  char timeText[6];
  snprintf(timeText, sizeof(timeText), "%02u:%02u", settings.hour, settings.minute);

  JsonDocument response;
  JsonObject alarm = response["alarms"].to<JsonArray>().add<JsonObject>();
  alarm["id"]      = 1;
  alarm["time"]    = timeText;
  JsonArray days   = alarm["days"].to<JsonArray>();
  for (int day = 1; day <= 7; day++) days.add(day);
  alarm["enabled"] = settings.enabled;
  alarm["label"]   = "Alarm";

  reply(200, response);
}

void ApiServer::handleNotFound() {
  JsonDocument response;
  response["error"] = "not found";
  reply(404, response);
}
