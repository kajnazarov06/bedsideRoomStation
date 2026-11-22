#pragma once
#include "../http/HttpRequest.h"
#include "../http/HttpResponse.h"
#include "../../../drivers/RtcClock.h"
#include "../../../drivers/DhtSensor.h"
#include "../../../app/AlarmController.h"

// GET /api/state: time, climate, WiFi, next alarm.
class StateController {
public:
  StateController(RtcClock& clock, DhtSensor& climate, const AlarmController& alarm)
    : _clock(clock), _climate(climate), _alarm(alarm) {}

  void show(HttpRequest& request, HttpResponse& response);

private:
  RtcClock&              _clock;
  DhtSensor&             _climate;
  const AlarmController& _alarm;
};
