#pragma once

#include "vl53l_mz_api_wrapper.h"

#ifdef VL53L_MZ_USE_VL53L7CX
#include "vl53l7cx_api.h"

#if defined(VL53L_MZ_RUN_XTALK_CALIBRATION) || defined(VL53L_MZ_SET_XTALK_CALIBRATION_DATA)
#include "vl53l7cx_plugin_xtalk.h"
#endif

namespace esphome::vl53l_mz {

#if defined(VL53L_MZ_RUN_XTALK_CALIBRATION) || defined(VL53L_MZ_SET_XTALK_CALIBRATION_DATA)
static uint8_t vl53l7cx_xtalk_buffer[VL53L7CX_XTALK_BUFFER_SIZE];
#endif

class VL53L7CXApiWrapper : public VL53LMZApiWrapper{
public:
  explicit VL53L7CXApiWrapper(i2c::I2CDevice *i2c_device) : VL53LMZApiWrapper(i2c_device) {
    this->config_.platform.reference_ = this;
    this->config_.platform.rd_byte_func = &rd_byte_func;
    this->config_.platform.wr_byte_func = &wr_byte_func;
    this->config_.platform.rd_bytes_func = &rd_bytes_func;
    this->config_.platform.wr_bytes_func = &wr_bytes_func;
    this->config_.platform.delay_func = &delay_func;
  }

  uint8_t is_alive(uint8_t *p_is_alive) override { return vl53l7cx_is_alive(&this->config_, p_is_alive); }

  uint8_t set_i2c_address(const uint16_t i2c_address) override { return vl53l7cx_set_i2c_address(&this->config_, i2c_address); }

  uint8_t init() override { return vl53l7cx_init(&this->config_); }

  uint8_t check_data_ready(uint8_t *p_is_ready) override { return vl53l7cx_check_data_ready(&this->config_, p_is_ready); }

  uint8_t get_ranging_data() override {
    uint8_t status = vl53l7cx_get_ranging_data(&this->config_, &this->api_results_);
    this->results_.silicon_temp_degc = this->api_results_.silicon_temp_degc;
    this->results_.nb_target_per_zone = VL53L7CX_NB_TARGET_PER_ZONE;
    this->results_.nb_target_detected = this->api_results_.nb_target_detected;
    this->results_.range_sigma_mm = this->api_results_.range_sigma_mm;
    this->results_.distance_mm = this->api_results_.distance_mm;
    this->results_.reflectance = this->api_results_.reflectance;
    this->results_.target_status = this->api_results_.target_status;
    return status;
  }

  uint8_t set_resolution(const uint8_t resolution) override {
    if (resolution == VL53LMZ_INVALID_OPTION) return VL53L7CX_STATUS_ERROR;
    return vl53l7cx_set_resolution(&this->config_, resolution);
  }

  uint8_t set_ranging_frequency_hz(const uint8_t frequency_hz) override {
    return vl53l7cx_set_ranging_frequency_hz(&this->config_, frequency_hz);
  }

  uint8_t set_ranging_mode(const uint8_t ranging_mode) override {
    if (ranging_mode == VL53LMZ_INVALID_OPTION) return VL53L7CX_STATUS_ERROR;
    return vl53l7cx_set_ranging_mode(&this->config_, ranging_mode);
  }

  uint8_t set_integration_time_ms(const uint32_t integration_time_ms) override {
    return vl53l7cx_set_integration_time_ms(&this->config_, integration_time_ms);
  }

  uint8_t set_sharpener_percent(const uint8_t sharpener_percent) override {
    return vl53l7cx_set_sharpener_percent(&this->config_, sharpener_percent);
  }

  uint8_t set_target_order(const uint8_t target_order) override {
    if (target_order == VL53LMZ_INVALID_OPTION) return VL53L7CX_STATUS_ERROR;
    return vl53l7cx_set_target_order(&this->config_, target_order);
  }

  uint8_t set_vhv_repeat_count(const uint32_t repeat_count) override {
    return vl53l7cx_set_VHV_repeat_count(&this->config_, repeat_count);
  }

  uint8_t set_power_mode_wakeup() override {
    return vl53l7cx_set_power_mode(&this->config_, VL53L7CX_POWER_MODE_WAKEUP);
  }

  uint8_t set_power_mode_sleep() override {
    return vl53l7cx_set_power_mode(&this->config_, VL53L7CX_POWER_MODE_SLEEP);
  }

  uint8_t start_ranging() override { return vl53l7cx_start_ranging(&this->config_); }

  uint8_t stop_ranging() override { return vl53l7cx_stop_ranging(&this->config_); }

#if defined(VL53L_MZ_RUN_XTALK_CALIBRATION) || defined(VL53L_MZ_SET_XTALK_CALIBRATION_DATA)
  uint8_t calibrate_xtalk(uint16_t reflectance_percent, uint8_t nb_samples, uint16_t distance_mm) override {
    return vl53l7cx_calibrate_xtalk(&this->config_, reflectance_percent, nb_samples, distance_mm);
  }

  uint8_t get_caldata_xtalk(uint8_t *p_xtalk_data) override { return vl53l7cx_get_caldata_xtalk(&this->config_, p_xtalk_data); }

  uint8_t set_caldata_xtalk(uint8_t *p_xtalk_data) override { return vl53l7cx_set_caldata_xtalk(&this->config_, p_xtalk_data); }

  uint8_t *get_xtalk_buffer() override { return vl53l7cx_xtalk_buffer; }

  uint16_t get_xtalk_buffer_size() {return VL53L7CX_XTALK_BUFFER_SIZE; }
#endif

  uint8_t get_default_i2c_address() {
    return VL53L7CX_DEFAULT_I2C_ADDRESS >> 1;
  }

  uint8_t to_api_resolution(VL53LMZResolution resolution) {
    if (resolution == VL53LMZ_4X4) {
      return VL53L7CX_RESOLUTION_4X4;
    } else if (resolution == VL53LMZ_8X8) {
      return VL53L7CX_RESOLUTION_8X8;
    } else {
      return VL53LMZ_INVALID_OPTION;
    }
  }

  virtual uint8_t to_api_ranging_mode(VL53LMZMode mode) {
    if (mode == VL53LMZ_AUTO) {
      return VL53L7CX_RANGING_MODE_AUTONOMOUS;
    } else if (mode == VL53LMZ_CONTINUOUS) {
      return VL53L7CX_RANGING_MODE_CONTINUOUS;
    } else {
      return VL53LMZ_INVALID_OPTION;
    }
  }

  virtual uint8_t to_api_target_order(VL53LMZTargetOrder target_order) {
    if (target_order == VL53LMZ_STRONGEST) {
      return VL53L7CX_TARGET_ORDER_STRONGEST;
    } else if (target_order == VL53LMZ_CLOSEST) {
      return VL53L7CX_TARGET_ORDER_CLOSEST;
    } else {
      return VL53LMZ_INVALID_OPTION;
    }
  }

protected:
  VL53L7CX_Configuration config_{};
  VL53L7CX_ResultsData api_results_{};
};

}  // namespace esphome::vl53l_mz
#endif
