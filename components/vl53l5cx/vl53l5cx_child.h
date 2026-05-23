#pragma once

#include "esphome/core/helpers.h"
#include "vl53l5cx_api.h"

namespace esphome::vl53l5cx {
class VL53L5CX;

class VL53L5CXChild : public Parented<VL53L5CX> {
 public:
  virtual void on_update(const VL53L5CX_ResultsData *result_data, uint8_t zones) = 0;

 protected:
  friend VL53L5CX;
};
}  // namespace esphome::vl53l5cx
