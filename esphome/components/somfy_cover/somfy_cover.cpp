#include "esphome/core/log.h"
#include "somfy_cover.h"

namespace esphome {
namespace somfy_cover {

static const char *TAG = "somfy_cover.cover";

void SomfyCover::setup() {
  ESP_LOGCONFIG(TAG, "Setting up Somfy Cover '%s'...", this->get_name().c_str());

  // Use member initialisation if possible, or ensure these are deleted in the destructor
  this->storage_ = std::make_unique<NVSRollingCodeStorage>(NVS_ROLLING_CODE_STORAGE, this->storage_key_);

  this->remote_ = std::make_unique<SomfyRemote>(
      this->cc1101_module_->get_emitter_pin(), 
      this->remote_code_, 
      this->storage_.get()
  );
  this->remote_->setup();

  // Optimized Automation Triggers
  auto setup_trigger = [this](Trigger<> *trigger, Command cmd) {
    auto *action = new SomfyCoverAction<>([this, cmd] { this->send_command(cmd); });
    auto *automation = new Automation<>(trigger);
    automation->add_action(action);
  };

  setup_trigger(this->get_open_trigger(), Command::Up);
  setup_trigger(this->get_close_trigger(), Command::Down);
  setup_trigger(this->get_stop_trigger(), Command::My);

  // Attach the prog button callback
  if (this->cover_prog_button_ != nullptr) {
    this->cover_prog_button_->add_on_press_callback([this] { this->program(); });
  }

  this->has_built_in_endstop_ = true;
  this->assumed_state_ = true;

  TimeBasedCover::setup();
}

void SomfyCover::loop() { 
  TimeBasedCover::loop(); 
}

void SomfyCover::dump_config() {
  LOG_COVER(TAG, "Somfy Cover", this);
  ESP_LOGCONFIG(TAG, "  Remote Code: 0x%06X", this->remote_code_);
}

cover::CoverTraits SomfyCover::get_traits() {
  auto traits = TimeBasedCover::get_traits();
  traits.set_supports_tilt(false);
  return traits;
}

void SomfyCover::control(const cover::CoverCall &call) { 
  TimeBasedCover::control(call); 
}

void SomfyCover::open() {
  ESP_LOGI(TAG, "'%s': Sending OPEN command", this->get_name().c_str());
  this->send_command(Command::Up);
}

void SomfyCover::close() {
  ESP_LOGI(TAG, "'%s': Sending CLOSE command", this->get_name().c_str());
  this->send_command(Command::Down);
}

void SomfyCover::stop() {
  ESP_LOGI(TAG, "'%s': Sending STOP command", this->get_name().c_str());
  this->send_command(Command::My);
}

void SomfyCover::program() {
  ESP_LOGW(TAG, "'%s': Sending PROG command", this->get_name().c_str());
  this->send_command(Command::Prog);
}

void SomfyCover::send_command(Command command) {
  // Queue the command through the CC1101 module to prevent SPI collisions
  this->cc1101_module_->sent_command([this, command] { 
    this->remote_->sendCommand(command, this->repeat_count_); 
  });
}

}  // namespace somfy_cover
}  // namespace esphome
