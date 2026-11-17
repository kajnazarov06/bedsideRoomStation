#pragma once
#include <Arduino.h>

// Every hardware module can be initialised and identified.
class IComponent {
public:
  virtual ~IComponent() = default;
  virtual bool begin() = 0;
  virtual const char* name() const = 0;
};
