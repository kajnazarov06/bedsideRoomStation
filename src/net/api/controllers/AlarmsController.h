#pragma once
#include "../http/HttpRequest.h"
#include "../http/HttpResponse.h"
#include "../../../app/AlarmController.h"

// GET /api/alarms       {"alarms":[{id, time, days, enabled, label}, ...]}
// PUT /api/alarms/{id}  body: any of {"time":"07:30", "days":[1,2,3,4,5], "enabled":true}
//                       -> the updated alarm; 404 unknown id, 400 bad JSON, 422 bad value
// (Named "Alarms" to keep it apart from the app's AlarmController.)
class AlarmsController {
public:
  explicit AlarmsController(AlarmController& alarm) : _alarm(alarm) {}

  void index(HttpRequest& request, HttpResponse& response);
  void update(HttpRequest& request, HttpResponse& response);

private:
  AlarmController& _alarm;
};
