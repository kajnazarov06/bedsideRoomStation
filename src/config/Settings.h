#pragma once
#include <Arduino.h>

// User-tunable behaviour. Wiring lives in Pins.h; internal timings stay
// next to the code that uses them.
namespace Settings {
  // Dashboard
  constexpr unsigned long REFRESH_INTERVAL_MS = 2000;              // DHT11 needs >= 1 s between reads
  constexpr unsigned long MESSAGE_TIME_MS     = 1500;              // boot / error messages

  // Alarm
  constexpr uint8_t       ALARM_COUNT         = 5;                 // alarms kept in flash
  constexpr uint16_t      ALARM_TRACK         = 1;                 // 0001.mp3
  constexpr uint8_t       ALARM_VOLUME        = 20;                // 0..30
  constexpr unsigned long RING_MAX_MS         = 10UL * 60 * 1000;  // auto-stop after 10 min

  // Device settings: defaults only, the live values are edited through
  // PUT /api/settings and saved in flash (SettingsStore)
  constexpr const char*   DEVICE_NAME         = "Bedroom clock";   // shown in the app
  constexpr const char*   TIME_ZONE           = "local";           // RTC holds local wall time
  constexpr uint8_t       BRIGHTNESS          = 100;               // LCD backlight, 0..100 %
  constexpr uint8_t       SNOOZE_MINUTES      = 5;                 // 1..60

  // Network API
  constexpr const char*   DEVICE_TYPE         = "clock";           // API / mDNS device type
  constexpr const char*   FIRMWARE_VERSION    = "0.1.0";
  constexpr uint16_t      API_PORT            = 80;
}
