#pragma once
#include <DHT.h>
#include "../core/IComponent.h"

struct ClimateReading {
  float temperatureC;
  float humidity;
  bool  valid;
};

class DhtSensor : public IComponent {
public:
  DhtSensor(uint8_t pin, uint8_t type = DHT11);

  bool begin() override;
  const char* name() const override { return "DHT11"; }

  ClimateReading read();

private:
  DHT _dht;
};
