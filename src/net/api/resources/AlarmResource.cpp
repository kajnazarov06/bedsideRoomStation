#include "AlarmResource.h"
#include "DateTimeFormat.h"

void AlarmResource::toJson(uint8_t index, const AlarmSettings& settings, JsonObject json) {
  json["id"]   = index + 1;
  json["time"] = DateTimeFormat::hourMinute(settings.hour, settings.minute);

  JsonArray days = json["days"].to<JsonArray>();
  for (uint8_t day = 1; day <= 7; day++) {
    if (settings.ringsOn(day)) days.add(day);
  }

  json["enabled"] = settings.enabled;
  json["label"]   = String("Alarm ") + (index + 1);
}

const char* AlarmResource::fromJson(JsonObjectConst json, AlarmSettings& settings) {
  AlarmSettings updated = settings;

  if (!json["time"].isNull()) {
    if (!DateTimeFormat::parseHourMinute(json["time"].as<const char*>(), updated.hour, updated.minute)) {
      return "time must be \"HH:MM\" (00:00 - 23:59)";
    }
  }

  if (!json["days"].isNull()) {
    if (!json["days"].is<JsonArrayConst>()) return "days must be an array of 1 (Mon) ... 7 (Sun)";
    uint8_t days = 0;
    for (JsonVariantConst day : json["days"].as<JsonArrayConst>()) {
      if (!day.is<int>() || day.as<int>() < 1 || day.as<int>() > 7) {
        return "days must be an array of 1 (Mon) ... 7 (Sun)";
      }
      days |= AlarmDays::forDay(day.as<int>());
    }
    if (days == 0) return "days must not be empty";
    updated.days = days;
  }

  if (!json["enabled"].isNull()) {
    if (!json["enabled"].is<bool>()) return "enabled must be true or false";
    updated.enabled = json["enabled"].as<bool>();
  }

  settings = updated;
  return nullptr;
}
