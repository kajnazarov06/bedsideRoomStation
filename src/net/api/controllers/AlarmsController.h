#pragma once
#include "../http/HttpRequest.h"
#include "../http/HttpResponse.h"
#include "../../../app/AlarmController.h"

// GET /api/alarms: {"alarms":[{id, time, days, enabled, label}]}
// (Named "Alarms" to keep it apart from the app's AlarmController.)
class AlarmsController {
public:
  explicit AlarmsController(const AlarmController& alarm) : _alarm(alarm) {}

  void index(HttpRequest& request, HttpResponse& response);

private:
  const AlarmController& _alarm;
};
