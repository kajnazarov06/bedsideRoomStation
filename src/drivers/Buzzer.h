#pragma once
#include "IComponent.h"

struct Note {
  uint16_t frequencyHz;
  uint16_t durationMs;
};

class Buzzer : public IComponent {
public:
  explicit Buzzer(uint8_t pin);

  bool begin() override;
  const char* name() const override { return "Buzzer"; }

  void beep(uint16_t frequencyHz, uint16_t durationMs);
  void playMelody(const Note* notes, size_t count, uint16_t gapMs = 50);

private:
  uint8_t _pin;
};
