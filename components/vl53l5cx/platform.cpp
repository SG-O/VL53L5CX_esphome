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
static const char *const TAG = "vl53l5cx_platform";

uint8_t VL53L5CX_RdByte(const VL53L5CX_Platform *p_platform, const uint16_t RegisterAddress, uint8_t *p_value) {
  const i2c::ErrorCode status = p_platform->i2cDevice_->read_register16(RegisterAddress, p_value, 1);
  /* This function returns 0 if OK */
  if (status) {
    ESP_LOGD(TAG, "Failed to read: %x", status);
  }
  return static_cast<uint8_t>(status);
}

uint8_t VL53L5CX_WrByte(const VL53L5CX_Platform *p_platform, const uint16_t RegisterAddress, const uint8_t value) {
  const i2c::ErrorCode status = p_platform->i2cDevice_->write_register16(RegisterAddress, &value, 1);
  /* This function returns 0 if OK */
  if (status) {
    ESP_LOGD(TAG, "Failed to write: %x", status);
  }
  return static_cast<uint8_t>(status);
}

uint8_t VL53L5CX_WrMulti(const VL53L5CX_Platform *p_platform, const uint16_t RegisterAddress, const uint8_t *p_values,
                         const uint32_t size) {
  uint32_t rest;
  i2c::ErrorCode status;
  for (uint32_t i = 0; i < size; i += VL53L5CX_NB_BYTES_PER_I2C_OP) {
    rest = size - i;
    if (rest > VL53L5CX_NB_BYTES_PER_I2C_OP)
      rest = VL53L5CX_NB_BYTES_PER_I2C_OP;
    status = p_platform->i2cDevice_->write_register16(RegisterAddress + i, p_values + i, rest);
    /* This function returns 0 if OK */
    if (status) {
      ESP_LOGD(TAG, "Failed to write multiple: %x", status);
      break;
    }
    arch_feed_wdt();
  }
  return static_cast<uint8_t>(status);
}

uint8_t VL53L5CX_RdMulti(const VL53L5CX_Platform *p_platform, const uint16_t RegisterAddress, uint8_t *p_values,
                         const uint32_t size) {
  uint32_t rest;
  i2c::ErrorCode status;
  for (uint32_t i = 0; i < size; i += VL53L5CX_NB_BYTES_PER_I2C_OP) {
    rest = size - i;
    if (rest > VL53L5CX_NB_BYTES_PER_I2C_OP)
      rest = VL53L5CX_NB_BYTES_PER_I2C_OP;
    status = p_platform->i2cDevice_->read_register16(RegisterAddress + i, p_values + i, rest);
    /* This function returns 0 if OK */
    if (status) {
      ESP_LOGD(TAG, "Failed to read multiple: %x", status);
      break;
    }
    arch_feed_wdt();
  }
  return static_cast<uint8_t>(status);
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
  delay(TimeMs);
  arch_feed_wdt();
  return 0;
}
}  // namespace esphome::vl53l5cx
