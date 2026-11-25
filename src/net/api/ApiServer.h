#pragma once
#include <WebServer.h>
#include "../../core/IComponent.h"
#include "../../drivers/RtcClock.h"
#include "../../drivers/DhtSensor.h"
#include "../../app/AlarmController.h"
#include "../../core/ITextDisplay.h"
#include "../../storage/SettingsStore.h"
#include "../DeviceAuth.h"
#include "http/Router.h"
#include "middleware/RequireToken.h"
#include "controllers/InfoController.h"
#include "controllers/PairingController.h"
#include "controllers/StateController.h"
#include "controllers/AlarmsController.h"
#include "controllers/SettingsController.h"

// JSON HTTP API. Owns the web server, the middleware and the controllers;
// the endpoints themselves are listed in Routes.cpp.
//
// Call poll() from every loop() pass.
class ApiServer : public IComponent {
public:
  ApiServer(DeviceAuth& auth, RtcClock& clock, DhtSensor& climate,
            AlarmController& alarm, SettingsStore& settings, ITextDisplay& display,
            uint16_t port);

  bool begin() override;
  const char* name() const override { return "API"; }

  void poll() { _server.handleClient(); }

private:
  void registerRoutes();  // defined in Routes.cpp

  WebServer _server;
  Router    _router;  // after _server: it keeps a reference to it

  RequireToken _requireToken;

  InfoController    _info;
  PairingController _pairing;
  StateController   _state;
  AlarmsController   _alarms;
  SettingsController _settings;
};
