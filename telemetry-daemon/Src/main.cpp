#include "bme280.hpp"
#include "can_interface.hpp"
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

// CAN message IDs for our telemetry data
static constexpr canid_t CAN_ID_TEMPERATURE = 0x100;
static constexpr canid_t CAN_ID_PRESSURE = 0x101;
static constexpr canid_t CAN_ID_HUMIDITY = 0x102;

// Unix socket path — logging service listens here
static constexpr const char *SOCKET_PATH = "/var/run/telemetry.sock";

// Pack a float into 4 bytes for CAN frame payload
static void packFloat(float val, uint8_t *buf) {
  memcpy(buf, &val, sizeof(float));
}

// Connect to the logging service Unix socket
// Returns socket fd or -1 on failure
static int connectToLogger() {
  int fd = socket(AF_UNIX, SOCK_STREAM, 0);
  if (fd < 0)
    return -1;

  sockaddr_un addr{};
  addr.sun_family = AF_UNIX;
  strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path) - 1);

  if (connect(fd, reinterpret_cast<sockaddr *>(&addr), sizeof(addr)) < 0) {
    close(fd);
    return -1;
  }
  return fd;
}

int main() {
  // Init BME280
  BME280 sensor;
  auto status = sensor.init("/dev/i2c-1", BME280::ADDR_PRIMARY);
  if (status != BME280::Status::OK) {
    fprintf(stderr, "BME280 init failed: %d\n", static_cast<int>(status));
    return 1;
  }

  // Init CAN
  CanInterface can("can0");

  // Connect to logging service
  int logger_fd = connectToLogger();
  if (logger_fd < 0) {
    fprintf(stderr,
            "Warning: could not connect to logger, continuing without it\n");
  }

  uint8_t payload[4];
  char log_line[128];

  while (true) {
    auto [st, data] = sensor.readAll();
    if (st != BME280::Status::OK) {
      fprintf(stderr, "BME280 read failed\n");
      sleep(1);
      continue;
    }

    // Print locally
    printf("T=%.2fC P=%.2fhPa H=%.2f%%\n", data.temperature_degC,
           data.pressure_hPa, data.humidity_rh);

    // Send temperature over CAN
    packFloat(data.temperature_degC, payload);
    can.sendFrame(CAN_ID_TEMPERATURE, payload, 4);

    // Send pressure over CAN
    packFloat(data.pressure_hPa, payload);
    can.sendFrame(CAN_ID_PRESSURE, payload, 4);

    // Send humidity over CAN
    packFloat(data.humidity_rh, payload);
    can.sendFrame(CAN_ID_HUMIDITY, payload, 4);

    // Send to logging service over Unix socket
    if (logger_fd >= 0) {
      int len =
          snprintf(log_line, sizeof(log_line), "T=%.2f P=%.2f H=%.2f\n",
                   data.temperature_degC, data.pressure_hPa, data.humidity_rh);
      if (write(logger_fd, log_line, len) < 0) {
        // Logger disconnected — try to reconnect next iteration
        close(logger_fd);
        logger_fd = connectToLogger();
      }
    }

    sleep(1);
  }
}
