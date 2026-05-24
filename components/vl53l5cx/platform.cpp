/**
 *
 * Copyright (c) 2021 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the ST_ULD_LICENSE file
 * in the root directory of this software component.
 * If no ST_ULD_LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */

#include "platform.h"

namespace esphome::vl53l5cx {

uint8_t VL53L5CX_RdByte(const VL53L5CX_Platform *p_platform, const uint16_t RegisterAddress, uint8_t *p_value) {
  return p_platform->rd_byte_func(p_platform->reference_, RegisterAddress, p_value);
}

uint8_t VL53L5CX_WrByte(const VL53L5CX_Platform *p_platform, const uint16_t RegisterAddress, const uint8_t value) {
  return p_platform->wr_byte_func(p_platform->reference_, RegisterAddress, value);
}

uint8_t VL53L5CX_WrMulti(const VL53L5CX_Platform *p_platform, const uint16_t RegisterAddress, const uint8_t *p_values,
                         const uint32_t size) {
  return p_platform->wr_bytes_func(p_platform->reference_, RegisterAddress, p_values, size);
}

uint8_t VL53L5CX_RdMulti(const VL53L5CX_Platform *p_platform, const uint16_t RegisterAddress, uint8_t *p_values,
                         const uint32_t size) {
  return p_platform->rd_bytes_func(p_platform->reference_, RegisterAddress, p_values, size);
}

uint8_t VL53L5CX_Reset_Sensor(VL53L5CX_Platform *p_platform) {
  uint8_t status = 0;

  /* (Optional) This function returns 0 if OK */

  /* Set pin LPN to LOW */
  /* Set pin AVDD to LOW */
  /* Set pin VDDIO  to LOW */
  VL53L5CX_WaitMs(p_platform, 100);

  /* Set pin LPN of to HIGH */
  /* Set pin AVDD of to HIGH */
  /* Set pin VDDIO of  to HIGH */
  VL53L5CX_WaitMs(p_platform, 100);

  return status;
}

void VL53L5CX_SwapBuffer(uint8_t *buffer, const uint16_t size) {
  uint32_t tmp;

  /* Example of possible implementation using <string.h> */
  for (uint32_t i = 0; i < size; i = i + 4) {
    tmp = (buffer[i] << 24) | (buffer[i + 1] << 16) | (buffer[i + 2] << 8) | (buffer[i + 3]);

    memcpy(&(buffer[i]), &tmp, 4);
  }
}

uint8_t VL53L5CX_WaitMs(VL53L5CX_Platform *p_platform, const uint32_t TimeMs) {
  /* This function returns 0 if OK */
  p_platform->delay_func(TimeMs);
  return 0;
}
}  // namespace esphome::vl53l5cx
