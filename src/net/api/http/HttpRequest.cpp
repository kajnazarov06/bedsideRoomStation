#include "HttpRequest.h"

namespace {
  constexpr const char* BEARER_PREFIX = "Bearer ";
}

const char* HttpRequest::method() const {
  switch (_server.method()) {
    case HTTP_GET:    return "GET";
    case HTTP_POST:   return "POST";
    case HTTP_PUT:    return "PUT";
    case HTTP_PATCH:  return "PATCH";
    case HTTP_DELETE: return "DELETE";
    default:          return "OTHER";
  }
}

String HttpRequest::bearerToken() const {
  const String authorization = header("Authorization");
  if (!authorization.startsWith(BEARER_PREFIX)) return String();
  return authorization.substring(strlen(BEARER_PREFIX));
}

JsonDocument HttpRequest::json() const {
  JsonDocument document;
  deserializeJson(document, _server.arg("plain"));
  return document;
}
