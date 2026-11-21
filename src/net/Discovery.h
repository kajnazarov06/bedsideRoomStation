#pragma once
#include "../core/IComponent.h"

// Advertises the station on the local network via mDNS, so apps can find it
// as "_smartclock._tcp" (and reach it at http://<id>.local).
class Discovery : public IComponent {
public:
  explicit Discovery(uint16_t port) : _port(port) {}

  bool begin() override;
  const char* name() const override { return "mDNS"; }

private:
  uint16_t _port;
};
