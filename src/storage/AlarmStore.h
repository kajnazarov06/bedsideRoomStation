#pragma once
#include <Preferences.h>
#include "../core/IComponent.h"

struct AlarmSettings {
  uint8_t hour    = 7;
  uint8_t minute  = 0;
  bool    enabled = false;
};

// Keeps the alarm settings in the ESP32 flash (NVS), so they survive power cuts.
class AlarmStore : public IComponent {
public:
  bool begin() override;
  const char* name() const override { return "Flash"; }

  AlarmSettings load();
  void save(const AlarmSettings& settings);

private:
  Preferences _prefs;
};
