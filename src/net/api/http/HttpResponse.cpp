#include "HttpResponse.h"
#include "HttpRequest.h"

namespace {
  constexpr int API_VERSION = 1;
}

void HttpResponse::json(int status, JsonDocument& body) {
  body["api_version"] = API_VERSION;
  String text;
  serializeJson(body, text);
  _server.send(status, "application/json", text);

  HttpRequest request(_server);
  Serial.printf("[API] %s %s -> %d\n", request.method(), request.path().c_str(), status);
}

void HttpResponse::error(int status, const char* message) {
  JsonDocument body;
  body["error"] = message;
  json(status, body);
}
