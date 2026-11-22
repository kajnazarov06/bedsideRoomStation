#pragma once
#include <ArduinoJson.h>
#include "../../../storage/AlarmStore.h"

// JSON shape of one alarm:
//   {"id":1, "time":"07:30", "days":[1,2,3,4,5,6,7], "enabled":true, "label":"Alarm"}
// days are ISO weekdays: 1 = Monday ... 7 = Sunday.
namespace AlarmResource {
  void toJson(const AlarmSettings& settings, JsonObject json);
}
