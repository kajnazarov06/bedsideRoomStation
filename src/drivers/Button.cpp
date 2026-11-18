#include "Button.h"

Button::Button(uint8_t pin, unsigned long longPressMs, unsigned long debounceMs)
  : _pin(pin), _longPressMs(longPressMs), _debounceMs(debounceMs) {}

bool Button::begin() {
  pinMode(_pin, INPUT_PULLUP);
  _lastRaw = _pressed = (digitalRead(_pin) == LOW);
  _longFired = _pressed;  // ignore a button that is already held at boot
  _lastChange = millis();
  return true;
}

ButtonEvent Button::poll() {
  const unsigned long now = millis();
  const bool raw = (digitalRead(_pin) == LOW);

  if (raw != _lastRaw) {
    _lastRaw = raw;
    _lastChange = now;
  }

  ButtonEvent event = ButtonEvent::None;

  if (now - _lastChange >= _debounceMs && raw != _pressed) {
    _pressed = raw;
    if (_pressed) {
      _pressedAt = now;
      _longFired = false;
    } else if (!_longFired) {
      event = ButtonEvent::Press;
    }
  }

  if (_pressed && !_longFired && now - _pressedAt >= _longPressMs) {
    _longFired = true;
    event = ButtonEvent::LongPress;
  }

  return event;
}
