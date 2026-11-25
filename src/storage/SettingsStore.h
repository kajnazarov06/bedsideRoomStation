#pragma once
#include <Preferences.h>
#include "../core/IComponent.h"

// Device settings that can be changed at runtime (through the API).
struct DeviceSettings {
  String  name;           // shown in the app, e.g. "Bedroom clock"
  String  timeZone;       // IANA name, e.g. "Africa/Abidjan"; the RTC still holds local wall time
  uint8_t brightness;     // LCD backlight, 0 ... 100 %
  uint8_t snoozeMinutes;  // 1 ... 60
};

// Keeps the device settings in the ESP32 flash (NVS), so they survive power cuts.
// Values missing from flash fall back to the defaults in Settings.h.
class SettingsStore : public IComponent {
public:
  bool begin() override;
  const char* name() const override { return "Settings"; }

  const DeviceSettings& settings() const { return _settings; }

  // Replaces the current settings and saves them.
  void save(const DeviceSettings& settings);

private:
  Preferences    _prefs;
  DeviceSettings _settings;
};
