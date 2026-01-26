#pragma once

#include "esphome/core/automation.h"
#include "esphome/core/component.h"
#include "esphome/components/time_based/time_based_cover.h"
#include "esphome/components/button/button.h"
#include "esphome/components/cc1101/cc1101.h"

// External libraries
#include <NVSRollingCodeStorage.h>
#include <SomfyRemote.h>
#include <memory>

#define NVS_ROLLING_CODE_STORAGE "somfy_cover"

namespace esphome {
namespace somfy_cover {

// Lightweight action helper
template<typename... Ts> class SomfyCoverAction : public Action<Ts...> {
 public:
  std::function<void(Ts...)> callback;
  explicit SomfyCoverAction(std::function<void(Ts...)> callback) : callback(std::move(callback)) {}
  void play(Ts... x) override { if (this->callback) this->callback(x...); }
};

class SomfyCover : public time_based::TimeBasedCover {
 public:
  void setup() override;
  void loop() override;
  void dump_config() override;

  // Setter Methods
  void set_cc1101_module(cc1101::CC1101 *module) { this->cc1101_module_ = module; }
  void set_prog_button(button::Button *btn) { this->cover_prog_button_ = btn; }
  void set_remote_code(uint32_t code) { this->remote_code_ = code; }
  void set_storage_key(const char *key) { this->storage_key_ = key; }
  void set_repeat_count(int repeat) { this->repeat_count_ = repeat; }

  cover::CoverTraits get_traits() override;

 protected:
  void control(const cover::CoverCall &call) override;
  
  // Logic Methods
  void open();
  void close();
  void stop();
  void program();
  void send_command(Command command);

  // Configuration members
  cc1101::CC1101 *cc1101_module_{nullptr};
  button::Button *cover_prog_button_{nullptr};
  uint32_t remote_code_{0};
  const char *storage_key_{nullptr};
  int repeat_count_{4};

  // Hardware/Storage Logic (Using smart pointers for safety)
  std::unique_ptr<SomfyRemote> remote_;
  std::unique_ptr<NVSRollingCodeStorage> storage_;

  // Using unique_ptr for Automations ensures they are cleaned up automatically
  std::unique_ptr<Automation<>> automationTriggerUp_;
  std::unique_ptr<Automation<>> automationTriggerDown_;
  std::unique_ptr<Automation<>> automationTriggerStop_;

  bool initialized_{false}; 
};

}  // namespace somfy_cover
}  // namespace esphome
