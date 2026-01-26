#pragma once

#include "esphome/core/automation.h"
#include "esphome/core/component.h"
#include "esphome/core/entity_base.h"
#include "esphome/components/time_based/time_based_cover.h"
#include "esphome/components/button/button.h"
#include "esphome/components/cc1101/cc1101.h"


// Libraries for SomfyRemote
#include <NVSRollingCodeStorage.h>
#include <SomfyRemote.h>

#define COVER_OPEN 1.0f
#define COVER_CLOSED 0.0f

#define NVS_ROLLING_CODE_STORAGE "somfy_cover"

namespace esphome {
namespace somfy_cover {

// Helper class to attach cover functions to the time based cover triggers
template<typename... Ts> class SomfyCoverAction : public Action<Ts...> {
 public:
  // The function to be called when the action plays.
  std::function<void(Ts...)> callback;

  explicit SomfyCoverAction(std::function<void(Ts...)> callback) : callback(callback) {}

  void play(Ts... x) override {
    if (callback)
      callback(x...);
  }
};

class SomfyCover : public time_based::TimeBasedCover {
 public:
  void setup() override;
  void loop() override;
  void dump_config() override;

  // Set time based cover values
  void set_open_duration(uint32_t open_duration) { this->open_duration_ = open_duration; }
  void set_close_duration(uint32_t close_duration) { this->close_duration_ = close_duration; }

  // Set cc1101 module
  void set_cc1101_module(cc1101::CC1101 *cc1101_module) { this->cc1101_module_ = cc1101_module; }

  // Set somfy cover button and value
  void set_prog_button(button::Button *cover_prog_button) { this->cover_prog_button_ = cover_prog_button; }
  void set_remote_code(uint32_t remote_code_) { this->remote_code_ = remote_code_; }
  void set_storage_key(const char *storage_key_) { this->storage_key_ = storage_key_; }
  void set_repeat_count(int repeat_count_) { this->repeat_count_ = repeat_count_; }

  cover::CoverTraits get_traits() override;

 protected:
  void control(const cover::CoverCall &call) override;

  // Set via the ESPHome yaml
  cc1101::CC1101 *cc1101_module_{nullptr};
  button::Button *cover_prog_button_{nullptr};
  uint32_t remote_code_{0};
  const char *storage_key_;
  int repeat_count_{4};  // Number of times to repeat the command

  // Set via the constructor
  SomfyRemote *remote_;
  NVSRollingCodeStorage *storage_;

  void open();
  void close();
  void stop();
  void program();

  // Create automations to attach the cover control functions
  Automation<> *automationTriggerUp_;
  SomfyCoverAction<> *actionTriggerUp;
  Automation<> *automationTriggerDown_;
  SomfyCoverAction<> *actionTriggerDown_;
  Automation<> *automationTriggerStop_;
  SomfyCoverAction<> *actionTriggerStop_;

  void send_command(Command command);
};

extern bool cc1101I_initialized;

}  // namespace somfy_cover
}  // namespace esphome
