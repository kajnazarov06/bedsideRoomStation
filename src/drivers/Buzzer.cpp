#include "Buzzer.h"

Buzzer::Buzzer(uint8_t pin) : _pin(pin) {}

bool Buzzer::begin() {
  pinMode(_pin, OUTPUT);
  digitalWrite(_pin, LOW);
  return true;
}

void Buzzer::beep(uint16_t frequencyHz, uint16_t durationMs) {
  tone(_pin, frequencyHz, durationMs);
  delay(durationMs);
  noTone(_pin);
}

void Buzzer::playMelody(const Note* notes, size_t count, uint16_t gapMs) {
  for (size_t i = 0; i < count; ++i) {
    beep(notes[i].frequencyHz, notes[i].durationMs);
    delay(gapMs);
  }
}
