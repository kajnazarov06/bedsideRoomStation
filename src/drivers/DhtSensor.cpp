#include "DhtSensor.h"

DhtSensor::DhtSensor(uint8_t pin, uint8_t type) : _dht(pin, type) {}

bool DhtSensor::begin() {
  _dht.begin();
  return true;  // DHT has no handshake; failures show up as NaN readings
}

ClimateReading DhtSensor::read() {
  ClimateReading reading;
  reading.humidity     = _dht.readHumidity();
  reading.temperatureC = _dht.readTemperature();
  reading.valid        = !isnan(reading.humidity) && !isnan(reading.temperatureC);
  return reading;
}
