#pragma once
#include "../http/HttpRequest.h"
#include "../http/HttpResponse.h"
#include "../../DeviceAuth.h"

// POST /api/pair/start    show a pairing code (Serial for now)
// POST /api/pair/confirm  {"code":"123456"} -> {"token":"..."}
// POST /api/unpair        forget the paired client (needs the token)
class PairingController {
public:
  explicit PairingController(DeviceAuth& auth) : _auth(auth) {}

  void start  (HttpRequest& request, HttpResponse& response);
  void confirm(HttpRequest& request, HttpResponse& response);
  void unpair (HttpRequest& request, HttpResponse& response);

private:
  void replyLocked(HttpResponse& response);  // 429 with retry_after

  DeviceAuth& _auth;
};
