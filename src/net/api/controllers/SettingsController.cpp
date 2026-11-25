#include "SettingsController.h"
#include "../resources/SettingsResource.h"

void SettingsController::show(HttpRequest&, HttpResponse& response) {
  JsonDocument body;
  SettingsResource::toJson(_store.settings(), body.to<JsonObject>());
  response.json(200, body);
}

void SettingsController::update(HttpRequest& request, HttpResponse& response) {
  JsonDocument input = request.json();
  if (!input.is<JsonObject>()) {
    response.error(400, "body must be a JSON object");
    return;
  }

  DeviceSettings settings = _store.settings();
  if (const char* error = SettingsResource::fromJson(input.as<JsonObjectConst>(), settings)) {
    response.error(422, error);
    return;
  }
  _store.save(settings);
  _display.setBrightness(settings.brightness);

  JsonDocument body;
  SettingsResource::toJson(_store.settings(), body.to<JsonObject>());
  response.json(200, body);
}
