#include "bme280.hpp"
#include <cstdio>
#include <unistd.h>

int main() {
  BME280 sensor;
  auto status = sensor.init("/dev/i2c-1", BME280::ADDR_PRIMARY);
  if (status != BME280::Status::OK) {
    fprintf(stderr, "init failed: %d\n", static_cast<int>(status));
    return 1;
  }

  while (true) {
    auto [st, data] = sensor.readAll();
    if (st == BME280::Status::OK) {
      printf("T=%.2fC P=%.2fhPa H=%.2f%%\n", data.temperature_degC,
             data.pressure_hPa, data.humidity_rh);
    }
    sleep(1);
  }
}
