#include "Router.h"
#include <vector>
#include <uri/UriBraces.h>

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

  if (strchr(path, '{')) _server.on(UriBraces(path), method, route);
  else                   _server.on(path, method, route);
}
