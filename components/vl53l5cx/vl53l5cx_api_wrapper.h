#pragma once

#include "vl53l5cx_api.h"
#include "esphome/core/defines.h"  //NOLINT

#if defined(RUN_XTALK_CALIBRATION) || defined(SET_XTALK_CALIBRATION_DATA)
#include "vl53l5cx_plugin_xtalk.h"
#endif

#define VL53L5CX_NB_BYTES_PER_I2C_OP 128

namespace esphome::vl53l5cx {

static const auto WRAPPER_TAG = "vl53l5cx_api_wrapper";

class VL53L5CXApiWrapper {
 public:
  explicit VL53L5CXApiWrapper(i2c::I2CDevice *i2c_device) {
    this->i2c_device_ = i2c_device;
    this->config_.platform.reference_ = this;
    this->config_.platform.rd_byte_func = &rd_byte_func;
    this->config_.platform.wr_byte_func = &wr_byte_func;
    this->config_.platform.rd_bytes_func = &rd_bytes_func;
    this->config_.platform.wr_bytes_func = &wr_bytes_func;
    this->config_.platform.delay_func = &delay_func;
  }

  uint8_t is_alive(uint8_t *p_is_alive) { return vl53l5cx_is_alive(&this->config_, p_is_alive); }

  uint8_t set_i2c_address(const uint16_t i2c_address) { return vl53l5cx_set_i2c_address(&this->config_, i2c_address); }

  uint8_t init() { return vl53l5cx_init(&this->config_); }

  uint8_t check_data_ready(uint8_t *p_is_ready) { return vl53l5cx_check_data_ready(&this->config_, p_is_ready); }

  uint8_t get_ranging_data() { return vl53l5cx_get_ranging_data(&this->config_, &this->results_); }
  VL53L5CX_ResultsData *get_results() { return &this->results_; }

  uint8_t set_resolution(const uint8_t resolution) { return vl53l5cx_set_resolution(&this->config_, resolution); }

  uint8_t set_ranging_frequency_hz(const uint8_t frequency_hz) {
    return vl53l5cx_set_ranging_frequency_hz(&this->config_, frequency_hz);
  }

  uint8_t set_ranging_mode(const uint8_t ranging_mode) {
    return vl53l5cx_set_ranging_mode(&this->config_, ranging_mode);
  }

  uint8_t set_integration_time_ms(const uint32_t integration_time_ms) {
    return vl53l5cx_set_integration_time_ms(&this->config_, integration_time_ms);
  }

  uint8_t set_sharpener_percent(const uint8_t sharpener_percent) {
    return vl53l5cx_set_sharpener_percent(&this->config_, sharpener_percent);
  }

  uint8_t set_target_order(const uint8_t target_order) {
    return vl53l5cx_set_target_order(&this->config_, target_order);
  }

  uint8_t set_vhv_repeat_count(const uint32_t repeat_count) {
    return vl53l5cx_set_VHV_repeat_count(&this->config_, repeat_count);
  }

  uint8_t set_power_mode(const uint8_t power_mode) { return vl53l5cx_set_power_mode(&this->config_, power_mode); }

  uint8_t start_ranging() { return vl53l5cx_start_ranging(&this->config_); }

  uint8_t stop_ranging() { return vl53l5cx_stop_ranging(&this->config_); }

#if defined(RUN_XTALK_CALIBRATION) || defined(SET_XTALK_CALIBRATION_DATA)
  uint8_t calibrate_xtalk(uint16_t reflectance_percent, uint8_t nb_samples, uint16_t distance_mm) {
    return vl53l5cx_calibrate_xtalk(&this->config_, reflectance_percent, nb_samples, distance_mm);
  }

  uint8_t get_caldata_xtalk(uint8_t *p_xtalk_data) { return vl53l5cx_get_caldata_xtalk(&this->config_, p_xtalk_data); }

  uint8_t set_caldata_xtalk(uint8_t *p_xtalk_data) { return vl53l5cx_set_caldata_xtalk(&this->config_, p_xtalk_data); }
#endif

 protected:
  static uint8_t rd_byte_func(const VL53L5CX_FUNC_REF reference, const uint16_t register_address, uint8_t *p_value) {
    const auto *const wrapper = static_cast<VL53L5CXApiWrapper*>(reference);
    const i2c::ErrorCode status = wrapper->i2c_device_->read_register16(register_address, p_value, 1);
    /* This function returns 0 if OK */
    if (status) {
      ESP_LOGD(WRAPPER_TAG, "Failed to read: %x", status);
    }
    return static_cast<uint8_t>(status);
  }

  static uint8_t wr_byte_func(const VL53L5CX_FUNC_REF reference, const uint16_t register_address, const uint8_t value) {
    const auto *const wrapper = static_cast<VL53L5CXApiWrapper*>(reference);
    const i2c::ErrorCode status = wrapper->i2c_device_->write_register16(register_address, &value, 1);
    /* This function returns 0 if OK */
    if (status) {
      ESP_LOGD(WRAPPER_TAG, "Failed to write: %x", status);
    }
    return static_cast<uint8_t>(status);
  }

  static uint8_t rd_bytes_func(const VL53L5CX_FUNC_REF reference, const uint16_t register_address, uint8_t *p_values, const uint32_t size) {
    const auto *const wrapper = static_cast<VL53L5CXApiWrapper*>(reference);
    uint32_t rest;
    i2c::ErrorCode status;
    for (uint32_t i = 0; i < size; i += VL53L5CX_NB_BYTES_PER_I2C_OP) {
      rest = size - i;
      if (rest > VL53L5CX_NB_BYTES_PER_I2C_OP)
        rest = VL53L5CX_NB_BYTES_PER_I2C_OP;
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

  static uint8_t wr_bytes_func(const VL53L5CX_FUNC_REF reference, const uint16_t register_address, const uint8_t *p_values, const uint32_t size) {
    const auto *const wrapper = static_cast<VL53L5CXApiWrapper*>(reference);
    uint32_t rest;
    i2c::ErrorCode status;
    for (uint32_t i = 0; i < size; i += VL53L5CX_NB_BYTES_PER_I2C_OP) {
      rest = size - i;
      if (rest > VL53L5CX_NB_BYTES_PER_I2C_OP)
        rest = VL53L5CX_NB_BYTES_PER_I2C_OP;
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

  static void delay_func (uint32_t ms) {
    delay(ms);
    arch_feed_wdt();
  }

  VL53L5CX_Configuration config_{};
  VL53L5CX_ResultsData results_{};
  i2c::I2CDevice *i2c_device_{nullptr};
};

}  // namespace esphome::vl53l5cx
