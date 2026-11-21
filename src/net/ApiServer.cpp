#include "ApiServer.h"
#include <WiFi.h>
#include "DeviceIdentity.h"
#include "../config/Settings.h"

namespace {
  constexpr int API_VERSION = 1;

  // RTC time is local wall time, so no "Z" suffix.
  void formatIso(const RtcDateTime& t, char* out, size_t len) {
    snprintf(out, len, "%04u-%02u-%02uT%02u:%02u:%02u",
             t.Year(), t.Month(), t.Day(), t.Hour(), t.Minute(), t.Second());
  }

  // Next time the alarm will ring: today if still ahead, otherwise tomorrow.
  RtcDateTime nextAlarm(const RtcDateTime& now, const AlarmSettings& a) {
    RtcDateTime today(now.Year(), now.Month(), now.Day(), a.hour, a.minute, 0);
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
  JsonDocument d;
  d["error"] = "locked";
  d["retry_after"] = _auth.retryAfterS();
  reply(429, d);
}

bool ApiServer::requireAuth() {
  const String header = _server.header("Authorization");
  const char* prefix = "Bearer ";
  if (header.startsWith(prefix) && _auth.verifyToken(header.substring(strlen(prefix)))) return true;

  JsonDocument d;
  d["error"] = "unauthorized";
  reply(401, d);
  return false;
}

// ---------------------------------------------------------------- routes

void ApiServer::handleInfo() {
  JsonDocument d;
  d["id"]     = _deviceId;
  d["type"]   = Settings::DEVICE_TYPE;
  d["name"]   = Settings::DEVICE_NAME;
  d["fw"]     = Settings::FIRMWARE_VERSION;
  d["mac"]    = WiFi.macAddress();
  d["paired"] = _auth.isPaired();
  JsonArray caps = d["capabilities"].to<JsonArray>();
  caps.add("clock");
  caps.add("temperature");
  caps.add("humidity");
  caps.add("alarm");
  reply(200, d);
}

void ApiServer::handlePairStart() {
  if (!_auth.startPairing()) return replyLocked();

  JsonDocument d;
  d["expires_in"] = 120;
  reply(200, d);
}

void ApiServer::handlePairConfirm() {
  JsonDocument in;
  deserializeJson(in, _server.arg("plain"));  // bad JSON -> empty code -> wrong code
  const char* code = in["code"] | "";

  String token;
  switch (_auth.confirmPairing(code, token)) {
    case DeviceAuth::PairResult::Ok: {
      JsonDocument d;
      d["token"] = token;
      return reply(200, d);
    }
    case DeviceAuth::PairResult::WrongCode: {
      JsonDocument d;
      d["error"] = "wrong code";
      d["attempts_left"] = _auth.attemptsLeft();
      return reply(401, d);
    }
    case DeviceAuth::PairResult::LockedOut:
      return replyLocked();
  }
}

void ApiServer::handleUnpair() {
  if (!requireAuth()) return;
  _auth.unpair();

  JsonDocument d;
  d["ok"] = true;
  reply(200, d);
}

void ApiServer::handleState() {
  if (!requireAuth()) return;

  JsonDocument d;
  RtcDateTime now = _clock.now();
  char ts[20];
  if (now.IsValid()) {
    formatIso(now, ts, sizeof(ts));
    d["time"] = ts;
  } else {
    d["time"] = nullptr;
  }
  d["tz"] = Settings::TIME_ZONE;

  ClimateReading r = _climate.read();  // the DHT library caches reads for 2 s
  if (r.valid) {
    d["temperature_c"] = r.temperatureC;
    d["humidity_pct"]  = r.humidity;
  } else {
    d["temperature_c"] = nullptr;
    d["humidity_pct"]  = nullptr;
  }

  d["wifi_rssi"] = WiFi.RSSI();
  d["uptime_s"]  = millis() / 1000;

  const AlarmSettings& alarm = _alarm.settings();
  if (alarm.enabled && now.IsValid()) {
    char next[20];
    formatIso(nextAlarm(now, alarm), next, sizeof(next));
    d["next_alarm"] = next;
  } else {
    d["next_alarm"] = nullptr;
  }

  reply(200, d);
}

void ApiServer::handleNotFound() {
  JsonDocument d;
  d["error"] = "not found";
  reply(404, d);
}
