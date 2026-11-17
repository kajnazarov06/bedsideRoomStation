#pragma once
#include <LiquidCrystal_I2C.h>
#include "../core/ITextDisplay.h"

class LcdDisplay : public ITextDisplay {
public:
  LcdDisplay(uint8_t sdaPin, uint8_t sclPin,
             uint8_t address = 0x3F, uint8_t cols = 16, uint8_t rows = 2);

  bool begin() override;
  const char* name() const override { return "LCD"; }

  uint8_t cols() const override { return _cols; }
  uint8_t rows() const override { return _rows; }
  void clear() override;
  void printLine(uint8_t row, const char* text) override;

private:
  uint8_t _sdaPin;
  uint8_t _sclPin;
  uint8_t _address;
  uint8_t _cols;
  uint8_t _rows;
  LiquidCrystal_I2C _lcd;
};
