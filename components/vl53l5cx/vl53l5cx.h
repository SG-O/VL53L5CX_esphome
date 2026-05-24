#pragma once

#include <list>

#include "esphome/core/component.h"
#include "esphome/core/defines.h"  //NOLINT
#include "esphome/core/hal.h"
#include "esphome/components/i2c/i2c.h"
#include "vl53l5cx_child.h"
#include "vl53l5cx_api.h"

namespace esphome::vl53l5cx {
enum VL53L5CXResolution {
  VL53L5CX_4X4,
  VL53L5CX_8X8,
};

enum VL53L5CXMode {
  VL53L5CX_CONTINUOUS,
  VL53L5CX_AUTO,
};

enum VL53L5CXTargetOrder {
  VL53L5CX_CLOSEST,
  VL53L5CX_STRONGEST,
};

class VL53L5CX : public PollingComponent, public i2c::I2CDevice {
 public:
  VL53L5CX();

  void setup() override;

  void dump_config() override;

  void update() override;

  void loop() override;

  void start_ranging();
  void stop_ranging();

  void set_resolution(const VL53L5CXResolution resolution) { this->resolution_ = resolution; }
  void set_ranging_frequency(const uint32_t ranging_frequency) { this->ranging_frequency_ = ranging_frequency; }
  void set_ranging_mode(const VL53L5CXMode ranging_mode) { this->ranging_mode_ = ranging_mode; }
  void set_integration_time(const uint32_t integration_time) { this->integration_time_ = integration_time; }
  void set_sharpening(const uint8_t sharpening) { this->sharpening_ = sharpening; }
  void set_target_order(const VL53L5CXTargetOrder target_order) { this->target_order_ = target_order; }
  void set_continuous_update(const bool continuous_update) { this->continuous_update_ = continuous_update; }
#ifdef RUN_XTALK_CALIBRATION
  void set_xtalk_calibration_reflectance(uint8_t xtalk_calibration_reflectance) {
    this->xtalk_calibration_reflectance_ = xtalk_calibration_reflectance;
  }
  void set_xtalk_calibration_distance(uint16_t xtalk_calibration_distance) {
    this->xtalk_calibration_distance_ = xtalk_calibration_distance;
  }
#endif
#ifdef SET_XTALK_CALIBRATION_DATA
  void set_xtalk_calibration_data(const char *xtalk_calibration_data) {
    this->xtalk_calibration_data_ = xtalk_calibration_data;
  }
#endif
  void set_lp_pin(GPIOPin *lp) { this->lp_pin_ = lp; }
  void set_reset_pin(GPIOPin *reset) { this->reset_pin_ = reset; }

  void register_sensor(VL53L5CXChild *obj) { this->sensors_.push_back(obj); }

  static bool all_sensors_initialized();

  protected:
  void setup_pins_() const;
  void hw_reset_() const;
  void fail_(const char *message, uint8_t data);

  VL53L5CXResolution resolution_{VL53L5CX_8X8};
  uint32_t ranging_frequency_{1};
  VL53L5CXMode ranging_mode_{VL53L5CX_AUTO};
  uint32_t integration_time_{5};
  uint8_t sharpening_{14};
  VL53L5CXTargetOrder target_order_{VL53L5CX_STRONGEST};
  bool continuous_update_{false};
#ifdef RUN_XTALK_CALIBRATION
  uint8_t xtalk_calibration_reflectance_{3};
  uint16_t xtalk_calibration_distance_{600};
#endif
#ifdef SET_XTALK_CALIBRATION_DATA
  const char *xtalk_calibration_data_{nullptr};
#endif
  GPIOPin *lp_pin_{nullptr};
  GPIOPin *reset_pin_{nullptr};
  bool initiated_read_{false};
  bool device_initiated_{false};
  uint8_t is_ready_{0};
  VL53L5CX_Configuration configuration_{};
  VL53L5CX_ResultsData results_{};
  uint8_t zones_{};

  static std::list<VL53L5CX *> vl53_devices;  // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)
  static bool lp_pin_setup_complete;      // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)

  std::vector<VL53L5CXChild *> sensors_;
};
}  // namespace esphome::vl53l5cx
