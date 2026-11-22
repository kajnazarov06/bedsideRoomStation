#pragma once
#include "HttpRequest.h"
#include "HttpResponse.h"

// Runs before a route's handler.
class IMiddleware {
public:
  virtual ~IMiddleware() = default;

  // true: continue to the next middleware / the handler.
  // false: the middleware already sent a response, stop here.
  virtual bool handle(HttpRequest& request, HttpResponse& response) = 0;
};
