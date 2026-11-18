#pragma once
#include "../core/ITextDisplay.h"
#include "../drivers/Button.h"
#include "../drivers/RtcClock.h"
#include "../drivers/Mp3Player.h"
#include "../drivers/Buzzer.h"
#include "../storage/AlarmStore.h"

// One-button alarm logic.
//
//  Clock screen:  Press -> show alarm time + ON/OFF for a few seconds
//                 Hold  -> start editing
//  Editing:       Press -> change the blinking value (+1 / toggle ON-OFF)
//                 Hold  -> next value: hour -> minute -> status -> save
//                 20 s without a press cancels without saving
//  Ringing:       Press or hold -> stop the music
class AlarmController {
public:
  AlarmController(ITextDisplay& display, RtcClock& clock, Mp3Player& player,
                  Buzzer& buzzer, AlarmStore& store);

  void begin();                    // load saved settings
  void handle(ButtonEvent event);  // feed button events
  void update();                   // call every loop()

  // True while the alarm screens own the display (dashboard must not draw).
  bool ownsDisplay() const { return _mode != Mode::Idle; }

private:
  enum class Mode  : uint8_t { Idle, Showing, Editing, Ringing };
  enum class Field : uint8_t { Hour, Minute, Status };

  void startShowing(const char* title);
  void startEditing();
  void nextField();
  void incrementField();
  void startRinging();
  void stopRinging();
  void checkAlarm();

  void render();
  void renderShowing();
  void renderEditing();
  void renderRinging();

  ITextDisplay& _display;
  RtcClock&     _clock;
  Mp3Player&    _player;
  Buzzer&       _buzzer;
  AlarmStore&   _store;

  AlarmSettings _settings;  // saved values
  AlarmSettings _draft;     // values being edited

  Mode          _mode  = Mode::Idle;
  Field         _field = Field::Hour;
  const char*   _title = "Alarm";

  unsigned long _modeSince     = 0;  // when the current mode started
  unsigned long _lastInput     = 0;  // last button event while editing
  unsigned long _lastClockRead = 0;
  uint32_t      _lastTriggerMinute = UINT32_MAX;
  bool          _dirty         = false;
  bool          _blinkHidden   = false;
};
