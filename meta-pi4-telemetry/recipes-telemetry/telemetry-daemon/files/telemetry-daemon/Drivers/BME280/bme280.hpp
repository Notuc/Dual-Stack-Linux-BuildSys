#ifndef BME280_HPP
#define BME280_HPP

#include <cstdint>
#include <string>
#include <tuple>

// PI5/4 C++ port  STM32F4 (HAL-based) implementation.
// Reference: Bosch BME280 datasheet (register map, compensation formulas)

class BME280 {
public:
  //  I used i2cdetect -y <bus>` to confirm which address the BME280 was at
  static constexpr uint8_t ADDR_PRIMARY = 0x77;   // SDO -> VDDIO
  static constexpr uint8_t ADDR_SECONDARY = 0x76; // SDO -> GND

  // Output data
  // returned by readAll().
  struct Telemetry {
    float temperature_degC;
    float pressure_hPa;
    float humidity_rh;
  };

  // Factory calibration coefficients
  // Each BME280 unit is factory-trimmed; these coefficients are burned into the
  // sensor's NVM The raw byte packing in parseCalibration() depends on this
  // exact layout.
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

  // Operation result codes & Equivalent of HAL_StatusTypeDef
  enum class Status {
    OK,           // Success
    OPEN_FAILED,  // open() on the I2C bus device node failed
    IOCTL_FAILED, // ioctl(I2C_SLAVE, addr) failed  (rare; usually means addr is
                  // invalid or the fd is bad)
    IO_ERROR,   // An SMBus read/write transaction failed or returned a // short
                // read (device didn't ACK, wrong address, wiring fault, etc.
    BAD_CHIP_ID // Device responded, but WHOAMI/chip-id register didn't  match
                // the expected BME280 value (wrong sensor, or corrupted
                // transaction)
  };

  BME280() = default;

  BME280(const char *bus_path, uint8_t addr = ADDR_PRIMARY);

  ~BME280();

  // Opens (if not already open) the I2C bus device, binds the sensor
  // address, verifies the chip ID, reads + parses factory calibration
  // data, and configures the sensor into normal measurement mode.
  // Must be called (and must return Status::OK) before readAll().
  Status init(const char *bus_path = "/dev/i2c-1", uint8_t addr = ADDR_PRIMARY);

  // Performs a burst read of the sensor's raw data registers (0xF7-0xFE:
  // pressure, temperature, humidity — 8 bytes total) and applies the
  // Bosch compensation formulas to produce physical units.
  // Must be called only after a successful init().

  std::tuple<Status, Telemetry> readAll();

private:
  // --- BME280 register map
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

  int m_fd{-1};                 // Open file descriptor for the I2C bus device
                                // node (e.g. /dev/i2c-1). -1 = not open.
                                // Replaces the STM32 I2C_HandleTypeDef*.
  uint8_t m_addr{ADDR_PRIMARY}; // 7-bit I2C address currently bound via
                                // ioctl(I2C_SLAVE, ...).
  CalibrationData m_calib{};    // Factory calibration coefficients, populated
                                // by init() -> parseCalibration().
  int32_t m_tFine{0};           // Intermediate "fine temperature" value shared
                                // between the temperature, pressure, and
                                // humidity compensation formulas (per Bosch
                                // datasheet — pressure/humidity math depends
                                // on the temperature compensation having run
                                // first within the same readAll() call).

  // Low-level transport helpers
  Status memRead(uint8_t reg, uint8_t *buf, uint8_t len);

  // Writes a single byte `val` to register `reg`. Equivalent to
  // HAL_I2C_Mem_Write(hi2c, addr, reg, 1, &val, 1, HAL_MAX_DELAY).
  Status memWrite(uint8_t reg, uint8_t val);

  // Unpacks the two raw calibration byte blocks (26 bytes from 0x88,
  // 7 bytes from 0xE1) read during init() into the typed CalibrationData
  // struct, per the byte layout defined in the Bosch datasheet.
  void parseCalibration(const uint8_t *c1, const uint8_t *c2);
};

#endif
