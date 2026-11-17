#include "Mp3Player.h"

Mp3Player::Mp3Player(HardwareSerial& serial, uint8_t rxPin, uint8_t txPin)
  : _serial(serial), _rxPin(rxPin), _txPin(txPin) {}

bool Mp3Player::begin() {
  _serial.begin(9600, SERIAL_8N1, _rxPin, _txPin);
  delay(1000);  // DFPlayer needs time to boot and read the SD card
  return _player.begin(_serial);
}

void Mp3Player::setVolume(uint8_t volume) {
  _player.volume(min<uint8_t>(volume, 30));
}

void Mp3Player::play(uint16_t track) {
  _player.play(track);
}
