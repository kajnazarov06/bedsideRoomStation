#include "Mp3Player.h"

Mp3Player::Mp3Player(HardwareSerial& serial, uint8_t rxPin, uint8_t txPin)
  : _serial(serial), _rxPin(rxPin), _txPin(txPin) {}

bool Mp3Player::begin() {
  Serial.printf("[MP3] UART2 9600 baud, ESP32 RX=GPIO%u (<- DFPlayer TX), TX=GPIO%u (-> DFPlayer RX)\n",
                _rxPin, _txPin);
  _serial.begin(9600, SERIAL_8N1, _rxPin, _txPin);
  delay(1000);  // DFPlayer needs time to boot and read the SD card

  _online = _player.begin(_serial);
  if (!_online) {
    Serial.println("[MP3] DFPlayer NOT found. Check: TX/RX swapped? 5V power? SD card inserted?");
    return false;
  }

  Serial.println("[MP3] DFPlayer online");
  return true;
}

void Mp3Player::setVolume(uint8_t volume) {
  volume = min<uint8_t>(volume, 30);
  Serial.printf("[MP3] volume -> %u\n", volume);
  _player.volume(volume);
}

void Mp3Player::play(uint16_t track) {
  Serial.printf("[MP3] play track %u%s\n", track, _online ? "" : " (player offline!)");
  _player.play(track);
}

void Mp3Player::loop(uint16_t track) {
  Serial.printf("[MP3] loop track %u%s\n", track, _online ? "" : " (player offline!)");
  _player.loop(track);
}

void Mp3Player::stop() {
  Serial.println("[MP3] stop");
  _player.stop();
}

void Mp3Player::printInfo() {
  // Each read waits for the DFPlayer's answer; -1 means no answer.
  int state  = _player.readState();
  int volume = _player.readVolume();
  int files  = _player.readFileCounts();
  Serial.printf("[MP3] state=%d (0 stopped, 1 playing, 2 paused, -1 no answer)\n", state);
  Serial.printf("[MP3] volume=%d, files on SD=%d\n", volume, files);
  if (files == 0) Serial.println("[MP3] SD card has no MP3 files: put 0001.mp3 in the root folder");
}

void Mp3Player::poll() {
  if (_player.available()) logMessage(_player.readType(), _player.read());
}

void Mp3Player::logMessage(uint8_t type, int value) {
  switch (type) {
    case TimeOut:               Serial.println("[MP3] time out (no answer)");          break;
    case WrongStack:            Serial.println("[MP3] wrong data on the serial line"); break;
    case DFPlayerCardInserted:  Serial.println("[MP3] SD card inserted");              break;
    case DFPlayerCardRemoved:   Serial.println("[MP3] SD card removed");               break;
    case DFPlayerCardOnline:    Serial.println("[MP3] SD card online");                break;
    case DFPlayerPlayFinished:  Serial.printf("[MP3] track %d finished\n", value);     break;
    case DFPlayerError:
      Serial.print("[MP3] ERROR: ");
      switch (value) {
        case Busy:             Serial.println("busy / SD card not found"); break;
        case Sleeping:         Serial.println("sleeping");                  break;
        case SerialWrongStack: Serial.println("wrong serial data");         break;
        case CheckSumNotMatch: Serial.println("checksum mismatch");         break;
        case FileIndexOut:     Serial.println("track number out of range"); break;
        case FileMismatch:     Serial.println("track not found");           break;
        case Advertise:        Serial.println("in advertise");              break;
        default:               Serial.printf("code %d\n", value);           break;
      }
      break;
    default:
      Serial.printf("[MP3] message type=%u value=%d\n", type, value);
      break;
  }
}
