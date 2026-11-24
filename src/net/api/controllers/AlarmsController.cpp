#include "AlarmsController.h"
#include "../resources/AlarmResource.h"

void AlarmsController::index(HttpRequest&, HttpResponse& response) {
  JsonDocument body;
  JsonArray alarms = body["alarms"].to<JsonArray>();
  for (uint8_t i = 0; i < _alarm.alarms().size(); i++) {
    AlarmResource::toJson(i, _alarm.alarms()[i], alarms.add<JsonObject>());
  }
  response.json(200, body);
}

void AlarmsController::update(HttpRequest& request, HttpResponse& response) {
  const long id = request.param(0).toInt();  // "abc" gives 0 -> not found
  if (id < 1 || id > (long)_alarm.alarms().size()) {
    response.error(404, "alarm not found");
    return;
  }
  const uint8_t index = id - 1;

  JsonDocument input = request.json();
  if (!input.is<JsonObject>()) {
    response.error(400, "body must be a JSON object");
    return;
  }

  AlarmSettings settings = _alarm.alarms()[index];
  if (const char* error = AlarmResource::fromJson(input.as<JsonObjectConst>(), settings)) {
    response.error(422, error);
    return;
  }
  _alarm.setAlarm(index, settings);

  JsonDocument body;
  AlarmResource::toJson(index, _alarm.alarms()[index], body.to<JsonObject>());
  response.json(200, body);
}
