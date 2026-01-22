#include "esphome/core/log.h"
#include "somfy_cover.h"

namespace esphome {
namespace somfy_cover {

static const char *TAG = "somfy_cover.cover";

// Default value
bool cc1101I_initialized = false;

void SomfyCover::setup() {
  // Setup cover rolling code storage
  this->storage_ = new NVSRollingCodeStorage(NVS_ROLLING_CODE_STORAGE, this->storage_key_);

  // Setup the Somfy Remote
  this->remote_ = new SomfyRemote(this->cc1101_module_->get_emitter_pin(), this->remote_code_, this->storage_);
  this->remote_->setup();

  // Attach to timebased cover controls
  automationTriggerUp_ = new Automation<>(this->get_open_trigger());
  actionTriggerUp = new SomfyCoverAction<>([=, this] { return this->open(); });
  automationTriggerUp_->add_action(actionTriggerUp);

  automationTriggerDown_ = new Automation<>(this->get_close_trigger());
  actionTriggerDown_ = new SomfyCoverAction<>([=, this] { return this->close(); });
  automationTriggerDown_->add_action(actionTriggerDown_);

  automationTriggerStop_ = new Automation<>(this->get_stop_trigger());
  actionTriggerStop_ = new SomfyCoverAction<>([=, this] { return this->stop(); });
  automationTriggerStop_->add_action(actionTriggerStop_);

  // Attach the prog button
  this->cover_prog_button_->add_on_press_callback([=, this] { return this->program(); });

  // Set extra settings
  this->has_built_in_endstop_ = true;
  this->assumed_state_ = true;

  TimeBasedCover::setup();
}

void SomfyCover::loop() { TimeBasedCover::loop(); }

void SomfyCover::dump_config() { ESP_LOGCONFIG(TAG, "Somfy cover"); }

cover::CoverTraits SomfyCover::get_traits() {
  auto traits = TimeBasedCover::get_traits();
  traits.set_supports_tilt(false);

  return traits;
}

void SomfyCover::control(const cover::CoverCall &call) { TimeBasedCover::control(call); }

void SomfyCover::open() {
  std::string command = "OPEN " + this->get_object_id_to();
  ESP_LOGI("somfy", command.c_str());
  this->send_command(Command::Up);
}

void SomfyCover::close() {
  std::string command = "CLOSE " + this->get_object_id_to();
  ESP_LOGI("somfy", command.c_str());
  this->send_command(Command::Down);
}

void SomfyCover::stop() {
  std::string command = "STOP " + this->get_object_id_to();
  ESP_LOGI("somfy", command.c_str());
  this->send_command(Command::My);
}

void SomfyCover::program() {
  std::string command = "PROG " + this->get_object_id_to();
  ESP_LOGI("somfy", command.c_str());
  this->send_command(Command::Prog);
}

void SomfyCover::send_command(Command command) {
  this->cc1101_module_->sent_command([=, this] { this->remote_->sendCommand(command, this->repeat_count_); });
}

}  // namespace somfy_cover
}  // namespace esphome
