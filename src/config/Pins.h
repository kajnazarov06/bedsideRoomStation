#pragma once
#include <Arduino.h>

// Single source of truth for ESP32 wiring.
namespace Pins {
  // I2C LCD
  constexpr uint8_t LCD_SDA       = 21;
  constexpr uint8_t LCD_SCL       = 22;

  // DHT11
  constexpr uint8_t DHT_DATA      = 4;

  // DS1302 RTC
  constexpr uint8_t RTC_DAT       = 25;
  constexpr uint8_t RTC_CLK       = 26;
  constexpr uint8_t RTC_RST       = 27;

  // DFPlayer Mini (pins as seen from the ESP32)
  constexpr uint8_t DFPLAYER_RX   = 16;  // ESP32 RX  <- DFPlayer TX
  constexpr uint8_t DFPLAYER_TX   = 17;  // ESP32 TX  -> DFPlayer RX (via 1 kΩ)

  // Buzzer
  constexpr uint8_t BUZZER        = 33;
}
