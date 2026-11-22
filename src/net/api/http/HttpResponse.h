#pragma once
#include <WebServer.h>
#include <ArduinoJson.h>

// Sends JSON replies. Every reply gets "api_version" and is logged to Serial.
class HttpResponse {
public:
  explicit HttpResponse(WebServer& server) : _server(server) {}

  void json(int status, JsonDocument& body);
  void error(int status, const char* message);  // {"error": message}

private:
  WebServer& _server;
};
