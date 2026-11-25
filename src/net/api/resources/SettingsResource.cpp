#include "SettingsResource.h"

namespace {
  constexpr size_t MAX_NAME_LENGTH      = 32;
  constexpr size_t MAX_TIME_ZONE_LENGTH = 48;  // longest IANA names are ~30 chars
  constexpr int    MIN_SNOOZE_MINUTES   = 1;
  constexpr int    MAX_SNOOZE_MINUTES   = 60;

  bool isText(JsonVariantConst value, size_t maxLength) {
    if (!value.is<const char*>()) return false;
    const size_t length = strlen(value.as<const char*>());
    return length > 0 && length <= maxLength;
  }

  bool isIntegerBetween(JsonVariantConst value, int minimum, int maximum) {
    return value.is<int>() && value.as<int>() >= minimum && value.as<int>() <= maximum;
  }
}

void SettingsResource::toJson(const DeviceSettings& settings, JsonObject json) {
  json["name"]       = settings.name;
  json["tz"]         = settings.timeZone;
  json["brightness"] = settings.brightness;
  json["snooze_min"] = settings.snoozeMinutes;
}

const char* SettingsResource::fromJson(JsonObjectConst json, DeviceSettings& settings) {
  DeviceSettings updated = settings;

  if (!json["name"].isNull()) {
    if (!isText(json["name"], MAX_NAME_LENGTH)) return "name must be a string of 1 - 32 characters";
    updated.name = json["name"].as<const char*>();
  }

  if (!json["tz"].isNull()) {
    if (!isText(json["tz"], MAX_TIME_ZONE_LENGTH)) return "tz must be a time zone name like \"Africa/Abidjan\"";
    updated.timeZone = json["tz"].as<const char*>();
  }

  if (!json["brightness"].isNull()) {
    if (!isIntegerBetween(json["brightness"], 0, 100)) return "brightness must be an integer 0 - 100";
    updated.brightness = json["brightness"].as<int>();
  }

  if (!json["snooze_min"].isNull()) {
    if (!isIntegerBetween(json["snooze_min"], MIN_SNOOZE_MINUTES, MAX_SNOOZE_MINUTES)) {
      return "snooze_min must be an integer 1 - 60";
    }
    updated.snoozeMinutes = json["snooze_min"].as<int>();
  }

  settings = updated;
  return nullptr;
}
