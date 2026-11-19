#pragma once
#include <HardwareSerial.h>
#include <DFRobotDFPlayerMini.h>
#include "../core/IComponent.h"

class Mp3Player : public IComponent {
public:
  Mp3Player(HardwareSerial& serial, uint8_t rxPin, uint8_t txPin);

  bool begin() override;
  const char* name() const override { return "DFPlayer"; }

  void setVolume(uint8_t volume);  // 0..30
  void play(uint16_t track);
  void loop(uint16_t track);       // repeat a track until stop()
  void stop();

  void poll();       // call every loop(): logs messages sent by the DFPlayer
  void printInfo();  // logs state, volume and number of files on the SD card

private:
  void logMessage(uint8_t type, int value);

  HardwareSerial& _serial;
  uint8_t _rxPin;
  uint8_t _txPin;
  bool    _online = false;
  DFRobotDFPlayerMini _player;
};
