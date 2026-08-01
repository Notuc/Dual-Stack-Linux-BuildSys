#include "bme280.hpp"
#include <cerrno>
#include <cstdio>
#include <cstring>
#include <fcntl.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <unistd.h>
extern "C" {
#include <i2c/smbus.h>
}

BME280::BME280(const char *bus_path, uint8_t addr) : m_addr(addr) {
  m_fd = open(bus_path, O_RDWR);
  if (m_fd >= 0) {
    ioctl(m_fd, I2C_SLAVE, m_addr);
  }
}

BME280::~BME280() {
  if (m_fd >= 0)
    close(m_fd);
}

void BME280::parseCalibration(const uint8_t *c1, const uint8_t *c2) {
  // --- UNCHANGED from your STM32 version, byte-order math is
  // platform-independent ---
  m_calib.dig_T1 = static_cast<uint16_t>(c1[0] | (c1[1] << 8));
  m_calib.dig_T2 = static_cast<int16_t>(c1[2] | (c1[3] << 8));
  m_calib.dig_T3 = static_cast<int16_t>(c1[4] | (c1[5] << 8));

  m_calib.dig_P1 = static_cast<uint16_t>(c1[6] | (c1[7] << 8));
  m_calib.dig_P2 = static_cast<int16_t>(c1[8] | (c1[9] << 8));
  m_calib.dig_P3 = static_cast<int16_t>(c1[10] | (c1[11] << 8));
  m_calib.dig_P4 = static_cast<int16_t>(c1[12] | (c1[13] << 8));
  m_calib.dig_P5 = static_cast<int16_t>(c1[14] | (c1[15] << 8));
  m_calib.dig_P6 = static_cast<int16_t>(c1[16] | (c1[17] << 8));
  m_calib.dig_P7 = static_cast<int16_t>(c1[18] | (c1[19] << 8));
  m_calib.dig_P8 = static_cast<int16_t>(c1[20] | (c1[21] << 8));
  m_calib.dig_P9 = static_cast<int16_t>(c1[22] | (c1[23] << 8));

  m_calib.dig_H1 = c1[25];

  m_calib.dig_H2 = static_cast<int16_t>(c2[0] | (c2[1] << 8));
  m_calib.dig_H3 = c2[2];
  m_calib.dig_H4 = static_cast<int16_t>((c2[3] << 4) | (c2[4] & 0x0F));
  m_calib.dig_H5 = static_cast<int16_t>((c2[5] << 4) | (c2[4] >> 4));
  m_calib.dig_H6 = static_cast<int8_t>(c2[6]);
}

// Transport helpers replacing HAL_I2C_Mem_Read
// HAL_I2C_Mem_Read(hi2c, addr, reg, 1, buf, len, timeout)
//   becomes
// i2c_smbus_read_i2c_block_data(fd, reg, len, buf)
//

BME280::Status BME280::memRead(uint8_t reg, uint8_t *buf, uint8_t len) {
  errno = 0;
  int n = i2c_smbus_read_i2c_block_data(m_fd, reg, len, buf);
  if (n < 0) {
    fprintf(stderr, "memRead failed: fd=%d reg=0x%02X len=%d errno=%d (%s)\n",
            m_fd, reg, len, errno, strerror(errno));
    return Status::IO_ERROR;
  }
  if (n != len) {
    fprintf(stderr, "memRead short: reg=0x%02X expected=%d got=%d\n", reg, len,
            n);
    return Status::IO_ERROR;
  }
  return Status::OK;
}

BME280::Status BME280::memWrite(uint8_t reg, uint8_t val) {
  if (i2c_smbus_write_byte_data(m_fd, reg, val) < 0)
    return Status::IO_ERROR;
  return Status::OK;
}

BME280::Status BME280::init(const char *bus_path, uint8_t addr) {
  m_addr = addr;

  // (Re)open in case init() is called after a default construction
  if (m_fd < 0) {
    m_fd = open(bus_path, O_RDWR);
    if (m_fd < 0)
      return Status::OPEN_FAILED;
  }
  if (ioctl(m_fd, I2C_SLAVE, m_addr) < 0) {
    fprintf(stderr, "ioctl(I2C_SLAVE) failed: errno=%d (%s)\n", errno,
            strerror(errno));
    return Status::IOCTL_FAILED;
  }
  fprintf(stderr, "ioctl OK: fd=%d addr=0x%02X\n", m_fd, m_addr);

  Status status;
  uint8_t chip_id = 0;
  uint8_t calib1[26];
  uint8_t calib2[7];

  // Confirm device identity
  status = memRead(REG_CHIP_ID, &chip_id, 1);
  if (status != Status::OK)
    return status;
  if (chip_id != CHIP_ID_VALUE)
    return Status::BAD_CHIP_ID;

  // Read calibration registers
  status = memRead(REG_CALIB00, calib1, 26);
  if (status != Status::OK)
    return status;

  status = memRead(REG_CALIB26, calib2, 7);
  if (status != Status::OK)
    return status;

  parseCalibration(calib1, calib2);

  // Configure humidity oversampling
  uint8_t data = 0x01; // osrs_h x1
  status = memWrite(REG_CTRL_HUM, data);
  if (status != Status::OK)
    return status;

  // Configure temperature/pressure oversampling
  data = (0b001 << 5) | (0b001 << 2) | 0b11;
  status = memWrite(REG_CTRL_MEAS, data);

  return status;
}

std::tuple<BME280::Status, BME280::Telemetry> BME280::readAll() {
  uint8_t raw[8];
  Telemetry data{};

  Status status = memRead(REG_DATA_START, raw, 8);
  if (status != Status::OK) {
    return {status, data};
  }

  // Compensation math is pure integer/float arithmetic

  int32_t adc_P = (static_cast<int32_t>(raw[0]) << 12) |
                  (static_cast<int32_t>(raw[1]) << 4) | (raw[2] >> 4);
  int32_t adc_T = (static_cast<int32_t>(raw[3]) << 12) |
                  (static_cast<int32_t>(raw[4]) << 4) | (raw[5] >> 4);
  int32_t adc_H = (static_cast<int32_t>(raw[6]) << 8) | raw[7];

  // Temperature
  int32_t var1 =
      ((((adc_T >> 3) - (static_cast<int32_t>(m_calib.dig_T1) << 1))) *
       static_cast<int32_t>(m_calib.dig_T2)) >>
      11;
  int32_t var2 = (((((adc_T >> 4) - static_cast<int32_t>(m_calib.dig_T1)) *
                    ((adc_T >> 4) - static_cast<int32_t>(m_calib.dig_T1))) >>
                   12) *
                  static_cast<int32_t>(m_calib.dig_T3)) >>
                 14;
  m_tFine = var1 + var2;
  int32_t T = (m_tFine * 5 + 128) >> 8;
  data.temperature_degC = static_cast<float>(T) / 100.0f;

  // Pressure

  int64_t p_var1 = static_cast<int64_t>(m_tFine) - 128000;
  int64_t p_var2 = p_var1 * p_var1 * static_cast<int64_t>(m_calib.dig_P6);
  p_var2 = p_var2 + ((p_var1 * static_cast<int64_t>(m_calib.dig_P5)) << 17);
  p_var2 = p_var2 + (static_cast<int64_t>(m_calib.dig_P4) << 35);
  p_var1 = ((p_var1 * p_var1 * static_cast<int64_t>(m_calib.dig_P3)) >> 8) +
           ((p_var1 * static_cast<int64_t>(m_calib.dig_P2)) << 12);
  p_var1 = (((static_cast<int64_t>(1) << 47) + p_var1)) *
               static_cast<int64_t>(m_calib.dig_P1) >>
           33;

  if (p_var1 == 0) {
    data.pressure_hPa = 0.0f;
  } else {
    int64_t p = 1048576 - adc_P;
    p = (((p << 31) - p_var2) * 3125) / p_var1;
    p_var1 =
        (static_cast<int64_t>(m_calib.dig_P9) * (p >> 13) * (p >> 13)) >> 25;
    p_var2 = (static_cast<int64_t>(m_calib.dig_P8) * p) >> 19;
    p = ((p + p_var1 + p_var2) >> 8) +
        (static_cast<int64_t>(m_calib.dig_P7) << 4);
    data.pressure_hPa = (static_cast<float>(p) / 256.0f) / 100.0f;
  }

  // Humidity
  int32_t v_x1_u32r = m_tFine - 76800;
  v_x1_u32r =
      (((((adc_H << 14) - (static_cast<int32_t>(m_calib.dig_H4) << 20) -
          (static_cast<int32_t>(m_calib.dig_H5) * v_x1_u32r)) +
         16384) >>
        15) *
       (((((((v_x1_u32r * static_cast<int32_t>(m_calib.dig_H6)) >> 10) *
            (((v_x1_u32r * static_cast<int32_t>(m_calib.dig_H3)) >> 11) +
             32768)) >>
           10) +
          2097152) *
             static_cast<int32_t>(m_calib.dig_H2) +
         8192) >>
        14));
  v_x1_u32r = (v_x1_u32r - (((((v_x1_u32r >> 15) * (v_x1_u32r >> 15)) >> 7) *
                             static_cast<int32_t>(m_calib.dig_H1)) >>
                            4));
  v_x1_u32r = (v_x1_u32r < 0) ? 0 : v_x1_u32r;
  v_x1_u32r = (v_x1_u32r > 419430400) ? 419430400 : v_x1_u32r;
  uint32_t H = static_cast<uint32_t>(v_x1_u32r >> 12);
  data.humidity_rh = static_cast<float>(H) / 1024.0f;

  return {Status::OK, data};
}
