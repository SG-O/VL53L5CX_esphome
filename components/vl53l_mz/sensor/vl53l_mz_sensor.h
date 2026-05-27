#pragma once

#include "esphome/components/sensor/sensor.h"
#include "../vl53l_mz_child.h"

namespace esphome::vl53l_mz {
enum VL53LMZZoneMode {
  VL53LMZ_SINGLE,
  VL53LMZ_AVERAGE,
  VL53LMZ_NEAREST,
  VL53LMZ_FARTHEST,
  VL53LMZ_CENTER,
};

enum VL53LMZZoneData {
  VL53LMZ_DISTANCE,
  VL53LMZ_SIGMA,
  VL53LMZ_ZONE_REFLECTANCE,
  VL53LMZ_ZONE_TARGET_COUNT,
};

class VL53LMZSensor : public VL53LMZChild, public sensor::Sensor, public Component {
 public:
  void dump_config() override;

  void setup() override;

  void on_update(const VL53LMZ_ResultsData *result_data, VL53LMZ_Resolution_Detail resolution) override;

  void set_zone_mode(const VL53LMZZoneMode zone_mode) { this->zone_mode_ = zone_mode; }
  void set_zone_data(const VL53LMZZoneData zone_data) { this->zone_data_ = zone_data; }
  void set_selected_zone(const uint8_t selected_zone) { this->selected_zone_ = selected_zone; }

 protected:
  [[nodiscard]] static float convert_distance(const VL53LMZ_ResultsData *result_data, uint16_t zone);

  [[nodiscard]] float select_data(const VL53LMZ_ResultsData *result_data, uint16_t zone, bool square_sigma) const;

  VL53LMZZoneMode zone_mode_{VL53LMZ_SINGLE};
  VL53LMZZoneData zone_data_{VL53LMZ_DISTANCE};
  uint8_t selected_zone_{0};

  float result_ = 0.0f;
};
}  // namespace esphome::vl53l_mz
