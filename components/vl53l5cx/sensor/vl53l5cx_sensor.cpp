#include "vl53l5cx_sensor.h"
#include "esphome/core/log.h"

#include <cinttypes>
#include <cmath>

namespace esphome::vl53l5cx {
static const auto TAG = "vl53l5cx_sensor";

void VL53L5CXSensor::dump_config() {
  LOG_SENSOR("", "VL53L5CXSensor", this);
  if (this->is_failed()) {
    ESP_LOGE(TAG, ESP_LOG_MSG_COMM_FAIL);
  }
  switch (this->zone_mode_) {
    case VL53L5CX_SINGLE:
      ESP_LOGCONFIG(TAG, "  Zone Mode: SINGLE");
      break;
    case VL53L5CX_AVERAGE:
      ESP_LOGCONFIG(TAG, "  Zone Mode: AVERAGE");
      break;
    case VL53L5CX_NEAREST:
      ESP_LOGCONFIG(TAG, "  Zone Mode: NEAREST");
      break;
    case VL53L5CX_FARTHEST:
      ESP_LOGCONFIG(TAG, "  Zone Mode: FARTHEST");
      break;
    case VL53L5CX_CENTER:
      ESP_LOGCONFIG(TAG, "  Zone Mode: CENTER");
      break;
    default:
      ESP_LOGCONFIG(TAG, "  Zone Mode: unknown");
  }
  switch (this->zone_data_) {
    case VL53L5CX_DISTANCE:
      ESP_LOGCONFIG(TAG, "  Zone Data: DISTANCE");
      break;
    case VL53L5CX_SIGMA:
      ESP_LOGCONFIG(TAG, "  Zone Data: SIGMA");
      break;
    case VL53L5CX_ZONE_REFLECTANCE:
      ESP_LOGCONFIG(TAG, "  Zone Data: REFLECTANCE");
      break;
    case VL53L5CX_ZONE_TARGET_COUNT:
      ESP_LOGCONFIG(TAG, "  Zone Data: TARGET COUNT");
      break;
    default:
      ESP_LOGCONFIG(TAG, "  Zone Data: unknown");
  }
  ESP_LOGCONFIG(TAG, "  Selected Zone: %d", this->selected_zone_);
}

void VL53L5CXSensor::setup() {}

void VL53L5CXSensor::on_update(const VL53L5CX_ResultsData *result_data, const uint8_t zones) {
  float tmp;
  uint8_t i;
  uint8_t n;
  uint8_t valid;
  switch (this->zone_mode_) {
    case VL53L5CX_SINGLE:
      if (this->selected_zone_ >= zones) {
        ESP_LOGW(TAG, "Selected zone out of range.");
        this->publish_state(NAN);
        return;
      }
      this->result_ = select_data(result_data, this->selected_zone_, false);
      break;
    case VL53L5CX_AVERAGE:
      this->result_ = 0;
      valid = 0;
      for (i = 0; i < zones; i++) {
        tmp = select_data(result_data, i, true);
        if (tmp == NAN)
          continue;
        valid++;
        this->result_ += tmp;
      }
      if (valid < 1) {
        this->result_ = NAN;
        break;
      }
      this->result_ /= static_cast<float>(zones);
      if (this->zone_data_ == VL53L5CX_SIGMA) {
        this->result_ = std::sqrt(this->result_);
      }
      break;
    case VL53L5CX_NEAREST:
      valid = 0;
      n = 0;
      this->result_ = std::numeric_limits<float>::max();
      for (i = 0; i < zones; i++) {
        tmp = convert_distance(result_data, i);
        if (tmp == NAN)
          continue;
        valid++;
        if (tmp < this->result_) {
          this->result_ = tmp;
          n = i;
        }
      }
      if (valid < 1) {
        this->result_ = NAN;
      }
      this->result_ = select_data(result_data, n, false);
      break;
    case VL53L5CX_FARTHEST:
      valid = 0;
      n = 0;
      this->result_ = 0;
      for (i = 0; i < zones; i++) {
        tmp = convert_distance(result_data, i);
        if (tmp == NAN)
          continue;
        valid++;
        if (tmp > this->result_) {
          this->result_ = tmp;
          n = i;
        }
      }
      if (valid < 1) {
        this->result_ = NAN;
      }
      this->result_ = select_data(result_data, n, false);
      break;
    case VL53L5CX_CENTER:
      this->result_ = 0;
      if (zones == 64) {
        this->result_ += select_data(result_data, 27, true);
        this->result_ += select_data(result_data, 28, true);
        this->result_ += select_data(result_data, 35, true);
        this->result_ += select_data(result_data, 36, true);
      } else {
        this->result_ += select_data(result_data, 5, true);
        this->result_ += select_data(result_data, 6, true);
        this->result_ += select_data(result_data, 9, true);
        this->result_ += select_data(result_data, 10, true);
      }
      this->result_ /= 4;
      if (this->zone_data_ == VL53L5CX_SIGMA) {
        this->result_ = std::sqrt(this->result_);
      }
      break;
    default:
      ESP_LOGW(TAG, "Invalid zone mode.");
  }
  if (this->result_ == NAN) {
    ESP_LOGI(TAG, "Target is out of range");
  }
  this->publish_state(this->result_);
}

float VL53L5CXSensor::select_data(const VL53L5CX_ResultsData *result_data, const uint8_t zone,
                                    const bool square_sigma) const {
  float tmp;
  switch (this->zone_data_) {
    case VL53L5CX_DISTANCE:
      return convert_distance(result_data, zone);
    case VL53L5CX_SIGMA:
      tmp = static_cast<float>(result_data->range_sigma_mm[zone]) / 1000.0f;
      if (square_sigma)
        tmp *= tmp;
      return tmp;
    case VL53L5CX_ZONE_REFLECTANCE:
      return result_data->reflectance[zone];
    case VL53L5CX_ZONE_TARGET_COUNT:
      return result_data->nb_target_detected[zone];
    default:
      ESP_LOGW(TAG, "Invalid zone data.");
      return NAN;
  }
}

float VL53L5CXSensor::convert_distance(const VL53L5CX_ResultsData *result_data, const uint8_t zone) {
  if (const uint8_t status = result_data->target_status[zone]; status == 5 || status == 6 || status == 9) {
    return static_cast<float>(result_data->distance_mm[zone]) / 1000.0f;
  }
  return NAN;
}
}  // namespace esphome::vl53l5cx
