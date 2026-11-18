#pragma once
#include "../core/IComponent.h"

enum class ButtonEvent : uint8_t {
  None,
  Press,      // short tap, reported on release
  LongPress,  // reported once, as soon as the hold time is reached
};

// Debounced push button wired between a GPIO and GND (internal pull-up).
class Button : public IComponent {
public:
  explicit Button(uint8_t pin, unsigned long longPressMs = 1500,
                  unsigned long debounceMs = 30);

  bool begin() override;
  const char* name() const override { return "Button"; }

  // Call every loop(); returns at most one event per call.
  ButtonEvent poll();

private:
  uint8_t       _pin;
  unsigned long _longPressMs;
  unsigned long _debounceMs;

  bool          _lastRaw    = false;
  bool          _pressed    = false;  // debounced state
  bool          _longFired  = false;
  unsigned long _lastChange = 0;
  unsigned long _pressedAt  = 0;
};
