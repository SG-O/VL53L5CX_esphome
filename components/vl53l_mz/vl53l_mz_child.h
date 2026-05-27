#pragma once

#include "esphome/core/helpers.h"
#include "vl53l_mz_api_wrapper.h"

namespace esphome::vl53l_mz {
class VL53LMZ;

class VL53LMZChild : public Parented<VL53LMZ> {
 public:
  virtual void on_update(const VL53LMZ_ResultsData *result_data, VL53LMZ_Resolution_Detail resolution) = 0;

 protected:
  friend VL53LMZ;
};
}  // namespace esphome::vl53l_mz
