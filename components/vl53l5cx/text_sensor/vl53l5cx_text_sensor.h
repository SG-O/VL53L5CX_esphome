#pragma once

#include "esphome/components/text_sensor/text_sensor.h"
#include "../vl53l5cx_child.h"

namespace esphome::vl53l5cx {
enum VL53L5CXOutputFormatting {
  VL53L5CX_CSV,
  VL53L5CX_TABLE,
  VL53L5CX_BINARY,
  VL53L5CX_PNG,
};

enum VL53L5CXOutputData {
  VL53L5CX_RAW_DISTANCE,
  VL53L5CX_FLOAT_DISTANCE,
  VL53L5CX_RAW_SIGMA,
  VL53L5CX_FLOAT_SIGMA,
  VL53L5CX_REFLECTANCE,
  VL53L5CX_TARGET_COUNT,
  VL53L5CX_STATUS,
};

union MemFloat {
  float f;
  uint8_t b[4];
};

union MemShort {
  uint16_t u;
  uint8_t b[2];
};

union MemByte {
  uint8_t u;
  uint8_t b[1];
};

class VL53L5CXTextSensor : public VL53L5CXChild, public text_sensor::TextSensor, public Component {
 public:
  void dump_config() override;

  void on_update(const VL53L5CX_ResultsData *result_data, uint8_t zones) override;

  void set_output_formatting(VL53L5CXOutputFormatting output_formatting) { this->output_formatting_ = output_formatting; }

  void set_output_data(VL53L5CXOutputData output_data) { this->output_data_ = output_data; }
  void set_flip_x(bool flip_x) { this->flip_x_ = flip_x; }
  void set_flip_y(bool flip_y) { this->flip_y_ = flip_y; }

 protected:
  void output_visual_(const VL53L5CX_ResultsData *result_data, uint8_t zones, char separator, char row_spacer,
                      const std::string &eol_sequence, bool skip_separator_eol);

  void output_binary_(const VL53L5CX_ResultsData *result_data, uint8_t zones, bool as_png);

  [[nodiscard]] static float convert_distance(const VL53L5CX_ResultsData *result_data, uint8_t zone);

  [[nodiscard]] static float convert_sigma(const VL53L5CX_ResultsData *result_data, uint8_t zone);

  [[nodiscard]] uint8_t transform_(uint8_t zone, uint8_t zones) const;

  VL53L5CXOutputFormatting output_formatting_{VL53L5CX_CSV};
  VL53L5CXOutputData output_data_{VL53L5CX_FLOAT_DISTANCE};
  bool flip_x_{false};
  bool flip_y_{false};
};
}  // namespace esphome::vl53l5cx
