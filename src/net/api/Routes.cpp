#include "ApiServer.h"

// Every endpoint of the API (api_version 1), in one place.
//
//   GET  /api/info          public   device identity and capabilities
//   POST /api/pair/start    public   show a pairing code (Serial for now)
//   POST /api/pair/confirm  public   {"code":"123456"} -> {"token":"..."}
//   POST /api/unpair        token    forget the paired client
//   GET  /api/state         token    time, climate, WiFi, next alarm
//   GET  /api/alarms        token    {"alarms":[{id, time:"07:00", days:[1..7], enabled, label}]}
void ApiServer::registerRoutes() {
  _router.get ("/api/info",         Router::action(_info,    &InfoController::show));
  _router.post("/api/pair/start",   Router::action(_pairing, &PairingController::start));
  _router.post("/api/pair/confirm", Router::action(_pairing, &PairingController::confirm));

  _router.post("/api/unpair", Router::action(_pairing, &PairingController::unpair), { &_requireToken });
  _router.get ("/api/state",  Router::action(_state,   &StateController::show),     { &_requireToken });
  _router.get ("/api/alarms", Router::action(_alarms,  &AlarmsController::index),   { &_requireToken });

  _router.fallback([](HttpRequest&, HttpResponse& response) {
    response.error(404, "not found");
  });
}
