#pragma once
#include <HardwareSerial.h>
#include <DFRobotDFPlayerMini.h>
#include "IComponent.h"

class Mp3Player : public IComponent {
public:
  Mp3Player(HardwareSerial& serial, uint8_t rxPin, uint8_t txPin);

  bool begin() override;
  const char* name() const override { return "DFPlayer"; }

  void setVolume(uint8_t volume);  // 0..30
  void play(uint16_t track);

private:
  HardwareSerial& _serial;
  uint8_t _rxPin;
  uint8_t _txPin;
  DFRobotDFPlayerMini _player;
};
