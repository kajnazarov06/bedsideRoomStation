#pragma once
#include <WebServer.h>
#include <ArduinoJson.h>
#include "../core/IComponent.h"
#include "../drivers/RtcClock.h"
#include "../drivers/DhtSensor.h"
#include "../app/AlarmController.h"
#include "DeviceAuth.h"

// JSON HTTP API (api_version 1):
//
//   GET  /api/info          public   device identity and capabilities
//   POST /api/pair/start    public   show a pairing code (Serial for now)
//   POST /api/pair/confirm  public   {"code":"123456"} -> {"token":"..."}
//   POST /api/unpair        token    forget the paired client
//   GET  /api/state         token    time, climate, WiFi, next alarm
//   GET  /api/alarms        token    {"alarms":[{id, time:"07:00", days:[1..7], enabled, label}]}
//                                    days are ISO weekdays: 1 = Monday ... 7 = Sunday
//
// Call poll() from every loop() pass.
class ApiServer : public IComponent {
public:
  ApiServer(DeviceAuth& auth, RtcClock& clock, DhtSensor& climate,
            const AlarmController& alarm, uint16_t port);

  bool begin() override;
  const char* name() const override { return "API"; }

  void poll();

private:
  void handleInfo();
  void handlePairStart();
  void handlePairConfirm();
  void handleUnpair();
  void handleState();
  void handleAlarms();
  void handleNotFound();

  bool requireAuth();                       // sends 401 and returns false if not authorised
  void reply(int code, JsonDocument& doc);
  void replyLocked();

  WebServer              _server;
  DeviceAuth&            _auth;
  RtcClock&              _clock;
  DhtSensor&             _climate;
  const AlarmController& _alarm;
  String                 _deviceId;
};
