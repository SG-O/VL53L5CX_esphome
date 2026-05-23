#pragma once

#include "esphome/components/sensor/sensor.h"
#include "../vl53l5cx_child.h"

namespace esphome::vl53l5cx {
enum VL53L5CXZoneMode {
  VL53L5CX_SINGLE,
  VL53L5CX_AVERAGE,
  VL53L5CX_NEAREST,
  VL53L5CX_FARTHEST,
  VL53L5CX_CENTER,
};

enum VL53L5CXZoneData {
  VL53L5CX_DISTANCE,
  VL53L5CX_SIGMA,
  VL53L5CX_ZONE_REFLECTANCE,
  VL53L5CX_ZONE_TARGET_COUNT,
};

class VL53L5CXSensor : public VL53L5CXChild, public sensor::Sensor, public Component {
 public:
  void dump_config() override;

  void setup() override;

  void on_update(const VL53L5CX_ResultsData *result_data, uint8_t zones) override;

  void set_zone_mode(const VL53L5CXZoneMode zone_mode) { this->zone_mode_ = zone_mode; }
  void set_zone_data(const VL53L5CXZoneData zone_data) { this->zone_data_ = zone_data; }
  void set_selected_zone(const uint8_t selected_zone) { this->selected_zone_ = selected_zone; }

 protected:
  [[nodiscard]] static float convert_distance(const VL53L5CX_ResultsData *result_data, uint8_t zone);

  [[nodiscard]] float select_data(const VL53L5CX_ResultsData *result_data, uint8_t zone, bool square_sigma) const;

  VL53L5CXZoneMode zone_mode_{VL53L5CX_SINGLE};
  VL53L5CXZoneData zone_data_{VL53L5CX_DISTANCE};
  uint8_t selected_zone_{0};

  float result_ = 0.0f;
};
}  // namespace esphome::vl53l5cx
