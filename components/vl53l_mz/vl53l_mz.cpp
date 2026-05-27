#include "vl53l_mz.h"
#include "esphome/core/log.h"

#include <cinttypes>

#if defined(VL53L_MZ_RUN_XTALK_CALIBRATION) || defined(VL53L_MZ_SET_XTALK_CALIBRATION_DATA)
#include "esphome/core/helpers.h"
#endif

namespace esphome::vl53l_mz {
static const auto TAG = "vl53l_mz";

std::list<VL53LMZ *> VL53LMZ::vl53_devices;  // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)
bool VL53LMZ::lp_pin_setup_complete = false;  // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)
uint8_t status{255};                           // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)

VL53LMZ::VL53LMZ() {
  for (auto *device : vl53_devices) {
    if (device->address_ == this->address_) {
      ESP_LOGE(TAG, "Duplicate I2C address 0x%02X detected.", this->address_);
      this->mark_failed();
      return;
    }
  }
  VL53LMZ::vl53_devices.push_back(this);
}

void VL53LMZ::dump_config() {
  ESP_LOGCONFIG(TAG, "VL53LMZ:");
  if (this->is_failed()) {
    ESP_LOGE(TAG, ESP_LOG_MSG_COMM_FAIL);
  }
  LOG_UPDATE_INTERVAL(this);
  LOG_I2C_DEVICE(this);
  if (this->reset_pin_ != nullptr) {
    LOG_PIN("  Reset Pin: ", this->reset_pin_);
  }
  if (this->int_pin_ != nullptr) {
    LOG_PIN("  Interrupt Pin: ", this->int_pin_);
  }
  if (this->lp_pin_ != nullptr) {
    LOG_PIN("  LP Pin: ", this->lp_pin_);
  }
  switch (this->resolution_) {
    case VL53LMZ_4X4:
      ESP_LOGCONFIG(TAG, "  Resolution: 4X4");
      break;
    case VL53LMZ_8X8:
      ESP_LOGCONFIG(TAG, "  Resolution: 8X8");
      break;
    default:
      ESP_LOGCONFIG(TAG, "  Resolution: unknown");
  }
  ESP_LOGCONFIG(TAG, "  Ranging Frequency: %" PRIu32 " Hz", this->ranging_frequency_);
  switch (this->ranging_mode_) {
    case VL53LMZ_CONTINUOUS:
      ESP_LOGCONFIG(TAG, "  Ranging Mode: CONTINUOUS");
      break;
    case VL53LMZ_AUTO:
      ESP_LOGCONFIG(TAG, "  Ranging Mode: AUTO");
      break;
    default:
      ESP_LOGCONFIG(TAG, "  Ranging Mode: unknown");
  }
  ESP_LOGCONFIG(TAG, "  Integration Time: %" PRIu32 " ms", this->integration_time_ / 1000);
  ESP_LOGCONFIG(TAG, "  Sharpening: %d %%", this->sharpening_);
  switch (this->target_order_) {
    case VL53LMZ_STRONGEST:
      ESP_LOGCONFIG(TAG, "  Target Order: STRONGEST");
      break;
    case VL53LMZ_CLOSEST:
      ESP_LOGCONFIG(TAG, "  Target Order: CLOSEST");
      break;
    default:
      ESP_LOGCONFIG(TAG, "  Target Order: unknown");
  }
#ifdef VL53L_MZ_RUN_XTALK_CALIBRATION
  ESP_LOGCONFIG(TAG, "  Run Xtalk Calibration: true");
  ESP_LOGCONFIG(TAG, "  Xtalk Calibration Reflectance: %" PRIu32 " %%", this->xtalk_calibration_reflectance_);
  ESP_LOGCONFIG(TAG, "  Xtalk Calibration Distance: %" PRIu32 " mm", this->xtalk_calibration_distance_);
#else
  ESP_LOGCONFIG(TAG, "  Run Xtalk Calibration: false");
#endif
#ifdef VL53L_MZ_SET_XTALK_CALIBRATION_DATA
  if (this->xtalk_calibration_data_ != nullptr) {
    ESP_LOGCONFIG(TAG, "  Xtalk Calibration Data: %s", this->xtalk_calibration_data_);
  }
#endif
}

void VL53LMZ::setup() {
  if (this->is_failed())
    return;
  if (!lp_pin_setup_complete) {
    delay(10);
    ESP_LOGD(TAG, "Preparing pins.");
    for (const auto &vl53_device : vl53_devices) {
      vl53_device->setup_pins_();
    }
    lp_pin_setup_complete = true;
  }

  ESP_LOGD(TAG, "Checking API.");
  if (this->api_ == nullptr) {
    fail_("Missing API wrapper: ", 255);
    return;
  }

  uint8_t is_alive = false;
  if (this->lp_pin_ != nullptr) {
    ESP_LOGD(TAG, "Initializing I2C address.");
    // Pull the lp pin high (enable device)
    this->lp_pin_->digital_write(true);
    delayMicroseconds(100);
    this->hw_reset_();
    status = this->api_->is_alive(&is_alive);
    if (!is_alive) {
      ESP_LOGD(TAG, "Switching device address to: 0x%02X", this->address_);
      // Use the default address to set the wanted address to use before
      // switching back.
      const uint8_t final_address = this->address_;
      this->set_i2c_address(api_->get_default_i2c_address());

      status = this->api_->set_i2c_address(final_address << 1);
      if (status) {
        fail_("Failed to set I2C address: 0x%02X", status);
        return;
      }
      this->set_i2c_address(final_address);
    }
  } else {
    this->hw_reset_();
  }

  ESP_LOGD(TAG, "Checking connection.");
  status = this->api_->is_alive(&is_alive);
  if (!is_alive) {
    fail_("Device not detected at requested address: 0x%02X", this->address_);
    return;
  }
  if (status) {
    fail_("Device detection failed: 0x%02X", status);
    return;
  }

  ESP_LOGD(TAG, "Initializing sensor.");
  status = this->api_->init();
  if (status) {
    fail_("ULD Loading failed: 0x%02X", status);
    return;
  }

  ESP_LOGD(TAG, "Checking readiness.");
  status = this->api_->check_data_ready(&this->is_ready_);
  if (status) {
    fail_("Failed to check readiness: 0x%02X", status);
    return;
  }

#if defined(VL53L_MZ_RUN_XTALK_CALIBRATION)
  ESP_LOGD(TAG, "Calibrating xtalk.");
  status = this->api_->calibrate_xtalk(this->xtalk_calibration_reflectance_, 16,
                                     this->xtalk_calibration_distance_);
  if (status) {
    fail_("Xtalk calibration failed: 0x%02X", status);
    return;
  }

  status = this->api_->get_caldata_xtalk(this->api_->get_xtalk_buffer());
  if (status) {
    fail_("Getting xtalk calibration data failed: 0x%02X", status);
    return;
  }
  ESP_LOGI(TAG, "Xtalk calibration data: %s", base64_encode(this->api_->get_xtalk_buffer(), this->api_->get_xtalk_buffer_size()).c_str());
#elif defined(VL53L_MZ_SET_XTALK_CALIBRATION_DATA)
  if (this->xtalk_calibration_data_ != nullptr) {
    ESP_LOGD(TAG, "Writing xtalk config.");
    size_t data_length = strlen(this->xtalk_calibration_data_);
    size_t decoded = base64_decode(reinterpret_cast<const uint8_t *>(this->xtalk_calibration_data_), data_length,
                                   this->api_->get_xtalk_buffer(), this->api_->get_xtalk_buffer_size());
    if (decoded < this->api_->get_xtalk_buffer_size()) {
      fail_("Xtalk calibration data to short: 0x%02X", 255);
      return;
    }
    status = this->api_->set_caldata_xtalk(this->api_->get_xtalk_buffer());
    if (status) {
      fail_("Setting xtalk calibration data failed: 0x%02X", status);
      return;
    }
  }
#endif

  ESP_LOGD(TAG, "Setting resolution.");
  status = this->api_->set_resolution(this->api_->to_api_resolution(this->resolution_));
  if (status) {
    fail_("Failed to set resolution: 0x%02X", status);
    return;
  }

  ESP_LOGD(TAG, "Setting ranging frequency.");
  status = this->api_->set_ranging_frequency_hz(this->ranging_frequency_);
  if (status) {
    fail_("Failed to set ranging frequency: 0x%02X", status);
    return;
  }

  ESP_LOGD(TAG, "Setting ranging mode.");
  this->api_->set_ranging_mode(this->api_->to_api_ranging_mode(this->ranging_mode_));
  if (status) {
    fail_("Failed to set ranging mode: 0x%02X", status);
    return;
  }

  ESP_LOGD(TAG, "Setting integration time.");
  status = this->api_->set_integration_time_ms(this->integration_time_ / 1000);
  if (status) {
    fail_("Failed to set integration time: 0x%02X", status);
    return;
  }

  ESP_LOGD(TAG, "Setting sharpening.");
  status = this->api_->set_sharpener_percent(this->sharpening_);
  if (status) {
    fail_("Failed to set sharpening percentage: 0x%02X", status);
    return;
  }

  ESP_LOGD(TAG, "Setting target order.");
  this->api_->set_target_order(this->api_->to_api_target_order(this->target_order_));
  if (status) {
    fail_("Failed to set target order: 0x%02X", status);
    return;
  }

  ESP_LOGD(TAG, "Setting temperature compensation.");
  status = this->api_->set_vhv_repeat_count(this->ranging_frequency_ * 60);
  if (status) {
    fail_("Failed to set temperature compensation: 0x%02X", status);
    return;
  }

  this->device_initiated_ = true;
  ESP_LOGD(TAG, "Force stopping ranging.");
  this->start_ranging();
  this->stop_ranging();  // Workaround to prevent vl53_mz_check_data_ready returning 0x85

  // Check if all sensors have been initialized and start continuous update for those that need it
  if (all_sensors_initialized()) {
    ESP_LOGD(TAG, "All sensors initialized, starting continuous update for sensors that require it.");
    for (auto *device : vl53_devices) {
      if (device->continuous_update_ && device->device_initiated_) {
        device->stop_poller();
        device->start_ranging();
      }
    }
  }
}

void VL53LMZ::update() {
  if (this->continuous_update_) {
    return;
  }
  this->start_ranging();
}

void VL53LMZ::loop() {
  if (!this->device_initiated_) {
    return;
  }
  if (this->initiated_read_) {
    if (this->int_pin_ != nullptr) {
      if (this->interrupt_triggered_) {
        this->interrupt_triggered_ = false;
        this->process_ranging_data_();
      }
    } else {
      status = this->api_->check_data_ready(&this->is_ready_);
      if (status) {
        ESP_LOGW(TAG, "Data readiness check failed: %x", status);
        this->stop_ranging();
        this->start_ranging();
        return;
      }
      if (this->is_ready_) {
        this->process_ranging_data_();
      }
    }
  }
}

void VL53LMZ::process_ranging_data_() {
  status = this->api_->get_ranging_data();
  if (status) {
    ESP_LOGW(TAG, "Failed to get ranging data: %x", status);
    return;
  }
  for (auto *sensor : this->sensors_) {
    sensor->on_update(this->api_->get_results(), VL53LMZ_RESOLUTIONS[this->resolution_]);
  }
  if (!this->continuous_update_) {
    this->stop_ranging();
  }
}

void VL53LMZ::start_ranging() {
  if (!this->device_initiated_) {
    return;
  }
  if (this->initiated_read_) {
    return;
  }
  if (this->lp_pin_ != nullptr) {
    // Pull the lp pin high (enable device)
    this->lp_pin_->digital_write(true);
    delayMicroseconds(100);
  }
  status = this->api_->set_power_mode_wakeup();
  if (status) {
    ESP_LOGW(TAG, "Failed to set wake power mode: %x", status);
    return;
  }
  status = this->api_->start_ranging();
  if (status) {
    ESP_LOGW(TAG, "Failed to start ranging: %x", status);
    return;
  }
  ESP_LOGV(TAG, "Started Ranging");
  this->initiated_read_ = true;
}

void VL53LMZ::stop_ranging() {
  if (!this->device_initiated_) {
    return;
  }
  this->initiated_read_ = false;
  status = this->api_->stop_ranging();
  if (status) {
    ESP_LOGW(TAG, "Failed to stop ranging: %x", status);
    return;
  }
  status = this->api_->set_power_mode_sleep();
  if (status) {
    ESP_LOGW(TAG, "Failed to set sleep power mode: %x", status);
    return;
  }
  if (this->lp_pin_ != nullptr) {
    // Pull the lp pin low (switch to low power mode)
    this->lp_pin_->digital_write(false);
  }
  ESP_LOGV(TAG, "Stopped Ranging");
}

bool VL53LMZ::all_sensors_initialized() {
  for (const auto *device : vl53_devices) {
    if (device->is_failed()) {
      continue;
    }
    if (!device->device_initiated_) {
      return false;
    }
  }
  return true;
}

void VL53LMZ::hw_reset_() const {
  if (this->reset_pin_ != nullptr) {
    ESP_LOGD(TAG, "Performing hardware reset.");
    this->reset_pin_->digital_write(true);
    delay(10);
    this->reset_pin_->digital_write(false);
    delay(10);
  }
}

void VL53LMZ::setup_pins_() {
  if (this->reset_pin_ != nullptr) {
    // Init reset pins
    this->reset_pin_->setup();
    this->reset_pin_->digital_write(false);
  }
  if (this->int_pin_ != nullptr) {
    this->int_pin_->setup();
    this->int_pin_->attach_interrupt(VL53LMZ::intr_handle, this, gpio::INTERRUPT_FALLING_EDGE);
  }
  if (this->lp_pin_ != nullptr) {
    // Init lp pins and power down device
    this->lp_pin_->setup();
    this->lp_pin_->digital_write(false);
  }
}

void VL53LMZ::intr_handle(VL53LMZ *arg) {
  arg->interrupt_triggered_ = true;
}

void VL53LMZ::fail_(const char *message, const uint8_t data) {
  if (this->lp_pin_ != nullptr) {
    this->lp_pin_->digital_write(false);
  }
  ESP_LOGE(TAG, message, data);
  this->mark_failed();
}

}  // namespace esphome::vl53l_mz
