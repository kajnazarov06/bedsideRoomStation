#include "Router.h"
#include <vector>
#include <uri/UriBraces.h>

namespace {
  // "/api/alarms/{id}" -> "/api/alarms/{}". The ESP32 UriBraces matcher only
  // understands empty braces; named ones never match.
  String bracePattern(const char* path) {
    String pattern;
    bool insideBraces = false;
    for (const char* character = path; *character != '\0'; character++) {
      if (*character == '{')      { insideBraces = true;  pattern += "{}"; }
      else if (*character == '}') { insideBraces = false; }
      else if (!insideBraces)     { pattern += *character; }
    }
    return pattern;
  }
}

void Router::get(const char* path, Handler handler, std::initializer_list<IMiddleware*> middleware) {
  add(HTTP_GET, path, handler, middleware);
}

void Router::post(const char* path, Handler handler, std::initializer_list<IMiddleware*> middleware) {
  add(HTTP_POST, path, handler, middleware);
}

void Router::put(const char* path, Handler handler, std::initializer_list<IMiddleware*> middleware) {
  add(HTTP_PUT, path, handler, middleware);
}

void Router::fallback(Handler handler) {
  _server.onNotFound([this, handler] {
    HttpRequest  request(_server);
    HttpResponse response(_server);
    handler(request, response);
  });
}

void Router::add(HTTPMethod method, const char* path, Handler handler,
                 std::initializer_list<IMiddleware*> middleware) {
  std::vector<IMiddleware*> pipeline(middleware);
  auto route = [this, handler, pipeline] {
    HttpRequest  request(_server);
    HttpResponse response(_server);
    for (IMiddleware* step : pipeline) {
      if (!step->handle(request, response)) return;
    }
    handler(request, response);
  };

  if (strchr(path, '{')) _server.on(UriBraces(bracePattern(path)), method, route);
  else                   _server.on(path, method, route);
}
