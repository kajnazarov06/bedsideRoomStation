#pragma once
#include "../../../drivers/RtcClock.h"
#include <WString.h>

namespace DateTimeFormat {
  // "2026-11-22T07:30:00". RTC time is local wall time, so no "Z" suffix.
  inline String iso(const RtcDateTime& dateTime) {
    char text[20];
    snprintf(text, sizeof(text), "%04u-%02u-%02uT%02u:%02u:%02u",
             dateTime.Year(), dateTime.Month(), dateTime.Day(),
             dateTime.Hour(), dateTime.Minute(), dateTime.Second());
    return String(text);
  }

  // "07:30"
  inline String hourMinute(uint8_t hour, uint8_t minute) {
    char text[6];
    snprintf(text, sizeof(text), "%02u:%02u", hour, minute);
    return String(text);
  }
}
