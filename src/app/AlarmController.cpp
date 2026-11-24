#include "AlarmController.h"
#include "../config/Settings.h"

namespace {
  constexpr unsigned long SHOW_TIME_MS      = 4000;               // info screen
  constexpr unsigned long EDIT_TIMEOUT_MS   = 20000;              // cancel editing
  constexpr unsigned long BLINK_MS          = 400;                // edited value blink
  constexpr unsigned long CLOCK_CHECK_MS    = 500;                // RTC polling
  constexpr uint32_t      SECONDS_PER_DAY   = 24UL * 60 * 60;

  // What a press cycles through while editing the days.
  constexpr uint8_t DAY_CHOICES[] = {
    AlarmDays::EVERY_DAY, AlarmDays::WEEKDAYS, AlarmDays::WEEKEND,
    AlarmDays::forDay(1), AlarmDays::forDay(2), AlarmDays::forDay(3), AlarmDays::forDay(4),
    AlarmDays::forDay(5), AlarmDays::forDay(6), AlarmDays::forDay(7),
  };
  constexpr size_t DAY_CHOICE_COUNT = sizeof(DAY_CHOICES) / sizeof(DAY_CHOICES[0]);

  bool isDayChoice(uint8_t days) {
    for (uint8_t choice : DAY_CHOICES) if (choice == days) return true;
    return false;
  }

  // 1 = Monday ... 7 = Sunday (RtcDateTime counts from Sunday = 0).
  uint8_t isoWeekday(const RtcDateTime& dateTime) {
    const uint8_t dayOfWeek = dateTime.DayOfWeek();
    return dayOfWeek == 0 ? 7 : dayOfWeek;
  }

  // "MTWTF--": one letter per ringing day, '-' for the others. `text` needs 8 bytes.
  void formatDays(uint8_t days, char* text) {
    static const char LETTERS[] = "MTWTFSS";
    for (uint8_t i = 0; i < 7; i++) text[i] = (days & (1 << i)) ? LETTERS[i] : '-';
    text[7] = '\0';
  }
}

AlarmController::AlarmController(ITextDisplay& display, RtcClock& clock,
                                 Mp3Player& player, Buzzer& buzzer,
                                 AlarmStore& store)
  : _display(display), _clock(clock), _player(player),
    _buzzer(buzzer), _store(store) {}

void AlarmController::begin() {
  _alarms = _store.load();
  for (uint8_t i = 0; i < _alarms.size(); i++) logAlarm("loaded", i, _alarms[i]);
}

void AlarmController::ringNow() {
  Serial.println("[ALARM] ring now (test)");
  startRinging(0);
}

bool AlarmController::setAlarm(uint8_t index, const AlarmSettings& settings) {
  if (index >= _alarms.size()) return false;
  _alarms[index] = settings;
  _store.save(_alarms);
  logAlarm("set", index, settings);
  _lastTriggerMinute = UINT32_MAX;  // allow the new time to ring this minute
  if (_mode == Mode::Showing && _shownIndex == index) _dirty = true;
  return true;
}

bool AlarmController::nextRing(const RtcDateTime& now, RtcDateTime& next) const {
  bool found = false;
  for (const AlarmSettings& settings : _alarms) {
    RtcDateTime candidate;
    if (nextRingOf(settings, now, candidate) && (!found || candidate < next)) {
      next  = candidate;
      found = true;
    }
  }
  return found;
}

bool AlarmController::nextRingOf(const AlarmSettings& settings, const RtcDateTime& now,
                                 RtcDateTime& next) {
  if (!settings.enabled || (settings.days & AlarmDays::EVERY_DAY) == 0) return false;

  RtcDateTime today(now.Year(), now.Month(), now.Day(), settings.hour, settings.minute, 0);
  for (uint32_t dayOffset = 0; dayOffset <= 7; dayOffset++) {
    RtcDateTime candidate = today + dayOffset * SECONDS_PER_DAY;
    if (candidate > now && settings.ringsOn(isoWeekday(candidate))) {
      next = candidate;
      return true;
    }
  }
  return false;
}

// ---------------------------------------------------------------- input

void AlarmController::handle(ButtonEvent event) {
  if (event == ButtonEvent::None) return;
  Serial.printf("[BUTTON] %s\n", event == ButtonEvent::Press ? "press" : "hold");

  switch (_mode) {
    case Mode::Idle:
      if (event == ButtonEvent::Press) {
        _buzzer.beep(2000, 20);
        startShowing(0, "Alarm");
      } else {
        _buzzer.beep(1500, 80);
        startEditing(0);
      }
      break;

    case Mode::Showing:
      if (event == ButtonEvent::Press) {
        _buzzer.beep(2000, 20);
        if (_shownIndex + 1 < _alarms.size()) startShowing(_shownIndex + 1, "Alarm");
        else                                  _mode = Mode::Idle;  // back to the clock
      } else {
        _buzzer.beep(1500, 80);
        startEditing(_shownIndex);
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

void AlarmController::startShowing(uint8_t index, const char* title) {
  _mode = Mode::Showing;
  _shownIndex = index;
  _title = title;
  _modeSince = millis();
  _dirty = true;
}

void AlarmController::startEditing(uint8_t index) {
  _mode = Mode::Editing;
  _shownIndex = index;
  _field = Field::Hour;
  _draft = _alarms[index];
  _customDays = isDayChoice(_draft.days) ? 0 : _draft.days;
  Serial.printf("[ALARM] editing alarm %u: hour\n", index + 1);
  _modeSince = _lastInput = millis();
  _blinkHidden = false;
  _dirty = true;
}

void AlarmController::nextField() {
  switch (_field) {
    case Field::Hour:   _field = Field::Minute; Serial.println("[ALARM] editing: minute"); break;
    case Field::Minute: _field = Field::Days;   Serial.println("[ALARM] editing: days");   break;
    case Field::Days:   _field = Field::Status; Serial.println("[ALARM] editing: ON/OFF"); break;
    case Field::Status:
      setAlarm(_shownIndex, _draft);
      startShowing(_shownIndex, "Saved");
      return;
  }
  _modeSince = millis();  // restart blink so the new value is visible
  _blinkHidden = false;
  _dirty = true;
}

void AlarmController::incrementField() {
  switch (_field) {
    case Field::Hour:   _draft.hour    = (_draft.hour + 1) % 24;    break;
    case Field::Minute: _draft.minute  = (_draft.minute + 1) % 60;  break;
    case Field::Days:   _draft.days    = nextDaysChoice(_draft.days); break;
    case Field::Status: _draft.enabled = !_draft.enabled;           break;
  }
  logAlarm("draft", _shownIndex, _draft);
  _modeSince = millis();
  _blinkHidden = false;
  _dirty = true;
}

// Next entry of DAY_CHOICES; the alarm's own custom days come after the last one.
uint8_t AlarmController::nextDaysChoice(uint8_t days) const {
  for (size_t i = 0; i < DAY_CHOICE_COUNT; i++) {
    if (DAY_CHOICES[i] != days) continue;
    if (i + 1 < DAY_CHOICE_COUNT) return DAY_CHOICES[i + 1];
    return _customDays != 0 ? _customDays : DAY_CHOICES[0];
  }
  return DAY_CHOICES[0];  // custom days -> start over
}

void AlarmController::startRinging(uint8_t index) {
  Serial.printf("[ALARM] RINGING alarm %u\n", index + 1);
  _mode = Mode::Ringing;
  _ringingIndex = index;
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
        startShowing(_shownIndex, "Cancel");
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
  if (_mode == Mode::Ringing) return;

  bool anyEnabled = false;
  for (const AlarmSettings& settings : _alarms) anyEnabled |= settings.enabled;
  if (!anyEnabled) return;

  RtcDateTime dateTime = _clock.now();
  if (!dateTime.IsValid()) {
    Serial.println("[ALARM] RTC time invalid, cannot check alarm");
    return;
  }

  const uint32_t minuteKey = dateTime.TotalSeconds() / 60;
  if (minuteKey == _lastTriggerMinute) return;  // already rang this minute

  const uint8_t today = isoWeekday(dateTime);
  for (uint8_t i = 0; i < _alarms.size(); i++) {
    const AlarmSettings& settings = _alarms[i];
    if (!settings.enabled || !settings.ringsOn(today)) continue;
    if (dateTime.Hour() != settings.hour || dateTime.Minute() != settings.minute) continue;

    _lastTriggerMinute = minuteKey;
    Serial.printf("[ALARM] alarm %u time reached %02u:%02u\n", i + 1, dateTime.Hour(), dateTime.Minute());
    startRinging(i);
    return;
  }
}

void AlarmController::logAlarm(const char* prefix, uint8_t index,
                               const AlarmSettings& settings) const {
  char days[8];
  formatDays(settings.days, days);
  Serial.printf("[ALARM] %s %u: %02u:%02u %s %s\n", prefix, index + 1,
                settings.hour, settings.minute, days, settings.enabled ? "ON" : "OFF");
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

// "Alarm 1    07:30"
// "MTWTF-- in 7h12"   (hours+minutes, or "2d03h" when days away)   or   "MTWTF--      OFF"
void AlarmController::renderShowing() {
  const AlarmSettings& settings = _alarms[_shownIndex];

  char heading[12];
  snprintf(heading, sizeof(heading), "%s %u", _title, _shownIndex + 1);
  char top[17];
  snprintf(top, sizeof(top), "%-10s %02u:%02u", heading, settings.hour, settings.minute);

  char days[8];
  formatDays(settings.days, days);

  char status[10] = "OFF";
  if (settings.enabled) {
    RtcDateTime now = _clock.now();
    RtcDateTime next;
    if (now.IsValid() && nextRingOf(settings, now, next)) {
      const uint32_t minutesLeft = (next.TotalSeconds() - now.TotalSeconds() + 59) / 60;
      if (minutesLeft < 24 * 60) {
        snprintf(status, sizeof(status), "in %luh%02lu", (unsigned long)(minutesLeft / 60),
                 (unsigned long)(minutesLeft % 60));
      } else {
        snprintf(status, sizeof(status), "in %lud%02luh", (unsigned long)(minutesLeft / (24 * 60)),
                 (unsigned long)(minutesLeft / 60 % 24));
      }
    } else {
      snprintf(status, sizeof(status), "ON");
    }
  }

  char bottom[17];
  snprintf(bottom, sizeof(bottom), "%s %8s", days, status);

  _display.printLine(0, top);
  _display.printLine(1, bottom);
}

// "Set days      ON"
// "#1 07:30 MTWTF--"   (the edited value blinks)
void AlarmController::renderEditing() {
  static const char* const labels[] = { "Set hour", "Set minute", "Set days", "Set ON/OFF" };

  char hour[3], minute[3], days[8], status[4];
  snprintf(hour, sizeof(hour), "%02u", _draft.hour);
  snprintf(minute, sizeof(minute), "%02u", _draft.minute);
  formatDays(_draft.days, days);
  snprintf(status, sizeof(status), "%s", _draft.enabled ? "ON" : "OFF");

  if (_blinkHidden) {
    switch (_field) {
      case Field::Hour:   strcpy(hour, "  ");      break;
      case Field::Minute: strcpy(minute, "  ");    break;
      case Field::Days:   strcpy(days, "       "); break;
      case Field::Status: strcpy(status, "   ");   break;
    }
  }

  char top[17], bottom[17];
  snprintf(top, sizeof(top), "%-12s%4s", labels[static_cast<uint8_t>(_field)], status);
  snprintf(bottom, sizeof(bottom), "#%u %s:%s %s", _shownIndex + 1, hour, minute, days);

  _display.printLine(0, top);
  _display.printLine(1, bottom);
}

// "WAKE UP!   07:30"
// "Press to stop"
void AlarmController::renderRinging() {
  const AlarmSettings& settings = _alarms[_ringingIndex];
  char top[17];
  snprintf(top, sizeof(top), "WAKE UP!   %02u:%02u", settings.hour, settings.minute);
  _display.printLine(0, top);
  _display.printLine(1, "Press to stop");
}
