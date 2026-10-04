#pragma once

#include "esphome/core/component.h"
#include "esphome/components/cover/cover.h"
#include "esphome/components/button/button.h"
#include "esphome/components/cc1101/cc1101.h"

// External libraries
#include <NVSRollingCodeStorage.h>
#include <SomfyRemote.h>
#include <memory>

#define NVS_ROLLING_CODE_STORAGE "somfy_cover"

namespace esphome {
namespace somfy_cover {

// Standalone time-based Somfy RTS cover.
// ESPHome's time_based::TimeBasedCover is now `final`, so the position tracking
// it used to provide is implemented here directly.
class SomfyCover : public cover::Cover, public Component {
 public:
  void setup() override;
  void loop() override;
  void dump_config() override;
  float get_setup_priority() const override { return setup_priority::DATA; }

  // Setter methods
  void set_cc1101_module(cc1101::CC1101 *module) { this->cc1101_module_ = module; }
  void set_prog_button(button::Button *btn) { this->cover_prog_button_ = btn; }
  void set_remote_code(uint32_t code) { this->remote_code_ = code; }
  void set_storage_key(const char *key) { this->storage_key_ = key; }
  void set_repeat_count(int repeat) { this->repeat_count_ = repeat; }
  void set_open_duration(uint32_t duration) { this->open_duration_ = duration; }
  void set_close_duration(uint32_t duration) { this->close_duration_ = duration; }

  cover::CoverTraits get_traits() override;

 protected:
  void control(const cover::CoverCall &call) override;

  // Position tracking
  void start_direction_(cover::CoverOperation dir);
  void recompute_position_();
  bool is_at_target_() const;

  // Radio
  void program_();
  void send_command_(Command command);

  // Configuration
  cc1101::CC1101 *cc1101_module_{nullptr};
  button::Button *cover_prog_button_{nullptr};
  uint32_t remote_code_{0};
  const char *storage_key_{nullptr};
  int repeat_count_{4};
  uint32_t open_duration_{0};
  uint32_t close_duration_{0};

  // Hardware / storage
  std::unique_ptr<SomfyRemote> remote_;
  std::unique_ptr<NVSRollingCodeStorage> storage_;

  // Movement state
  uint32_t last_recompute_time_{0};
  uint32_t last_publish_time_{0};
  float target_position_{0};
  cover::CoverOperation last_operation_{cover::COVER_OPERATION_OPENING};
};

}  // namespace somfy_cover
}  // namespace esphome
