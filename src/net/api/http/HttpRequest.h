#pragma once
#include <WebServer.h>
#include <ArduinoJson.h>

// Read-only view of the request being handled.
class HttpRequest {
public:
  explicit HttpRequest(WebServer& server) : _server(server) {}

  const char* method() const;
  String      path() const { return _server.uri(); }
  String      header(const char* name) const { return _server.header(name); }

  // Token from "Authorization: Bearer <token>", empty when missing.
  String bearerToken() const;

  // Body parsed as JSON. Bad or missing JSON gives an empty document.
  JsonDocument json() const;

private:
  WebServer& _server;
};
