#include "fancoil_climate.h"
#include "esphome/core/hal.h"
#include "esphome/core/log.h"

namespace esphome::fancoil_climate {

static const char *const TAG = "fancoil_climate";
static const uint32_t HOLD_MS = 4000;

using namespace climate;

void FancoilClimate::dump_config() { LOG_CLIMATE("", "Fancoil climate (Modbus B32)", this); }

ClimateTraits FancoilClimate::traits() {
  auto t = ClimateTraits();
  t.add_feature_flags(CLIMATE_SUPPORTS_CURRENT_TEMPERATURE | CLIMATE_SUPPORTS_ACTION);
  t.set_supported_modes({CLIMATE_MODE_OFF, CLIMATE_MODE_HEAT, CLIMATE_MODE_COOL, CLIMATE_MODE_HEAT_COOL});
  t.set_supported_fan_modes({CLIMATE_FAN_AUTO, CLIMATE_FAN_LOW, CLIMATE_FAN_HIGH});
  t.set_supported_presets({CLIMATE_PRESET_NONE, CLIMATE_PRESET_SLEEP});
  t.set_visual_min_temperature(5.0f);
  t.set_visual_max_temperature(40.0f);
  t.set_visual_target_temperature_step(0.5f);
  t.set_visual_current_temperature_step(0.1f);
  return t;
}

bool FancoilClimate::held_(uint16_t reg) const {
  const uint32_t now = millis();
  switch (reg) {
    case 201: return hold_201_ != 0 && (int32_t) (hold_201_ - now) > 0;
    case 231: return hold_231_ != 0 && (int32_t) (hold_231_ - now) > 0;
    case 233: return hold_233_ != 0 && (int32_t) (hold_233_ - now) > 0;
  }
  return false;
}

void FancoilClimate::on_reg(uint16_t reg, int16_t value) {
  if (held_(reg))
    return;
  switch (reg) {
    case 0:   this->room_ = value * 0.1f; break;
    case 104: this->r104_ = (uint16_t) value; break;
    case 201: this->r201_ = (uint16_t) value; break;
    case 231: this->r231_ = value; break;
    case 233: this->r233_ = value; break;
    default: return;
  }
  this->update_state_(false);
}

bool FancoilClimate::pop_write(uint32_t &out) {
  if (this->writes_.empty())
    return false;
  out = this->writes_.front();
  this->writes_.erase(this->writes_.begin());
  return true;
}

void FancoilClimate::queue_write_(uint16_t reg, uint16_t value) {
  // ugyanarra a regiszterre csak a legutolsó írás maradjon a sorban
  for (auto it = this->writes_.begin(); it != this->writes_.end();) {
    if ((*it >> 16) == reg)
      it = this->writes_.erase(it);
    else
      ++it;
  }
  this->writes_.push_back(((uint32_t) reg << 16) | value);
  const uint32_t until = millis() + HOLD_MS;
  if (reg == 201) this->hold_201_ = until;
  if (reg == 231) this->hold_231_ = until;
  if (reg == 233) this->hold_233_ = until;
}

}  // namespace esphome::fancoil_climate
