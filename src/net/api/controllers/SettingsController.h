#pragma once
#include "../http/HttpRequest.h"
#include "../http/HttpResponse.h"
#include "../../../core/ITextDisplay.h"
#include "../../../storage/SettingsStore.h"

// GET /api/settings  {"name":"Bedroom clock", "tz":"Africa/Abidjan", "brightness":50, "snooze_min":5}
// PUT /api/settings  body: any of {"name", "tz", "brightness", "snooze_min"}
//                    -> the updated settings; 400 bad JSON, 422 bad value
class SettingsController {
public:
  SettingsController(SettingsStore& store, ITextDisplay& display)
    : _store(store), _display(display) {}

  void show(HttpRequest& request, HttpResponse& response);
  void update(HttpRequest& request, HttpResponse& response);

private:
  SettingsStore& _store;
  ITextDisplay&  _display;
};
