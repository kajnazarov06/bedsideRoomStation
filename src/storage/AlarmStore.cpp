#include "AlarmStore.h"

bool AlarmStore::begin() {
  return _prefs.begin("alarm", false);
}

AlarmSettings AlarmStore::load() {
  AlarmSettings s;
  s.hour    = _prefs.getUChar("hour", s.hour);
  s.minute  = _prefs.getUChar("minute", s.minute);
  s.enabled = _prefs.getBool("enabled", s.enabled);
  if (s.hour > 23)   s.hour = 0;
  if (s.minute > 59) s.minute = 0;
  return s;
}

void AlarmStore::save(const AlarmSettings& s) {
  _prefs.putUChar("hour", s.hour);
  _prefs.putUChar("minute", s.minute);
  _prefs.putBool("enabled", s.enabled);
}
