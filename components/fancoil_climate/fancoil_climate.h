#pragma once

#include <cmath>
#include <vector>
#include "esphome/core/component.h"
#include "esphome/components/climate/climate.h"

namespace esphome::fancoil_climate {

// Regiszterek (0-bázis):
//   0   szobahő x0.1
//   104 státusz: bit1 hő-/hűtési igény
//   201 ventilátor bit0-1 (0 auto, 1 min, 2 éjszakai, 3 max), bit4 keylock, bit7 standby
//   231 alapjel x0.1
//   233 üzemmód: 0 auto, 3 fűtés, 5 hűtés
class FancoilClimate : public climate::Climate, public Component {
 public:
  float get_setup_priority() const override { return setup_priority::DATA; }
  void dump_config() override;

  /// A lambda hívja minden beolvasott regiszter után.
  void on_reg(uint16_t reg, int16_t value);
  /// A lambda hívja: ha van kiírandó regiszter, (reg << 16) | érték.
  bool pop_write(uint32_t &out);

 protected:
  climate::ClimateTraits traits() override;
  void control(const climate::ClimateCall &call) override;

  void queue_write_(uint16_t reg, uint16_t value);
  bool held_(uint16_t reg) const;
  void update_state_(bool force);

  float room_{NAN};
  int r104_{-1};
  int r201_{-1};
  int r231_{-1};
  int r233_{-1};

  // A saját írás után néhány másodpercig a régi (még kiírás előtti)
  // beolvasott értéket figyelmen kívül hagyjuk, hogy ne villogjon vissza.
  uint32_t hold_201_{0}, hold_231_{0}, hold_233_{0};

  std::vector<uint32_t> writes_;
  bool published_once_{false};
};

}  // namespace esphome::fancoil_climate
