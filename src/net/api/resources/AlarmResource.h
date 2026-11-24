#pragma once
#include <ArduinoJson.h>
#include "../../../storage/AlarmStore.h"

// JSON shape of one alarm:
//   {"id":1, "time":"07:30", "days":[1,2,3,4,5], "enabled":true, "label":"Alarm 1"}
// id is 1 ... Settings::ALARM_COUNT; days are ISO weekdays: 1 = Monday ... 7 = Sunday.
namespace AlarmResource {
  void toJson(uint8_t index, const AlarmSettings& settings, JsonObject json);

  // Applies the fields present in `json` ("time", "days", "enabled") to `settings`.
  // Returns an error message, or nullptr when the body is valid.
  const char* fromJson(JsonObjectConst json, AlarmSettings& settings);
}
