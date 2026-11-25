#pragma once
#include <ArduinoJson.h>
#include "../../../storage/SettingsStore.h"

// JSON shape of the device settings:
//   {"name":"Bedroom clock", "tz":"Africa/Abidjan", "brightness":50, "snooze_min":5}
namespace SettingsResource {
  void toJson(const DeviceSettings& settings, JsonObject json);

  // Applies the fields present in `json` to `settings`.
  // Returns an error message, or nullptr when the body is valid.
  const char* fromJson(JsonObjectConst json, DeviceSettings& settings);
}
