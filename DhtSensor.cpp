#include "DhtSensor.h"

DhtSensor::DhtSensor(uint8_t pin, uint8_t type) : _dht(pin, type) {}

bool DhtSensor::begin() {
  _dht.begin();
  return true;  // DHT has no handshake; failures show up as NaN readings
}

ClimateReading DhtSensor::read() {
  ClimateReading r;
  r.humidity     = _dht.readHumidity();
  r.temperatureC = _dht.readTemperature();
  r.valid        = !isnan(r.humidity) && !isnan(r.temperatureC);
  return r;
}
