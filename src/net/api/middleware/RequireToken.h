#pragma once
#include "../http/IMiddleware.h"
#include "../../DeviceAuth.h"

// Lets the request through only with a valid "Authorization: Bearer <token>",
// otherwise answers 401.
class RequireToken : public IMiddleware {
public:
  explicit RequireToken(const DeviceAuth& auth) : _auth(auth) {}

  bool handle(HttpRequest& request, HttpResponse& response) override;

private:
  const DeviceAuth& _auth;
};
