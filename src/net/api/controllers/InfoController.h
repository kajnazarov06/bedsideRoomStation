#pragma once
#include "../http/HttpRequest.h"
#include "../http/HttpResponse.h"
#include "../../DeviceAuth.h"

// GET /api/info: device identity and capabilities (public).
class InfoController {
public:
  explicit InfoController(const DeviceAuth& auth) : _auth(auth) {}

  void show(HttpRequest& request, HttpResponse& response);

private:
  const DeviceAuth& _auth;
};
