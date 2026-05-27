#include "vl53l_mz_text_sensor.h"
#include "esphome/core/log.h"
#define UNCOMPNG_CONFIG__STATIC_FUNCTIONS  // NOLINT
#define UNCOMPNG_IMPLEMENTATION
#include "uncompng.c"

#include <cinttypes>

namespace esphome::vl53l_mz {
static const auto TAG = "vl53l_mz_text_sensor";

static char buffer[1000]; // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)
static char mini_buffer[8]; // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)
static uint16_t buffer_offset; // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)
static uint16_t buffer_offset_b; // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)

void VL53LMZTextSensor::dump_config() {
  LOG_TEXT_SENSOR("", "VL53LMZTextSensor", this);
  switch (this->output_formatting_) {
    case VL53LMZ_CSV:
      ESP_LOGCONFIG(TAG, "  Output Formatting: CSV");
      break;
    case VL53LMZ_TABLE:
      ESP_LOGCONFIG(TAG, "  Output Formatting: TABLE");
      break;
    case VL53LMZ_BINARY:
      ESP_LOGCONFIG(TAG, "  Output Formatting: BINARY");
      break;
    case VL53LMZ_PNG:
      ESP_LOGCONFIG(TAG, "  Output Formatting: PNG");
      break;
    default:
      ESP_LOGCONFIG(TAG, "  Output Formatting: unknown");
  }
  switch (this->output_data_) {
    case VL53LMZ_RAW_DISTANCE:
      ESP_LOGCONFIG(TAG, "  Output Data: RAW DISTANCE");
      break;
    case VL53LMZ_FLOAT_DISTANCE:
      ESP_LOGCONFIG(TAG, "  Output Data: FLOAT DISTANCE");
      break;
    case VL53LMZ_RAW_SIGMA:
      ESP_LOGCONFIG(TAG, "  Output Data: RAW SIGMA");
      break;
    case VL53LMZ_FLOAT_SIGMA:
      ESP_LOGCONFIG(TAG, "  Output Data: FLOAT SIGMA");
      break;
    case VL53LMZ_REFLECTANCE:
      ESP_LOGCONFIG(TAG, "  Output Data: REFLECTANCE");
      break;
    case VL53LMZ_TARGET_COUNT:
      ESP_LOGCONFIG(TAG, "  Output Data: TARGET COUNT");
      break;
    case VL53LMZ_STATUS:
      ESP_LOGCONFIG(TAG, "  Output Data: STATUS");
      break;
    default:
      ESP_LOGCONFIG(TAG, "  Output Data: unknown");
  }
  ESP_LOGCONFIG(TAG, "  Flip X: %s", this->flip_x_ ? "true" : "false");
  ESP_LOGCONFIG(TAG, "  Flip Y: %s", this->flip_y_ ? "true" : "false");
}

void VL53LMZTextSensor::on_update(const VL53LMZ_ResultsData *result_data, const VL53LMZ_Resolution_Detail resolution) {
  switch (this->output_formatting_) {
    case VL53LMZ_CSV:
      output_visual_(result_data, resolution, ',', 0, "\n", false);
      break;
    case VL53LMZ_TABLE:
      output_visual_(result_data, resolution, '|', '-', "\n", true);
      break;
    case VL53LMZ_BINARY:
      output_binary_(result_data, resolution, false);
      break;
    case VL53LMZ_PNG:
      output_binary_(result_data, resolution, true);
      break;
    default:
      ESP_LOGW(TAG, "Invalid output formatting.");
      return;
  }
}

void VL53LMZTextSensor::output_visual_(const VL53LMZ_ResultsData *result_data, const VL53LMZ_Resolution_Detail resolution, const char separator,
                                        const char row_spacer, const std::string &eol_sequence,
                                        const bool skip_separator_eol) {
  uint8_t symbol_length = 4;

  buffer_offset = 0;
  if (this->output_data_ == VL53LMZ_FLOAT_DISTANCE)
    symbol_length = 5;
  if (this->output_data_ == VL53LMZ_STATUS)
    symbol_length = 3;
    uint8_t row_length = (resolution.width * symbol_length) + (resolution.width - 1);
  if (!skip_separator_eol)
    row_length++;

  for (uint16_t i = 0; i < resolution.total; i++) {
    if ((i % resolution.width) != 0) {
      buffer[buffer_offset] = separator;
      buffer_offset++;
    } else if (i > 0) {
      if (!skip_separator_eol) {
        buffer[buffer_offset] = separator;
        buffer_offset++;
      }
      if ((row_spacer != 0) && (i < (resolution.total - 1))) {
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
      case VL53LMZ_FLOAT_DISTANCE:
        mini_buffer_length = sprintf(mini_buffer, "%.3f", convert_distance(result_data, transform_(i, resolution)));
        break;
      case VL53LMZ_RAW_DISTANCE:
        mini_buffer_length = sprintf(mini_buffer, "%d", result_data->distance_mm[transform_(i, resolution)]);
        break;
      case VL53LMZ_FLOAT_SIGMA:
        mini_buffer_length = sprintf(mini_buffer, "%.3f", convert_sigma(result_data, transform_(i, resolution)));
        break;
      case VL53LMZ_RAW_SIGMA:
        mini_buffer_length = sprintf(mini_buffer, "%d", result_data->range_sigma_mm[transform_(i, resolution)]);
        break;
      case VL53LMZ_REFLECTANCE:
        mini_buffer_length = sprintf(mini_buffer, "%d", result_data->reflectance[transform_(i, resolution)]);
        break;
      case VL53LMZ_TARGET_COUNT:
        mini_buffer_length = sprintf(mini_buffer, "%d", result_data->nb_target_detected[transform_(i, resolution)]);
        break;
      case VL53LMZ_STATUS:
        mini_buffer_length = sprintf(mini_buffer, "%d", result_data->target_status[transform_(i, resolution)]);
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

void VL53LMZTextSensor::output_binary_(const VL53LMZ_ResultsData *result_data, VL53LMZ_Resolution_Detail resolution, bool as_png) {
  MemFloat tmp_f{};
  MemShort tmp_s{};
  MemByte tmp_b{};
  buffer_offset = 0;
  uint32_t pixel_format = 0;
  for (uint8_t i = 0; i < resolution.total; i++) {
    switch (this->output_data_) {
      case VL53LMZ_FLOAT_DISTANCE:
        if (as_png) {
          tmp_s.u = result_data->distance_mm[transform_(i, resolution)];
          if (tmp_s.u > 4093)
            tmp_s.u = 4093;
          tmp_s.u = tmp_s.u << 4;
          std::memcpy(buffer + buffer_offset, tmp_s.b, 2);
          buffer_offset += 2;
          pixel_format = UNCOMPNG__PIXEL_FORMAT__Y_16LE;
          break;
        }
        tmp_f.f = convert_distance(result_data, transform_(i, resolution));
        std::memcpy(buffer + buffer_offset, tmp_f.b, 4);
        buffer_offset += 4;
        break;
      case VL53LMZ_RAW_DISTANCE:
        tmp_s.u = result_data->distance_mm[transform_(i, resolution)];
        std::memcpy(buffer + buffer_offset, tmp_s.b, 2);
        buffer_offset += 2;
        pixel_format = UNCOMPNG__PIXEL_FORMAT__Y_16LE;
        break;
      case VL53LMZ_FLOAT_SIGMA:
        if (as_png) {
          tmp_s.u = result_data->range_sigma_mm[transform_(i, resolution)];
          if (tmp_s.u > 4093)
            tmp_s.u = 4093;
          tmp_s.u = tmp_s.u << 4;
          std::memcpy(buffer + buffer_offset, tmp_s.b, 2);
          buffer_offset += 2;
          pixel_format = UNCOMPNG__PIXEL_FORMAT__Y_16LE;
          break;
        }
        tmp_f.f = convert_sigma(result_data, transform_(i, resolution));
        std::memcpy(buffer + buffer_offset, tmp_f.b, 4);
        buffer_offset += 4;
        break;
      case VL53LMZ_RAW_SIGMA:
        tmp_s.u = result_data->range_sigma_mm[transform_(i, resolution)];
        std::memcpy(buffer + buffer_offset, tmp_s.b, 2);
        buffer_offset += 2;
        pixel_format = UNCOMPNG__PIXEL_FORMAT__Y_16LE;
        break;
      case VL53LMZ_REFLECTANCE:
        tmp_b.u = result_data->reflectance[transform_(i, resolution)];
        std::memcpy(buffer + buffer_offset, tmp_b.b, 1);
        buffer_offset += 1;
        pixel_format = UNCOMPNG__PIXEL_FORMAT__Y;
        break;
      case VL53LMZ_TARGET_COUNT:
        tmp_b.u = result_data->nb_target_detected[transform_(i, resolution)];
        std::memcpy(buffer + buffer_offset, tmp_b.b, 1);
        buffer_offset += 1;
        pixel_format = UNCOMPNG__PIXEL_FORMAT__Y;
        break;
      case VL53LMZ_STATUS:
        tmp_b.u = result_data->target_status[transform_(i, resolution)];
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
    uncompng__encode(&png_write_func, nullptr, reinterpret_cast<uint8_t *>(buffer), buffer_offset, resolution.width,
                     resolution.height, resolution.width * bpp, pixel_format);

    this->publish_state(base64_encode(reinterpret_cast<uint8_t *>(buffer) + 256, buffer_offset_b - 256));
    return;
  }

  this->publish_state(base64_encode(reinterpret_cast<uint8_t *>(buffer), buffer_offset));
}

float VL53LMZTextSensor::convert_distance(const VL53LMZ_ResultsData *result_data, const uint8_t zone) {
  if (const uint8_t status = result_data->target_status[zone]; status == 5 || status == 6 || status == 9) {
    return static_cast<float>(result_data->distance_mm[zone]) / 1000.0f;
  }
  return NAN;
}

float VL53LMZTextSensor::convert_sigma(const VL53LMZ_ResultsData *result_data, const uint8_t zone) {
  return static_cast<float>(result_data->range_sigma_mm[zone]) / 1000.0f;
}

uint8_t VL53LMZTextSensor::transform_(const uint8_t zone, const VL53LMZ_Resolution_Detail resolution) const {
  if ((!this->flip_x_) && (!this->flip_y_))
    return zone;
  uint8_t pos_x = zone % resolution.width;
  uint8_t pos_y = zone / resolution.width;

  if (this->flip_x_) {
    pos_x = resolution.width - pos_x - 1;
  }
  if (this->flip_y_) {
    pos_y = resolution.height - pos_y - 1;
  }

  return (pos_y * resolution.width) + pos_x;
}
}  // namespace esphome::vl53l_mz
