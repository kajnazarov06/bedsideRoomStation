#include "InfoController.h"
#include <WiFi.h>
#include "../../DeviceIdentity.h"
#include "../../../config/Settings.h"

void InfoController::show(HttpRequest&, HttpResponse& response) {
  JsonDocument body;
  body["id"]     = DeviceIdentity::id();
  body["type"]   = Settings::DEVICE_TYPE;
  body["name"]   = Settings::DEVICE_NAME;
  body["fw"]     = Settings::FIRMWARE_VERSION;
  body["mac"]    = WiFi.macAddress();
  body["paired"] = _auth.isPaired();

  JsonArray capabilities = body["capabilities"].to<JsonArray>();
  capabilities.add("clock");
  capabilities.add("temperature");
  capabilities.add("humidity");
  capabilities.add("alarm");

  response.json(200, body);
}
