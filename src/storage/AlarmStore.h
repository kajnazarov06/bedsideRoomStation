#pragma once
#include <array>
#include <Preferences.h>
#include "../core/IComponent.h"
#include "../config/Settings.h"

// Weekday bit masks for AlarmSettings::days, in ISO order:
// bit 0 = Monday ... bit 6 = Sunday.
namespace AlarmDays {
  constexpr uint8_t EVERY_DAY = 0x7F;
  constexpr uint8_t WEEKDAYS  = 0x1F;  // Mon-Fri
  constexpr uint8_t WEEKEND   = 0x60;  // Sat-Sun

  // isoDay: 1 = Monday ... 7 = Sunday
  constexpr uint8_t forDay(uint8_t isoDay) { return 1 << (isoDay - 1); }
}

struct AlarmSettings {
  uint8_t hour    = 7;
  uint8_t minute  = 0;
  uint8_t days    = AlarmDays::EVERY_DAY;
  bool    enabled = false;

  bool ringsOn(uint8_t isoDay) const { return days & AlarmDays::forDay(isoDay); }
};

using AlarmList = std::array<AlarmSettings, Settings::ALARM_COUNT>;

// Keeps the alarms in the ESP32 flash (NVS), so they survive power cuts.
class AlarmStore : public IComponent {
public:
  bool begin() override;
  const char* name() const override { return "Flash"; }

  AlarmList load();
  void save(const AlarmList& alarms);

private:
  Preferences _prefs;
};
