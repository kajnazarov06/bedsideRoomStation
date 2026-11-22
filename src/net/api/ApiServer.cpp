#include "ApiServer.h"

ApiServer::ApiServer(DeviceAuth& auth, RtcClock& clock, DhtSensor& climate,
                     const AlarmController& alarm, uint16_t port)
  : _server(port),
    _router(_server),
    _requireToken(auth),
    _info(auth),
    _pairing(auth),
    _state(clock, climate, alarm),
    _alarms(alarm) {}

bool ApiServer::begin() {
  const char* headers[] = { "Authorization" };  // WebServer only keeps headers it is told about
  _server.collectHeaders(headers, 1);

  registerRoutes();
  _server.begin();
  return true;
}
