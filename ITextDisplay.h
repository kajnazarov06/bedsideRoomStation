#pragma once
#include "IComponent.h"

// Abstraction for any line-based text display, so views don't depend on a
// specific LCD driver (Dependency Inversion).
class ITextDisplay : public IComponent {
public:
  virtual uint8_t cols() const = 0;
  virtual uint8_t rows() const = 0;
  virtual void clear() = 0;
  // Writes text on a row, padding/truncating to the display width.
  virtual void printLine(uint8_t row, const char* text) = 0;
};
