#include "SettingsStore.h"
#include "../config/Settings.h"

namespace {
  constexpr const char* KEY_NAME       = "name";
  constexpr const char* KEY_TIME_ZONE  = "tz";
  constexpr const char* KEY_BRIGHTNESS = "brightness";
  constexpr const char* KEY_SNOOZE     = "snoozeMin";
}

bool SettingsStore::begin() {
  if (!_prefs.begin("settings", false)) return false;

  _settings.name          = _prefs.getString(KEY_NAME, Settings::DEVICE_NAME);
  _settings.timeZone      = _prefs.getString(KEY_TIME_ZONE, Settings::TIME_ZONE);
  _settings.brightness    = min<uint8_t>(_prefs.getUChar(KEY_BRIGHTNESS, Settings::BRIGHTNESS), 100);
  _settings.snoozeMinutes = _prefs.getUChar(KEY_SNOOZE, Settings::SNOOZE_MINUTES);
  return true;
}

void SettingsStore::save(const DeviceSettings& settings) {
  _settings = settings;
  _prefs.putString(KEY_NAME, settings.name);
  _prefs.putString(KEY_TIME_ZONE, settings.timeZone);
  _prefs.putUChar(KEY_BRIGHTNESS, settings.brightness);
  _prefs.putUChar(KEY_SNOOZE, settings.snoozeMinutes);
}
