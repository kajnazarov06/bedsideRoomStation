#include "PairingController.h"

void PairingController::start(HttpRequest&, HttpResponse& response) {
  if (!_auth.startPairing()) return replyLocked(response);

  JsonDocument body;
  body["expires_in"] = _auth.codeSecondsLeft();
  response.json(200, body);
}

void PairingController::confirm(HttpRequest& request, HttpResponse& response) {
  JsonDocument input = request.json();
  const char* code = input["code"] | "";  // bad JSON -> empty code -> wrong code

  String token;
  switch (_auth.confirmPairing(code, token)) {
    case DeviceAuth::PairResult::Ok: {
      JsonDocument body;
      body["token"] = token;
      return response.json(200, body);
    }
    case DeviceAuth::PairResult::WrongCode: {
      JsonDocument body;
      body["error"] = "wrong code";
      body["attempts_left"] = _auth.attemptsLeft();
      return response.json(401, body);
    }
    case DeviceAuth::PairResult::LockedOut:
      return replyLocked(response);
  }
}

void PairingController::unpair(HttpRequest&, HttpResponse& response) {
  _auth.unpair();

  JsonDocument body;
  body["ok"] = true;
  response.json(200, body);
}

void PairingController::replyLocked(HttpResponse& response) {
  JsonDocument body;
  body["error"] = "locked";
  body["retry_after"] = _auth.retryAfterS();
  response.json(429, body);
}
