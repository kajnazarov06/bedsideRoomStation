#include "AlarmResource.h"
#include "DateTimeFormat.h"

namespace {
  // The firmware has a single alarm that rings every day.
  constexpr int         SINGLE_ALARM_ID = 1;
  constexpr const char* ALARM_LABEL     = "Alarm";
}

void AlarmResource::toJson(const AlarmSettings& settings, JsonObject json) {
  json["id"]   = SINGLE_ALARM_ID;
  json["time"] = DateTimeFormat::hourMinute(settings.hour, settings.minute);

  JsonArray days = json["days"].to<JsonArray>();
  for (int day = 1; day <= 7; day++) days.add(day);

  json["enabled"] = settings.enabled;
  json["label"]   = ALARM_LABEL;
}
