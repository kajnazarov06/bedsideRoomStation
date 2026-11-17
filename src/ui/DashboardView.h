#pragma once
#include "../core/ITextDisplay.h"
#include "../drivers/RtcClock.h"
#include "../drivers/DhtSensor.h"

// Formats clock + climate data for a 2-line display.
// Knows *what* to show; the display decides *how* to draw it.
class DashboardView {
public:
  DashboardView(ITextDisplay& display, RtcClock& clock, DhtSensor& climate);

  void render();

private:
  void renderTime(uint8_t row);
  void renderClimate(uint8_t row);

  ITextDisplay& _display;
  RtcClock&     _clock;
  DhtSensor&    _climate;
};
