#pragma once

#include "esphome/core/defines.h" //NOLINT
#include "esphome/components/i2c/i2c.h"
#include "vl53l_mz_const.h" //NOLINT

#define VL53L_MZ_NB_BYTES_PER_I2C_OP 128

namespace esphome::vl53l_mz {

static const auto WRAPPER_TAG = "vl53l_mz_api_wrapper";

typedef struct {
  int8_t silicon_temp_degc;
  uint8_t nb_target_per_zone;
  uint8_t *nb_target_detected;
  uint16_t *range_sigma_mm;
  int16_t *distance_mm;
  uint8_t *reflectance;
  uint8_t *target_status;
} VL53LMZ_ResultsData;

class VL53LMZApiWrapper {
 public:
  explicit VL53LMZApiWrapper(i2c::I2CDevice *i2c_device) {
    this->i2c_device_ = i2c_device;
  }

  virtual uint8_t is_alive(uint8_t *p_is_alive);
  virtual uint8_t set_i2c_address(uint16_t i2c_address);
  virtual uint8_t init();
  virtual uint8_t check_data_ready(uint8_t *p_is_ready);
  virtual uint8_t get_ranging_data();
  virtual uint8_t set_resolution(uint8_t resolution);
  virtual uint8_t set_ranging_frequency_hz(uint8_t frequency_hz);
  virtual uint8_t set_ranging_mode(uint8_t ranging_mode);
  virtual uint8_t set_integration_time_ms(uint32_t integration_time_ms);
  virtual uint8_t set_sharpener_percent(uint8_t sharpener_percent);
  virtual uint8_t set_target_order(uint8_t target_order) ;
  virtual uint8_t set_vhv_repeat_count(uint32_t repeat_count);
  virtual uint8_t set_power_mode_wakeup();
  virtual uint8_t set_power_mode_sleep();
  virtual uint8_t start_ranging();
  virtual uint8_t stop_ranging();
#if defined(VL53L_MZ_RUN_XTALK_CALIBRATION) || defined(VL53L_MZ_SET_XTALK_CALIBRATION_DATA)
  virtual uint8_t calibrate_xtalk(uint16_t reflectance_percent, uint8_t nb_samples, uint16_t distance_mm);
  virtual uint8_t get_caldata_xtalk(uint8_t *p_xtalk_data);
  virtual uint8_t set_caldata_xtalk(uint8_t *p_xtalk_data);
  virtual uint8_t *get_xtalk_buffer();
  virtual uint16_t get_xtalk_buffer_size();
#endif
  virtual uint8_t get_default_i2c_address();
  virtual uint8_t to_api_resolution(VL53LMZResolution resolution);
  virtual uint8_t to_api_ranging_mode(VL53LMZMode mode);
  virtual uint8_t to_api_target_order(VL53LMZTargetOrder target_order);
  VL53LMZ_ResultsData *get_results() {
    return &results_;
  }

 protected:
  static uint8_t rd_byte_func(void *reference, const uint16_t register_address, uint8_t *p_value) {
    const auto *const wrapper = static_cast<VL53LMZApiWrapper*>(reference);
    const i2c::ErrorCode status = wrapper->i2c_device_->read_register16(register_address, p_value, 1);
    /* This function returns 0 if OK */
    if (status) {
      ESP_LOGD(WRAPPER_TAG, "Failed to read: %x", status);
    }
    return static_cast<uint8_t>(status);
  }

  static uint8_t wr_byte_func(void *reference, const uint16_t register_address, const uint8_t value) {
    const auto *const wrapper = static_cast<VL53LMZApiWrapper*>(reference);
    const i2c::ErrorCode status = wrapper->i2c_device_->write_register16(register_address, &value, 1);
    /* This function returns 0 if OK */
    if (status) {
      ESP_LOGD(WRAPPER_TAG, "Failed to write: %x", status);
    }
    return static_cast<uint8_t>(status);
  }

  static uint8_t rd_bytes_func(void *reference, const uint16_t register_address, uint8_t *p_values, const uint32_t size) {
    const auto *const wrapper = static_cast<VL53LMZApiWrapper*>(reference);
    i2c::ErrorCode status = i2c::ErrorCode::ERROR_UNKNOWN;
    for (uint32_t i = 0; i < size; i += VL53L_MZ_NB_BYTES_PER_I2C_OP) {
      uint32_t rest = size - i;
      if (rest > VL53L_MZ_NB_BYTES_PER_I2C_OP)
        rest = VL53L_MZ_NB_BYTES_PER_I2C_OP;
      status = wrapper->i2c_device_->read_register16(register_address + i, p_values + i, rest);
      /* This function returns 0 if OK */
      if (status) {
        ESP_LOGD(WRAPPER_TAG, "Failed to read multiple: %x", status);
        break;
      }
      arch_feed_wdt();
    }
    return static_cast<uint8_t>(status);
  }

  static uint8_t wr_bytes_func(void *reference, const uint16_t register_address, const uint8_t *p_values, const uint32_t size) {
    const auto *const wrapper = static_cast<VL53LMZApiWrapper*>(reference);
    i2c::ErrorCode status = i2c::ErrorCode::ERROR_UNKNOWN;
    for (uint32_t i = 0; i < size; i += VL53L_MZ_NB_BYTES_PER_I2C_OP) {
      uint32_t rest = size - i;
      if (rest > VL53L_MZ_NB_BYTES_PER_I2C_OP)
        rest = VL53L_MZ_NB_BYTES_PER_I2C_OP;
      status = wrapper->i2c_device_->write_register16(register_address + i, p_values + i, rest);
      /* This function returns 0 if OK */
      if (status) {
        ESP_LOGD(WRAPPER_TAG, "Failed to write multiple: %x", status);
        break;
      }
      arch_feed_wdt();
    }
    return static_cast<uint8_t>(status);
  }

  static void delay_func (const uint32_t ms) {
    delay(ms);
    arch_feed_wdt();
  }

  i2c::I2CDevice *i2c_device_{nullptr};
  VL53LMZ_ResultsData results_{};
};
}  // namespace esphome::vl53l_mz
