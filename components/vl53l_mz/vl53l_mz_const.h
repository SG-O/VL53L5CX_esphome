#pragma once

namespace esphome::vl53l_mz {

#define VL53LMZ_INVALID_OPTION 255

typedef struct
{
  uint16_t total;
  uint8_t	 height;
  uint8_t	 width;
} VL53LMZ_Resolution_Detail;

enum VL53LMZResolution {
  VL53LMZ_4X4,
  VL53LMZ_8X8,
};

const VL53LMZ_Resolution_Detail VL53LMZ_RESOLUTIONS[] = {
  [VL53LMZ_4X4] = {4*4,4,4},
  [VL53LMZ_8X8] = {8*8,8,8},
};

enum VL53LMZMode {
  VL53LMZ_CONTINUOUS,
  VL53LMZ_AUTO,
};

enum VL53LMZTargetOrder {
  VL53LMZ_CLOSEST,
  VL53LMZ_STRONGEST,
};

}  // namespace esphome::vl53l_mz
