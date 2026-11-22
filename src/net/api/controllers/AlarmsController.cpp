#include "AlarmsController.h"
#include "../resources/AlarmResource.h"

void AlarmsController::index(HttpRequest&, HttpResponse& response) {
  JsonDocument body;
  JsonArray alarms = body["alarms"].to<JsonArray>();
  AlarmResource::toJson(_alarm.settings(), alarms.add<JsonObject>());
  response.json(200, body);
}
