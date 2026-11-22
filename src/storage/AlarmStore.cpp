#include "AlarmStore.h"

bool AlarmStore::begin() {
  return _prefs.begin("alarm", false);
}

AlarmSettings AlarmStore::load() {
  AlarmSettings settings;
  settings.hour    = _prefs.getUChar("hour", settings.hour);
  settings.minute  = _prefs.getUChar("minute", settings.minute);
  settings.enabled = _prefs.getBool("enabled", settings.enabled);
  if (settings.hour > 23)   settings.hour = 0;
  if (settings.minute > 59) settings.minute = 0;
  return settings;
}

void AlarmStore::save(const AlarmSettings& settings) {
  _prefs.putUChar("hour", settings.hour);
  _prefs.putUChar("minute", settings.minute);
  _prefs.putBool("enabled", settings.enabled);
}
