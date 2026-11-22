#include "AlarmController.h"
#include "../config/Settings.h"

namespace {
  constexpr unsigned long SHOW_TIME_MS      = 4000;               // info screen
  constexpr unsigned long EDIT_TIMEOUT_MS   = 20000;              // cancel editing
  constexpr unsigned long BLINK_MS          = 400;                // edited value blink
  constexpr unsigned long CLOCK_CHECK_MS    = 500;                // RTC polling
}

AlarmController::AlarmController(ITextDisplay& display, RtcClock& clock,
                                 Mp3Player& player, Buzzer& buzzer,
                                 AlarmStore& store)
  : _display(display), _clock(clock), _player(player),
    _buzzer(buzzer), _store(store) {}

void AlarmController::begin() {
  _settings = _store.load();
  Serial.printf("[ALARM] loaded %02u:%02u %s\n",
                _settings.hour, _settings.minute, _settings.enabled ? "ON" : "OFF");
}

void AlarmController::ringNow() {
  Serial.println("[ALARM] ring now (test)");
  startRinging();
}

// ---------------------------------------------------------------- input

void AlarmController::handle(ButtonEvent event) {
  if (event == ButtonEvent::None) return;
  Serial.printf("[BUTTON] %s\n", event == ButtonEvent::Press ? "press" : "hold");

  switch (_mode) {
    case Mode::Idle:
    case Mode::Showing:
      if (event == ButtonEvent::Press) {
        _buzzer.beep(2000, 20);
        startShowing("Alarm");
      } else {
        _buzzer.beep(1500, 80);
        startEditing();
      }
      break;

    case Mode::Editing:
      _lastInput = millis();
      if (event == ButtonEvent::Press) {
        _buzzer.beep(2000, 20);
        incrementField();
      } else {
        _buzzer.beep(1500, 80);
        nextField();
      }
      break;

    case Mode::Ringing:
      stopRinging();
      break;
  }
}

// ---------------------------------------------------------------- modes

void AlarmController::startShowing(const char* title) {
  _mode = Mode::Showing;
  _title = title;
  _modeSince = millis();
  _dirty = true;
}

void AlarmController::startEditing() {
  _mode = Mode::Editing;
  _field = Field::Hour;
  _draft = _settings;
  Serial.println("[ALARM] editing: hour");
  _modeSince = _lastInput = millis();
  _blinkHidden = false;
  _dirty = true;
}

void AlarmController::nextField() {
  switch (_field) {
    case Field::Hour:   _field = Field::Minute; Serial.println("[ALARM] editing: minute"); break;
    case Field::Minute: _field = Field::Status; Serial.println("[ALARM] editing: ON/OFF"); break;
    case Field::Status:
      _settings = _draft;
      _store.save(_settings);
      Serial.printf("[ALARM] saved %02u:%02u %s\n",
                    _settings.hour, _settings.minute, _settings.enabled ? "ON" : "OFF");
      _lastTriggerMinute = UINT32_MAX;  // allow the new time to ring today
      startShowing("Saved!");
      return;
  }
  _modeSince = millis();  // restart blink so the new value is visible
  _blinkHidden = false;
  _dirty = true;
}

void AlarmController::incrementField() {
  switch (_field) {
    case Field::Hour:   _draft.hour   = (_draft.hour + 1) % 24;  break;
    case Field::Minute: _draft.minute = (_draft.minute + 1) % 60; break;
    case Field::Status: _draft.enabled = !_draft.enabled;         break;
  }
  Serial.printf("[ALARM] draft %02u:%02u %s\n",
                _draft.hour, _draft.minute, _draft.enabled ? "ON" : "OFF");
  _modeSince = millis();
  _blinkHidden = false;
  _dirty = true;
}

void AlarmController::startRinging() {
  Serial.println("[ALARM] RINGING");
  _mode = Mode::Ringing;
  _modeSince = millis();
  _player.setVolume(Settings::ALARM_VOLUME);
  _player.loop(Settings::ALARM_TRACK);
  _dirty = true;
}

void AlarmController::stopRinging() {
  Serial.println("[ALARM] stopped");
  _player.stop();
  _mode = Mode::Idle;
}

// ---------------------------------------------------------------- update

void AlarmController::update() {
  if (millis() - _lastClockRead >= CLOCK_CHECK_MS) {
    _lastClockRead = millis();
    checkAlarm();
  }

  // Read after checkAlarm(): it may start ringing and set _modeSince, which
  // must not be later than `now` or the unsigned subtraction below wraps.
  const unsigned long now = millis();

  switch (_mode) {
    case Mode::Idle:
      return;

    case Mode::Showing:
      if (now - _modeSince >= SHOW_TIME_MS) _mode = Mode::Idle;
      break;

    case Mode::Editing: {
      if (now - _lastInput >= EDIT_TIMEOUT_MS) {
        Serial.println("[ALARM] editing timed out, not saved");
        startShowing("Not saved");
        break;
      }
      bool hidden = ((now - _modeSince) / BLINK_MS) % 2 == 1;
      if (hidden != _blinkHidden) {
        _blinkHidden = hidden;
        _dirty = true;
      }
      break;
    }

    case Mode::Ringing:
      if (now - _modeSince >= Settings::RING_MAX_MS) {
        Serial.println("[ALARM] auto-stop after 10 min");
        stopRinging();
      }
      break;
  }

  if (_dirty && _mode != Mode::Idle) render();
}

void AlarmController::checkAlarm() {
  if (!_settings.enabled || _mode == Mode::Ringing) return;

  RtcDateTime t = _clock.now();
  if (!t.IsValid()) {
    Serial.println("[ALARM] RTC time invalid, cannot check alarm");
    return;
  }
  if (t.Hour() != _settings.hour || t.Minute() != _settings.minute) return;

  const uint32_t minuteKey = t.TotalSeconds() / 60;
  if (minuteKey == _lastTriggerMinute) return;  // already rang this minute
  _lastTriggerMinute = minuteKey;
  Serial.printf("[ALARM] time reached %02u:%02u\n", t.Hour(), t.Minute());

  startRinging();
}

// ---------------------------------------------------------------- drawing

void AlarmController::render() {
  _dirty = false;
  switch (_mode) {
    case Mode::Showing: renderShowing(); break;
    case Mode::Editing: renderEditing(); break;
    case Mode::Ringing: renderRinging(); break;
    case Mode::Idle:    break;
  }
}

// "Alarm      07:30"
// "ON   in 7h 12m"   or   "OFF"
void AlarmController::renderShowing() {
  char top[17], bottom[17];
  snprintf(top, sizeof(top), "%-10s %02u:%02u", _title, _settings.hour, _settings.minute);

  if (!_settings.enabled) {
    snprintf(bottom, sizeof(bottom), "Status: OFF");
  } else {
    RtcDateTime t = _clock.now();
    if (t.IsValid()) {
      int nowMin   = t.Hour() * 60 + t.Minute();
      int alarmMin = _settings.hour * 60 + _settings.minute;
      int diff     = (alarmMin - nowMin + 1440) % 1440;
      if (diff == 0) diff = 1440;
      snprintf(bottom, sizeof(bottom), "ON  in %dh %02dm", diff / 60, diff % 60);
    } else {
      snprintf(bottom, sizeof(bottom), "Status: ON");
    }
  }

  _display.printLine(0, top);
  _display.printLine(1, bottom);
}

// "Set hour"
// "  >07:30   ON"   (the edited value blinks)
void AlarmController::renderEditing() {
  static const char* const labels[] = { "Set hour", "Set minute", "Set ON/OFF" };

  char hh[4], mm[4], st[4];
  snprintf(hh, sizeof(hh), "%02u", _draft.hour);
  snprintf(mm, sizeof(mm), "%02u", _draft.minute);
  snprintf(st, sizeof(st), "%s", _draft.enabled ? "ON" : "OFF");

  if (_blinkHidden) {
    switch (_field) {
      case Field::Hour:   strcpy(hh, "  ");  break;
      case Field::Minute: strcpy(mm, "  ");  break;
      case Field::Status: strcpy(st, "   "); break;
    }
  }

  char bottom[17];
  snprintf(bottom, sizeof(bottom), "  %s:%s   %s", hh, mm, st);

  _display.printLine(0, labels[static_cast<uint8_t>(_field)]);
  _display.printLine(1, bottom);
}

// "WAKE UP!   07:30"
// "Press to stop"
void AlarmController::renderRinging() {
  char top[17];
  snprintf(top, sizeof(top), "WAKE UP!   %02u:%02u", _settings.hour, _settings.minute);
  _display.printLine(0, top);
  _display.printLine(1, "Press to stop");
}
