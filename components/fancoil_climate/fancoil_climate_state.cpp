#include "fancoil_climate.h"
#include "esphome/core/hal.h"
#include "esphome/core/log.h"

namespace esphome::fancoil_climate {

using namespace climate;

void FancoilClimate::update_state_(bool force) {
  if (this->r201_ < 0 || this->r233_ < 0)
    return;

  ClimateMode mode;
  if (this->r201_ & 0x80) mode = CLIMATE_MODE_OFF;
  else if (this->r233_ == 3) mode = CLIMATE_MODE_HEAT;
  else if (this->r233_ == 5) mode = CLIMATE_MODE_COOL;
  else mode = CLIMATE_MODE_HEAT_COOL;

  const float target = this->r231_ >= 0 ? this->r231_ * 0.1f : NAN;

  const int fan = this->r201_ & 0x03;
  ClimatePreset preset = fan == 2 ? CLIMATE_PRESET_SLEEP : CLIMATE_PRESET_NONE;
  optional<ClimateFanMode> fan_mode;
  if (fan == 1) fan_mode = CLIMATE_FAN_LOW;
  else if (fan == 3) fan_mode = CLIMATE_FAN_HIGH;
  else if (fan == 0) fan_mode = CLIMATE_FAN_AUTO;
  // fan == 2 (Night): nincs fan_mode, mint az Innova AirLeaf-nél

  ClimateAction action = CLIMATE_ACTION_IDLE;
  const bool demand = this->r104_ >= 0 && (this->r104_ & 0x0002);
  if (mode == CLIMATE_MODE_OFF) {
    action = CLIMATE_ACTION_OFF;
  } else if (demand) {
    if (mode == CLIMATE_MODE_COOL)
      action = CLIMATE_ACTION_COOLING;
    else if (mode == CLIMATE_MODE_HEAT_COOL && !std::isnan(target) && !std::isnan(this->room_) && this->room_ > target)
      action = CLIMATE_ACTION_COOLING;
    else
      action = CLIMATE_ACTION_HEATING;
  }

  auto same_f = [](float a, float b) { return (std::isnan(a) && std::isnan(b)) || a == b; };
  const bool changed = !this->published_once_ || this->mode != mode || !same_f(this->target_temperature, target) ||
                       !same_f(this->current_temperature, this->room_) || this->action != action ||
                       this->fan_mode != fan_mode || this->preset != preset;
  if (!changed && !force)
    return;

  this->mode = mode;
  this->target_temperature = target;
  this->current_temperature = this->room_;
  this->action = action;
  this->fan_mode = fan_mode;
  this->preset = preset;
  this->published_once_ = true;
  this->publish_state();
}

}  // namespace esphome::fancoil_climate
