#include "DashboardView.h"

static constexpr char LCD_DEGREE = (char)0xDF;  // '°' in the HD44780 charset

DashboardView::DashboardView(ITextDisplay& display, RtcClock& clock, DhtSensor& climate)
  : _display(display), _clock(clock), _climate(climate) {}

void DashboardView::render() {
  renderTime(0);
  renderClimate(1);
}

// Row example: "2026-11-12 23:05"
void DashboardView::renderTime(uint8_t row) {
  RtcDateTime dateTime = _clock.now();
  char line[17];
  if (!dateTime.IsValid()) {
    snprintf(line, sizeof(line), "RTC error");
  } else {
    snprintf(line, sizeof(line), "%04u-%02u-%02u %02u:%02u",
             dateTime.Year(), dateTime.Month(), dateTime.Day(), dateTime.Hour(), dateTime.Minute());
  }
  _display.printLine(row, line);
}

// Row example: "T:23.0°C H:45%"
void DashboardView::renderClimate(uint8_t row) {
  ClimateReading reading = _climate.read();
  char line[17];
  if (!reading.valid) {
    snprintf(line, sizeof(line), "DHT11 error");
  } else {
    snprintf(line, sizeof(line), "T:%.1f%cC H:%.0f%%",
             reading.temperatureC, LCD_DEGREE, reading.humidity);
  }
  _display.printLine(row, line);
}
