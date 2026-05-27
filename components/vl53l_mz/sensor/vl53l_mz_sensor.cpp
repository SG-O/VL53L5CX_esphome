#include "vl53l_mz_sensor.h"
#include "esphome/core/log.h"

#include <cinttypes>
#include <cmath>

namespace esphome::vl53l_mz {
static const auto TAG = "vl53l_mz_sensor";

void VL53LMZSensor::dump_config() {
  LOG_SENSOR("", "VL53LMZSensor", this);
  if (this->is_failed()) {
    ESP_LOGE(TAG, ESP_LOG_MSG_COMM_FAIL);
  }
  switch (this->zone_mode_) {
    case VL53LMZ_SINGLE:
      ESP_LOGCONFIG(TAG, "  Zone Mode: SINGLE");
      break;
    case VL53LMZ_AVERAGE:
      ESP_LOGCONFIG(TAG, "  Zone Mode: AVERAGE");
      break;
    case VL53LMZ_NEAREST:
      ESP_LOGCONFIG(TAG, "  Zone Mode: NEAREST");
      break;
    case VL53LMZ_FARTHEST:
      ESP_LOGCONFIG(TAG, "  Zone Mode: FARTHEST");
      break;
    case VL53LMZ_CENTER:
      ESP_LOGCONFIG(TAG, "  Zone Mode: CENTER");
      break;
    default:
      ESP_LOGCONFIG(TAG, "  Zone Mode: unknown");
  }
  switch (this->zone_data_) {
    case VL53LMZ_DISTANCE:
      ESP_LOGCONFIG(TAG, "  Zone Data: DISTANCE");
      break;
    case VL53LMZ_SIGMA:
      ESP_LOGCONFIG(TAG, "  Zone Data: SIGMA");
      break;
    case VL53LMZ_ZONE_REFLECTANCE:
      ESP_LOGCONFIG(TAG, "  Zone Data: REFLECTANCE");
      break;
    case VL53LMZ_ZONE_TARGET_COUNT:
      ESP_LOGCONFIG(TAG, "  Zone Data: TARGET COUNT");
      break;
    default:
      ESP_LOGCONFIG(TAG, "  Zone Data: unknown");
  }
  ESP_LOGCONFIG(TAG, "  Selected Zone: %d", this->selected_zone_);
}

void VL53LMZSensor::setup() {}

void VL53LMZSensor::on_update(const VL53LMZ_ResultsData *result_data, const VL53LMZ_Resolution_Detail resolution) {
  float tmp;
  uint16_t i;
  uint16_t n;
  uint16_t valid;
  switch (this->zone_mode_) {
    case VL53LMZ_SINGLE:
      if (this->selected_zone_ >= resolution.total) {
        ESP_LOGW(TAG, "Selected zone out of range.");
        this->publish_state(NAN);
        return;
      }
      this->result_ = select_data(result_data, this->selected_zone_, false);
      break;
    case VL53LMZ_AVERAGE:
      this->result_ = 0;
      valid = 0;
      for (i = 0; i < resolution.total; i++) {
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
      this->result_ /= static_cast<float>(valid);
      if (this->zone_data_ == VL53LMZ_SIGMA) {
        this->result_ = std::sqrt(this->result_);
      }
      break;
    case VL53LMZ_NEAREST:
      valid = 0;
      n = 0;
      this->result_ = std::numeric_limits<float>::max();
      for (i = 0; i < resolution.total; i++) {
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
    case VL53LMZ_FARTHEST:
      valid = 0;
      n = 0;
      this->result_ = 0;
      for (i = 0; i < resolution.total; i++) {
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
    case VL53LMZ_CENTER:
      this->result_ = 0;
      if (resolution.total == 64) {
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
      if (this->zone_data_ == VL53LMZ_SIGMA) {
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

float VL53LMZSensor::select_data(const VL53LMZ_ResultsData *result_data, const uint16_t zone,
                                    const bool square_sigma) const {
  float tmp;
  switch (this->zone_data_) {
    case VL53LMZ_DISTANCE:
      return convert_distance(result_data, zone);
    case VL53LMZ_SIGMA:
      tmp = static_cast<float>(result_data->range_sigma_mm[zone]) / 1000.0f;
      if (square_sigma)
        tmp *= tmp;
      return tmp;
    case VL53LMZ_ZONE_REFLECTANCE:
      return result_data->reflectance[zone];
    case VL53LMZ_ZONE_TARGET_COUNT:
      return result_data->nb_target_detected[zone];
    default:
      ESP_LOGW(TAG, "Invalid zone data.");
      return NAN;
  }
}

float VL53LMZSensor::convert_distance(const VL53LMZ_ResultsData *result_data, const uint16_t zone) {
  if (const uint8_t status = result_data->target_status[zone]; status == 5 || status == 6 || status == 9) {
    return static_cast<float>(result_data->distance_mm[zone]) / 1000.0f;
  }
  return NAN;
}
}  // namespace esphome::vl53l_mz
