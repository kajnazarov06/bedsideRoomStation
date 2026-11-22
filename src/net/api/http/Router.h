#pragma once
#include <functional>
#include <initializer_list>
#include <WebServer.h>
#include "HttpRequest.h"
#include "HttpResponse.h"
#include "IMiddleware.h"

// Maps method + path to a handler, running the route's middleware first.
//
//   router.get("/api/state", Router::action(controller, &StateController::show), { &requireToken });
class Router {
public:
  using Handler = std::function<void(HttpRequest&, HttpResponse&)>;

  explicit Router(WebServer& server) : _server(server) {}

  void get (const char* path, Handler handler, std::initializer_list<IMiddleware*> middleware = {});
  void post(const char* path, Handler handler, std::initializer_list<IMiddleware*> middleware = {});
  void fallback(Handler handler);  // no route matched

  // Turns a controller method into a handler: Router::action(controller, &Controller::method).
  template <typename Controller>
  static Handler action(Controller& controller,
                        void (Controller::*method)(HttpRequest&, HttpResponse&)) {
    return [&controller, method](HttpRequest& request, HttpResponse& response) {
      (controller.*method)(request, response);
    };
  }

private:
  void add(HTTPMethod method, const char* path, Handler handler,
           std::initializer_list<IMiddleware*> middleware);

  WebServer& _server;
};
