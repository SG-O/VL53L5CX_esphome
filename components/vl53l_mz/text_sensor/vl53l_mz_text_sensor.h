#pragma once

#include "esphome/components/text_sensor/text_sensor.h"
#include "../vl53l_mz_child.h"

namespace esphome::vl53l_mz {
enum VL53LMZOutputFormatting {
  VL53LMZ_CSV,
  VL53LMZ_TABLE,
  VL53LMZ_BINARY,
  VL53LMZ_PNG,
};

enum VL53LMZOutputData {
  VL53LMZ_RAW_DISTANCE,
  VL53LMZ_FLOAT_DISTANCE,
  VL53LMZ_RAW_SIGMA,
  VL53LMZ_FLOAT_SIGMA,
  VL53LMZ_REFLECTANCE,
  VL53LMZ_TARGET_COUNT,
  VL53LMZ_STATUS,
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

class VL53LMZTextSensor : public VL53LMZChild, public text_sensor::TextSensor, public Component {
 public:
  void dump_config() override;

  void on_update(const VL53LMZ_ResultsData *result_data, VL53LMZ_Resolution_Detail resolution) override;

  void set_output_formatting(VL53LMZOutputFormatting output_formatting) { this->output_formatting_ = output_formatting; }

  void set_output_data(VL53LMZOutputData output_data) { this->output_data_ = output_data; }
  void set_flip_x(bool flip_x) { this->flip_x_ = flip_x; }
  void set_flip_y(bool flip_y) { this->flip_y_ = flip_y; }

 protected:
  void output_visual_(const VL53LMZ_ResultsData *result_data, VL53LMZ_Resolution_Detail resolution, char separator, char row_spacer,
                      const std::string &eol_sequence, bool skip_separator_eol);

  void output_binary_(const VL53LMZ_ResultsData *result_data, VL53LMZ_Resolution_Detail resolution, bool as_png);

  [[nodiscard]] static float convert_distance(const VL53LMZ_ResultsData *result_data, uint8_t zone);

  [[nodiscard]] static float convert_sigma(const VL53LMZ_ResultsData *result_data, uint8_t zone);

  [[nodiscard]] uint8_t transform_(uint8_t zone, VL53LMZ_Resolution_Detail resolution) const;

  VL53LMZOutputFormatting output_formatting_{VL53LMZ_CSV};
  VL53LMZOutputData output_data_{VL53LMZ_FLOAT_DISTANCE};
  bool flip_x_{false};
  bool flip_y_{false};
};
}  // namespace esphome::vl53l_mz
