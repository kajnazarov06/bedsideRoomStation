#include "AlarmStore.h"

namespace {
  // All alarms are stored as one blob: 4 bytes per alarm (hour, minute, days, enabled).
  constexpr const char* LIST_KEY         = "list";
  constexpr size_t      BYTES_PER_ALARM  = 4;
  constexpr size_t      MAX_STORED_BYTES = 32 * BYTES_PER_ALARM;  // copes with ALARM_COUNT changes

  AlarmSettings sanitized(AlarmSettings settings) {
    if (settings.hour > 23)   settings.hour = 0;
    if (settings.minute > 59) settings.minute = 0;
    settings.days &= AlarmDays::EVERY_DAY;
    if (settings.days == 0)   settings.days = AlarmDays::EVERY_DAY;
    return settings;
  }
}

bool AlarmStore::begin() {
  return _prefs.begin("alarm", false);
}

AlarmList AlarmStore::load() {
  AlarmList alarms;  // defaults for anything not stored

  if (_prefs.isKey(LIST_KEY)) {
    uint8_t bytes[MAX_STORED_BYTES];
    const size_t length = _prefs.getBytesLength(LIST_KEY);
    if (length <= sizeof(bytes) && _prefs.getBytes(LIST_KEY, bytes, sizeof(bytes)) == length) {
      for (size_t i = 0; i < alarms.size() && (i + 1) * BYTES_PER_ALARM <= length; i++) {
        const uint8_t* stored = bytes + i * BYTES_PER_ALARM;
        AlarmSettings settings;
        settings.hour    = stored[0];
        settings.minute  = stored[1];
        settings.days    = stored[2];
        settings.enabled = stored[3] != 0;
        alarms[i] = sanitized(settings);
      }
    }
  }

  return alarms;
}

void AlarmStore::save(const AlarmList& alarms) {
  uint8_t bytes[Settings::ALARM_COUNT * BYTES_PER_ALARM];
  for (size_t i = 0; i < alarms.size(); i++) {
    uint8_t* stored = bytes + i * BYTES_PER_ALARM;
    stored[0] = alarms[i].hour;
    stored[1] = alarms[i].minute;
    stored[2] = alarms[i].days;
    stored[3] = alarms[i].enabled ? 1 : 0;
  }
  _prefs.putBytes(LIST_KEY, bytes, sizeof(bytes));
}
