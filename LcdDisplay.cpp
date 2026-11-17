#include <Wire.h>
#include "LcdDisplay.h"

LcdDisplay::LcdDisplay(uint8_t sdaPin, uint8_t sclPin,
                       uint8_t address, uint8_t cols, uint8_t rows)
  : _sdaPin(sdaPin), _sclPin(sclPin), _address(address),
    _cols(cols), _rows(rows), _lcd(address, cols, rows) {}

bool LcdDisplay::begin() {
  Wire.begin(_sdaPin, _sclPin);

  Wire.beginTransmission(_address);
  bool found = (Wire.endTransmission() == 0);

  _lcd.init();
  _lcd.backlight();
  return found;
}

void LcdDisplay::clear() {
  _lcd.clear();
}

void LcdDisplay::printLine(uint8_t row, const char* text) {
  if (row >= _rows) return;
  _lcd.setCursor(0, row);

  // Write exactly `_cols` characters so leftovers from the previous
  // content are overwritten without a flickering clear().
  uint8_t i = 0;
  for (; i < _cols && text[i] != '\0'; ++i) _lcd.write(text[i]);
  for (; i < _cols; ++i) _lcd.write(' ');
}
