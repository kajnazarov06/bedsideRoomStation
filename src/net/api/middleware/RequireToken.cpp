#include "RequireToken.h"

bool RequireToken::handle(HttpRequest& request, HttpResponse& response) {
  if (_auth.verifyToken(request.bearerToken())) return true;

  response.error(401, "unauthorized");
  return false;        
}
