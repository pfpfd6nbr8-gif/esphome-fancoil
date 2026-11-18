#include "fancoil_climate.h"
#include "esphome/core/hal.h"
#include "esphome/core/log.h"

namespace esphome::fancoil_climate {

using namespace climate;

static const char *const TAG = "fancoil_climate.control";

void FancoilClimate::control(const ClimateCall &call) {
  if (this->r201_ < 0 || this->r233_ < 0) {
    ESP_LOGW(TAG, "Még nincs adat a fancoilról, a parancs elveszett");
    return;
  }
  uint16_t r201 = (uint16_t) this->r201_;

  if (call.get_mode().has_value()) {
    const ClimateMode m = *call.get_mode();
    if (m == CLIMATE_MODE_OFF) {
      r201 |= 0x80;
    } else {
      r201 &= ~0x80;
      int v = 0;  // HEAT_COOL = gyári auto
      if (m == CLIMATE_MODE_HEAT) v = 3;
      else if (m == CLIMATE_MODE_COOL) v = 5;
      if (v != this->r233_) {
        this->r233_ = v;
        this->queue_write_(233, (uint16_t) v);
      }
    }
  }

  int fan_idx = -1;
  if (call.get_fan_mode().has_value()) {
    switch (*call.get_fan_mode()) {
      case CLIMATE_FAN_LOW: fan_idx = 1; break;
      case CLIMATE_FAN_HIGH: fan_idx = 3; break;
      default: fan_idx = 0; break;
    }
  }
  if (call.get_preset().has_value()) {
    if (*call.get_preset() == CLIMATE_PRESET_SLEEP)
      fan_idx = 2;
    else if (fan_idx < 0 && (r201 & 0x03) == 2)
      fan_idx = 0;  // Night ki → auto ventilátor
  }
  if (fan_idx >= 0)
    r201 = (r201 & ~0x07) | (uint16_t) fan_idx;

  if (r201 != (uint16_t) this->r201_) {
    this->r201_ = r201;
    this->queue_write_(201, r201);
  }

  if (call.get_target_temperature().has_value()) {
    float t = *call.get_target_temperature();
    int v = (int) lroundf(t * 2.0f) * 5;  // 0,5 °C-ra kerekítve, x0.1
    if (v < 50) v = 50;
    if (v > 400) v = 400;
    if (v != this->r231_) {
      this->r231_ = v;
      this->queue_write_(231, (uint16_t) v);
    }
  }

  this->update_state_(true);
}

}  // namespace esphome::fancoil_climate
