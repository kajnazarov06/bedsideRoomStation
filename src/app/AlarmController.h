#pragma once
#include "../core/ITextDisplay.h"
#include "../drivers/Button.h"
#include "../drivers/RtcClock.h"
#include "../drivers/Mp3Player.h"
#include "../drivers/Buzzer.h"
#include "../storage/AlarmStore.h"

// One-button logic for Settings::ALARM_COUNT alarms, each with its own weekdays.
//
//  Clock screen:  Press -> show alarm 1 (time, days, ON/OFF) for a few seconds
//                 Hold  -> edit alarm 1
//  Showing:       Press -> show the next alarm (after the last one: back to the clock)
//                 Hold  -> edit the alarm on screen
//  Editing:       Press -> change the blinking value
//                          (+1 / next day choice / toggle ON-OFF)
//                 Hold  -> next value: hour -> minute -> days -> status -> save
//                 20 s without a press cancels without saving
//  Ringing:       Press or hold -> stop the music
//
// Day choices: every day, Mon-Fri, Sat-Sun, one single day (Mon ... Sun),
// plus the alarm's own custom days if they were set through the API.
class AlarmController {
public:
  AlarmController(ITextDisplay& display, RtcClock& clock, Mp3Player& player,
                  Buzzer& buzzer, AlarmStore& store);

  void begin();                    // load saved alarms
  void handle(ButtonEvent event);  // feed button events
  void update();                   // call every loop()
  void ringNow();                  // start ringing immediately (for testing)

  // True while the alarm screens own the display (dashboard must not draw).
  bool ownsDisplay() const { return _mode != Mode::Idle; }

  const AlarmList& alarms() const { return _alarms; }

  // Replaces alarm `index` (0-based) and saves it. False if the index is out of range.
  bool setAlarm(uint8_t index, const AlarmSettings& settings);

  // Earliest time any enabled alarm rings after `now`. False if none will ring.
  bool nextRing(const RtcDateTime& now, RtcDateTime& next) const;

private:
  enum class Mode  : uint8_t { Idle, Showing, Editing, Ringing };
  enum class Field : uint8_t { Hour, Minute, Days, Status };

  static bool nextRingOf(const AlarmSettings& settings, const RtcDateTime& now, RtcDateTime& next);

  void startShowing(uint8_t index, const char* title);
  void startEditing(uint8_t index);
  void nextField();
  void incrementField();
  uint8_t nextDaysChoice(uint8_t days) const;
  void startRinging(uint8_t index);
  void stopRinging();
  void checkAlarm();
  void logAlarm(const char* prefix, uint8_t index, const AlarmSettings& settings) const;

  void render();
  void renderShowing();
  void renderEditing();
  void renderRinging();

  ITextDisplay& _display;
  RtcClock&     _clock;
  Mp3Player&    _player;
  Buzzer&       _buzzer;
  AlarmStore&   _store;

  AlarmList     _alarms;           // saved values
  AlarmSettings _draft;            // values being edited
  uint8_t       _shownIndex   = 0; // alarm on the Showing / Editing screen
  uint8_t       _ringingIndex = 0; // alarm that is ringing
  uint8_t       _customDays   = 0; // edited alarm's non-preset days, 0 if none

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
