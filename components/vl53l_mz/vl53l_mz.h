#pragma once

#include <list>

#include "esphome/core/component.h"
#include "esphome/core/defines.h"  //NOLINT
#include "esphome/core/hal.h"
#include "esphome/components/i2c/i2c.h"
#include "vl53l_mz_const.h"
#include "vl53l_mz_child.h"

#ifdef VL53L_MZ_USE_VL53L5CX
#include "vl53l5cx_api_wrapper.h"
#endif
#ifdef VL53L_MZ_USE_VL53L7CX
#include "vl53l7cx_api_wrapper.h"
#endif
#ifdef VL53L_MZ_USE_VL53L8CX
#include "vl53l8cx_api_wrapper.h"
#endif

namespace esphome::vl53l_mz {

class VL53LMZ : public PollingComponent, public i2c::I2CDevice {
 public:
  virtual ~VL53LMZ() = default;
  VL53LMZ();

  void setup() override;

  void dump_config() override;

  void update() override;

  void loop() override;

  void start_ranging();
  void stop_ranging();

  void set_resolution(const VL53LMZResolution resolution) { this->resolution_ = resolution; }
  void set_ranging_frequency(const uint32_t ranging_frequency) { this->ranging_frequency_ = ranging_frequency; }
  void set_ranging_mode(const VL53LMZMode ranging_mode) { this->ranging_mode_ = ranging_mode; }
  void set_integration_time(const uint32_t integration_time) { this->integration_time_ = integration_time; }
  void set_sharpening(const uint8_t sharpening) { this->sharpening_ = sharpening; }
  void set_target_order(const VL53LMZTargetOrder target_order) { this->target_order_ = target_order; }
  void set_continuous_update(const bool continuous_update) { this->continuous_update_ = continuous_update; }
#ifdef VL53L_MZ_RUN_XTALK_CALIBRATION
  void set_xtalk_calibration_reflectance(uint8_t xtalk_calibration_reflectance) {
    this->xtalk_calibration_reflectance_ = xtalk_calibration_reflectance;
  }
  void set_xtalk_calibration_distance(uint16_t xtalk_calibration_distance) {
    this->xtalk_calibration_distance_ = xtalk_calibration_distance;
  }
#endif
#ifdef VL53L_MZ_SET_XTALK_CALIBRATION_DATA
  void set_xtalk_calibration_data(const char *xtalk_calibration_data) {
    this->xtalk_calibration_data_ = xtalk_calibration_data;
  }
#endif
  void set_lp_pin(GPIOPin *lp) { this->lp_pin_ = lp; }
  void set_reset_pin(GPIOPin *reset) { this->reset_pin_ = reset; }
  void set_int_pin(InternalGPIOPin *int_pin) { this->int_pin_ = int_pin; }

  void register_sensor(VL53LMZChild *obj) { this->sensors_.push_back(obj); }

  static bool all_sensors_initialized();
  static void intr_handle(VL53LMZ *arg);

  protected:
  void setup_pins_();
  void hw_reset_() const;
  void fail_(const char *message, uint8_t data);
  void process_ranging_data_();

  VL53LMZResolution resolution_{VL53LMZ_8X8};
  uint32_t ranging_frequency_{1};
  VL53LMZMode ranging_mode_{VL53LMZ_AUTO};
  uint32_t integration_time_{5};
  uint8_t sharpening_{14};
  VL53LMZTargetOrder target_order_{VL53LMZ_STRONGEST};
  bool continuous_update_{false};
#ifdef VL53L_MZ_RUN_XTALK_CALIBRATION
  uint8_t xtalk_calibration_reflectance_{3};
  uint16_t xtalk_calibration_distance_{600};
#endif
#ifdef VL53L_MZ_SET_XTALK_CALIBRATION_DATA
  const char *xtalk_calibration_data_{nullptr};
#endif
  GPIOPin *lp_pin_{nullptr};
  GPIOPin *reset_pin_{nullptr};
  InternalGPIOPin *int_pin_{nullptr};
  volatile bool interrupt_triggered_{false};
  bool initiated_read_{false};
  bool device_initiated_{false};
  uint8_t is_ready_{0};
  VL53LMZApiWrapper *api_{nullptr};

  static std::list<VL53LMZ *> vl53_devices;  // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)
  static bool lp_pin_setup_complete;      // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)

  std::vector<VL53LMZChild *> sensors_;
};

#ifdef VL53L_MZ_USE_VL53L5CX
class VL53L5CX : public VL53LMZ {
 public:
  VL53L5CX() {
    api_ = new VL53L5CXApiWrapper(this);
  }
};
#endif

#ifdef VL53L_MZ_USE_VL53L7CX
class VL53L7CX : public VL53LMZ {
public:
  VL53L7CX() {
    api_ = new VL53L7CXApiWrapper(this);
  }
};
#endif

#ifdef VL53L_MZ_USE_VL53L8CX
class VL53L8CX : public VL53LMZ {
public:
  VL53L8CX() {
    api_ = new VL53L8CXApiWrapper(this);
  }
};
#endif

}  // namespace esphome::vl53l_mz
