#include "RtcClock.h"

RtcClock::RtcClock(uint8_t datPin, uint8_t clkPin, uint8_t rstPin)
  : _wire(datPin, clkPin, rstPin), _rtc(_wire) {}

bool RtcClock::begin() {
  _rtc.Begin();

  const RtcDateTime compiled(__DATE__, __TIME__);

  if (_rtc.GetIsWriteProtected()) _rtc.SetIsWriteProtected(false);
  if (!_rtc.GetIsRunning())       _rtc.SetIsRunning(true);

  // Set the clock from the compile time if it's lost or behind.
  if (!_rtc.IsDateTimeValid() || _rtc.GetDateTime() < compiled) {
    _rtc.SetDateTime(compiled);
  }

  return _rtc.IsDateTimeValid();
}

RtcDateTime RtcClock::now() {
  return _rtc.GetDateTime();
}
