#pragma once
#include <ThreeWire.h>
#include <RtcDS1302.h>
#include "IComponent.h"

class RtcClock : public IComponent {
public:
  RtcClock(uint8_t datPin, uint8_t clkPin, uint8_t rstPin);

  bool begin() override;
  const char* name() const override { return "RTC"; }

  RtcDateTime now();

private:
  ThreeWire _wire;
  RtcDS1302<ThreeWire> _rtc;
};
