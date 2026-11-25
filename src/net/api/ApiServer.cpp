#include "ApiServer.h"

ApiServer::ApiServer(DeviceAuth& auth, RtcClock& clock, DhtSensor& climate,
                     AlarmController& alarm, SettingsStore& settings, ITextDisplay& display,
                     uint16_t port)
  : _server(port),
    _router(_server),
    _requireToken(auth),
    _info(auth, settings),
    _pairing(auth),
    _state(clock, climate, alarm, settings),
    _alarms(alarm),
    _settings(settings, display) {}

bool ApiServer::begin() {
  const char* headers[] = { "Authorization" };  // WebServer only keeps headers it is told about
  _server.collectHeaders(headers, 1);

  registerRoutes();
  _server.begin();
  return true;
}
