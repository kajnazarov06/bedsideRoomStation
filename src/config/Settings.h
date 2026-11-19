#pragma once
#include <Arduino.h>

// User-tunable behaviour. Wiring lives in Pins.h; internal timings stay
// next to the code that uses them.
namespace Settings {
  // Dashboard
  constexpr unsigned long REFRESH_INTERVAL_MS = 2000;              // DHT11 needs >= 1 s between reads
  constexpr unsigned long MESSAGE_TIME_MS     = 1500;              // boot / error messages

  // Alarm
  constexpr uint16_t      ALARM_TRACK         = 1;                 // 0001.mp3
  constexpr uint8_t       ALARM_VOLUME        = 20;                // 0..30
  constexpr unsigned long RING_MAX_MS         = 10UL * 60 * 1000;  // auto-stop after 10 min
}
