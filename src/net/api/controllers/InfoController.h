#pragma once
#include "../http/HttpRequest.h"
#include "../http/HttpResponse.h"
#include "../../DeviceAuth.h"
#include "../../../storage/SettingsStore.h"

// GET /api/info: device identity and capabilities (public).
class InfoController {
public:
  InfoController(const DeviceAuth& auth, const SettingsStore& settings)
    : _auth(auth), _settings(settings) {}

  void show(HttpRequest& request, HttpResponse& response);

private:
  const DeviceAuth&    _auth;
  const SettingsStore& _settings;
};
