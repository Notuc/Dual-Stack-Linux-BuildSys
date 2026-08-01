#ifndef BME280_HPP
#define BME280_HPP

#include <cstdint>
#include <string>
#include <tuple>

// From Bosch Datasheet
// Cpp implementation from my earlier c STM32f4 implementation
// with the PI5 in mind.

class BME280 {
public:
  static constexpr uint8_t ADDR_PRIMARY = 0x77;   // SDO -> VDDIO
  static constexpr uint8_t ADDR_SECONDARY = 0x76; // SDO -> GND

  struct Telemetry {
    float temperature_degC;
    float pressure_hPa;
    float humidity_rh;
  };

  struct CalibrationData {
    uint16_t dig_T1;
    int16_t dig_T2, dig_T3;
    uint16_t dig_P1;
    int16_t dig_P2, dig_P3, dig_P4, dig_P5, dig_P6, dig_P7, dig_P8, dig_P9;
    uint8_t dig_H1;
    int16_t dig_H2;
    uint8_t dig_H3;
    int16_t dig_H4, dig_H5;
    int8_t dig_H6;
  };

  enum class Status { OK, OPEN_FAILED, IOCTL_FAILED, IO_ERROR, BAD_CHIP_ID };

  BME280() = default;
  BME280(const char *bus_path, uint8_t addr = ADDR_PRIMARY);
  ~BME280();

  // No handle to pass in on Linux — just the bus device path + addr
  Status init(const char *bus_path = "/dev/i2c-1", uint8_t addr = ADDR_PRIMARY);

  std::tuple<Status, Telemetry> readAll();

private:
  static constexpr uint8_t REG_CHIP_ID = 0xD0;
  static constexpr uint8_t REG_RESET = 0xE0;
  static constexpr uint8_t REG_CTRL_HUM = 0xF2;
  static constexpr uint8_t REG_STATUS = 0xF3;
  static constexpr uint8_t REG_CTRL_MEAS = 0xF4;
  static constexpr uint8_t REG_CONFIG = 0xF5;
  static constexpr uint8_t REG_DATA_START = 0xF7;
  static constexpr uint8_t REG_CALIB00 = 0x88;
  static constexpr uint8_t REG_CALIB26 = 0xE1;
  static constexpr uint8_t CHIP_ID_VALUE = 0x60;

  int m_fd{-1}; // replaces I2C_HandleTypeDef*
  uint8_t m_addr{ADDR_PRIMARY};
  CalibrationData m_calib{};
  int32_t m_tFine{0};

  // Low-level transport helpers — this is the layer that used to be HAL calls
  bool writeReg(uint8_t reg, uint8_t val);
  bool readRegs(uint8_t reg, uint8_t *buf, size_t len);
  Status memRead(uint8_t reg, uint8_t *buf, uint8_t len);
  Status memWrite(uint8_t reg, uint8_t val);
  void parseCalibration(const uint8_t *c1, const uint8_t *c2);
};

#endif
