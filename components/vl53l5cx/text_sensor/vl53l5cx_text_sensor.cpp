#include "vl53l5cx_text_sensor.h"
#include "esphome/core/log.h"
#define UNCOMPNG_CONFIG__STATIC_FUNCTIONS  // NOLINT
#define UNCOMPNG_IMPLEMENTATION
#include "uncompng.c"

#include <cinttypes>

namespace esphome::vl53l5cx {
static const auto TAG = "vl53l5cx_text_sensor";

static char buffer[1000]; // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)
static char mini_buffer[8]; // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)
static uint16_t buffer_offset; // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)
static uint16_t buffer_offset_b; // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)

void VL53L5CXTextSensor::dump_config() {
  LOG_TEXT_SENSOR("", "VL53L5CXTextSensor", this);
  switch (this->output_formatting_) {
    case VL53L5CX_CSV:
      ESP_LOGCONFIG(TAG, "  Output Formatting: CSV");
      break;
    case VL53L5CX_TABLE:
      ESP_LOGCONFIG(TAG, "  Output Formatting: TABLE");
      break;
    case VL53L5CX_BINARY:
      ESP_LOGCONFIG(TAG, "  Output Formatting: BINARY");
      break;
    case VL53L5CX_PNG:
      ESP_LOGCONFIG(TAG, "  Output Formatting: PNG");
      break;
    default:
      ESP_LOGCONFIG(TAG, "  Output Formatting: unknown");
  }
  switch (this->output_data_) {
    case VL53L5CX_RAW_DISTANCE:
      ESP_LOGCONFIG(TAG, "  Output Data: RAW DISTANCE");
      break;
    case VL53L5CX_FLOAT_DISTANCE:
      ESP_LOGCONFIG(TAG, "  Output Data: FLOAT DISTANCE");
      break;
    case VL53L5CX_RAW_SIGMA:
      ESP_LOGCONFIG(TAG, "  Output Data: RAW SIGMA");
      break;
    case VL53L5CX_FLOAT_SIGMA:
      ESP_LOGCONFIG(TAG, "  Output Data: FLOAT SIGMA");
      break;
    case VL53L5CX_REFLECTANCE:
      ESP_LOGCONFIG(TAG, "  Output Data: REFLECTANCE");
      break;
    case VL53L5CX_TARGET_COUNT:
      ESP_LOGCONFIG(TAG, "  Output Data: TARGET COUNT");
      break;
    case VL53L5CX_STATUS:
      ESP_LOGCONFIG(TAG, "  Output Data: STATUS");
      break;
    default:
      ESP_LOGCONFIG(TAG, "  Output Data: unknown");
  }
  ESP_LOGCONFIG(TAG, "  Flip X: %s", this->flip_x_ ? "true" : "false");
  ESP_LOGCONFIG(TAG, "  Flip Y: %s", this->flip_y_ ? "true" : "false");
}

void VL53L5CXTextSensor::on_update(const VL53L5CX_ResultsData *result_data, const uint8_t zones) {
  switch (this->output_formatting_) {
    case VL53L5CX_CSV:
      output_visual_(result_data, zones, ',', 0, "\n", false);
      break;
    case VL53L5CX_TABLE:
      output_visual_(result_data, zones, '|', '-', "\n", true);
      break;
    case VL53L5CX_BINARY:
      output_binary_(result_data, zones, false);
      break;
    case VL53L5CX_PNG:
      output_binary_(result_data, zones, true);
      break;
    default:
      ESP_LOGW(TAG, "Invalid output formatting.");
      return;
  }
}

void VL53L5CXTextSensor::output_visual_(const VL53L5CX_ResultsData *result_data, const uint8_t zones, const char separator,
                                        const char row_spacer, const std::string &eol_sequence,
                                        const bool skip_separator_eol) {
  uint8_t sensor_width;

  uint8_t symbol_length = 4;

  buffer_offset = 0;
  if (this->output_data_ == VL53L5CX_FLOAT_DISTANCE)
    symbol_length = 5;
  if (this->output_data_ == VL53L5CX_STATUS)
    symbol_length = 3;
  if (zones == 64) {
    sensor_width = 8;
  } else if (zones == 16) {
    sensor_width = 4;
  } else {
    ESP_LOGW(TAG, "Invalid zone count: %d", zones);
    return;
  }
  uint8_t row_length = (sensor_width * symbol_length) + (sensor_width - 1);
  if (!skip_separator_eol)
    row_length++;

  for (uint8_t i = 0; i < zones; i++) {
    if ((i % sensor_width) != 0) {
      buffer[buffer_offset] = separator;
      buffer_offset++;
    } else if (i > 0) {
      if (!skip_separator_eol) {
        buffer[buffer_offset] = separator;
        buffer_offset++;
      }
      if ((row_spacer != 0) && (i < (zones - 1))) {
        buffer_offset += eol_sequence.copy(buffer + buffer_offset, eol_sequence.length(), 0);
        for (uint8_t j = 0; j < row_length; j++) {
          buffer[buffer_offset] = row_spacer;
          buffer_offset++;
        }
      }
      buffer_offset += eol_sequence.copy(buffer + buffer_offset, eol_sequence.length(), 0);
    }
    uint16_t mini_buffer_length;
    switch (this->output_data_) {
      case VL53L5CX_FLOAT_DISTANCE:
        mini_buffer_length = sprintf(mini_buffer, "%.3f", convert_distance(result_data, transform_(i, zones)));
        break;
      case VL53L5CX_RAW_DISTANCE:
        mini_buffer_length = sprintf(mini_buffer, "%d", result_data->distance_mm[transform_(i, zones)]);
        break;
      case VL53L5CX_FLOAT_SIGMA:
        mini_buffer_length = sprintf(mini_buffer, "%.3f", convert_sigma(result_data, transform_(i, zones)));
        break;
      case VL53L5CX_RAW_SIGMA:
        mini_buffer_length = sprintf(mini_buffer, "%d", result_data->range_sigma_mm[transform_(i, zones)]);
        break;
      case VL53L5CX_REFLECTANCE:
        mini_buffer_length = sprintf(mini_buffer, "%d", result_data->reflectance[transform_(i, zones)]);
        break;
      case VL53L5CX_TARGET_COUNT:
        mini_buffer_length = sprintf(mini_buffer, "%d", result_data->nb_target_detected[transform_(i, zones)]);
        break;
      case VL53L5CX_STATUS:
        mini_buffer_length = sprintf(mini_buffer, "%d", result_data->target_status[transform_(i, zones)]);
        break;
      default:
        ESP_LOGW(TAG, "Invalid output data.");
        return;
    }
    for (uint8_t j = mini_buffer_length; j < symbol_length; j++) {
      buffer[buffer_offset] = ' ';
      buffer_offset++;
    }
    std::memcpy(buffer + buffer_offset, mini_buffer, mini_buffer_length);
    buffer_offset += mini_buffer_length;
  }
  this->publish_state(buffer, buffer_offset);
}

static int png_write_func(void *context, const uint8_t *data_ptr, size_t data_len) {
  std::memcpy(buffer + buffer_offset_b, data_ptr, data_len);
  buffer_offset_b += data_len;
  return 0;
}

void VL53L5CXTextSensor::output_binary_(const VL53L5CX_ResultsData *result_data, uint8_t zones, bool as_png) {
  uint8_t sensor_width;
  if (zones == 64) {
    sensor_width = 8;
  } else if (zones == 16) {
    sensor_width = 4;
  } else {
    ESP_LOGW(TAG, "Invalid zone count: %d", zones);
    return;
  }

  MemFloat tmp_f{};
  MemShort tmp_s{};
  MemByte tmp_b{};
  buffer_offset = 0;
  uint32_t pixel_format = 0;
  for (uint8_t i = 0; i < zones; i++) {
    switch (this->output_data_) {
      case VL53L5CX_FLOAT_DISTANCE:
        if (as_png) {
          tmp_s.u = result_data->distance_mm[transform_(i, zones)];
          if (tmp_s.u > 4093)
            tmp_s.u = 4093;
          tmp_s.u = tmp_s.u << 4;
          std::memcpy(buffer + buffer_offset, tmp_s.b, 2);
          buffer_offset += 2;
          pixel_format = UNCOMPNG__PIXEL_FORMAT__Y_16LE;
          break;
        }
        tmp_f.f = convert_distance(result_data, transform_(i, zones));
        std::memcpy(buffer + buffer_offset, tmp_f.b, 4);
        buffer_offset += 4;
        break;
      case VL53L5CX_RAW_DISTANCE:
        tmp_s.u = result_data->distance_mm[transform_(i, zones)];
        std::memcpy(buffer + buffer_offset, tmp_s.b, 2);
        buffer_offset += 2;
        pixel_format = UNCOMPNG__PIXEL_FORMAT__Y_16LE;
        break;
      case VL53L5CX_FLOAT_SIGMA:
        if (as_png) {
          tmp_s.u = result_data->range_sigma_mm[transform_(i, zones)];
          if (tmp_s.u > 4093)
            tmp_s.u = 4093;
          tmp_s.u = tmp_s.u << 4;
          std::memcpy(buffer + buffer_offset, tmp_s.b, 2);
          buffer_offset += 2;
          pixel_format = UNCOMPNG__PIXEL_FORMAT__Y_16LE;
          break;
        }
        tmp_f.f = convert_sigma(result_data, transform_(i, zones));
        std::memcpy(buffer + buffer_offset, tmp_f.b, 4);
        buffer_offset += 4;
        break;
      case VL53L5CX_RAW_SIGMA:
        tmp_s.u = result_data->range_sigma_mm[transform_(i, zones)];
        std::memcpy(buffer + buffer_offset, tmp_s.b, 2);
        buffer_offset += 2;
        pixel_format = UNCOMPNG__PIXEL_FORMAT__Y_16LE;
        break;
      case VL53L5CX_REFLECTANCE:
        tmp_b.u = result_data->reflectance[transform_(i, zones)];
        std::memcpy(buffer + buffer_offset, tmp_b.b, 1);
        buffer_offset += 1;
        pixel_format = UNCOMPNG__PIXEL_FORMAT__Y;
        break;
      case VL53L5CX_TARGET_COUNT:
        tmp_b.u = result_data->nb_target_detected[transform_(i, zones)];
        std::memcpy(buffer + buffer_offset, tmp_b.b, 1);
        buffer_offset += 1;
        pixel_format = UNCOMPNG__PIXEL_FORMAT__Y;
        break;
      case VL53L5CX_STATUS:
        tmp_b.u = result_data->target_status[transform_(i, zones)];
        std::memcpy(buffer + buffer_offset, tmp_b.b, 1);
        buffer_offset += 1;
        pixel_format = UNCOMPNG__PIXEL_FORMAT__Y;
        break;
      default:
        ESP_LOGW(TAG, "Invalid output data.");
        return;
    }
  }

  if (as_png) {
    buffer_offset_b = 256;
    uint8_t bpp = 1;
    if (pixel_format == UNCOMPNG__PIXEL_FORMAT__Y_16LE)
      bpp = 2;
    uncompng__encode(&png_write_func, nullptr, reinterpret_cast<uint8_t *>(buffer), buffer_offset, sensor_width,
                     sensor_width, sensor_width * bpp, pixel_format);

    this->publish_state(base64_encode(reinterpret_cast<uint8_t *>(buffer) + 256, buffer_offset_b - 256));
    return;
  }

  this->publish_state(base64_encode(reinterpret_cast<uint8_t *>(buffer), buffer_offset));
}

float VL53L5CXTextSensor::convert_distance(const VL53L5CX_ResultsData *result_data, const uint8_t zone) {
  if (const uint8_t status = result_data->target_status[zone]; status == 5 || status == 6 || status == 9) {
    return static_cast<float>(result_data->distance_mm[zone]) / 1000.0f;
  }
  return NAN;
}

float VL53L5CXTextSensor::convert_sigma(const VL53L5CX_ResultsData *result_data, const uint8_t zone) {
  return static_cast<float>(result_data->range_sigma_mm[zone]) / 1000.0f;
}

uint8_t VL53L5CXTextSensor::transform_(const uint8_t zone, const uint8_t zones) const {
  if ((!this->flip_x_) && (!this->flip_y_))
    return zone;
  uint8_t sensor_width;
  if (zones == 64) {
    sensor_width = 8;
  } else if (zones == 16) {
    sensor_width = 4;
  } else {
    ESP_LOGW(TAG, "Invalid zone count: %d", zones);
    return 0;
  }

  uint8_t pos_x = zone % sensor_width;
  uint8_t pos_y = zone / sensor_width;

  if (this->flip_x_) {
    pos_x = sensor_width - pos_x - 1;
  }
  if (this->flip_y_) {
    pos_y = sensor_width - pos_y - 1;
  }

  return (pos_y * sensor_width) + pos_x;
}
}  // namespace esphome::vl53l5cx
