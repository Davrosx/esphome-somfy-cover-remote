#include "somfy_cover.h"
#include "esphome/core/application.h"
#include "esphome/core/hal.h"
#include "esphome/core/helpers.h"
#include "esphome/core/log.h"

namespace esphome {
namespace somfy_cover {

static const char *const TAG = "somfy_cover.cover";

using namespace esphome::cover;

void SomfyCover::setup() {
  this->storage_ = std::make_unique<NVSRollingCodeStorage>(NVS_ROLLING_CODE_STORAGE, this->storage_key_);
  this->remote_ = std::make_unique<SomfyRemote>(this->cc1101_module_->get_emitter_pin(), this->remote_code_,
                                                this->storage_.get());
  this->remote_->setup();

  if (this->cover_prog_button_ != nullptr) {
    this->cover_prog_button_->add_on_press_callback([this] { this->program_(); });
  }

  auto restore = this->restore_state_();
  if (restore.has_value()) {
    restore->apply(this);
  } else {
    this->position = 0.5f;
  }
}

void SomfyCover::loop() {
  if (this->current_operation == COVER_OPERATION_IDLE)
    return;

  const uint32_t now = App.get_loop_component_start_time();
  this->recompute_position_();

  if (this->is_at_target_()) {
    if (this->target_position_ == COVER_OPEN || this->target_position_ == COVER_CLOSED) {
      // Somfy motors stop at their end limits by themselves. Sending "My" here
      // would make an idle blind move to its favourite position instead.
      this->current_operation = COVER_OPERATION_IDLE;
    } else {
      this->start_direction_(COVER_OPERATION_IDLE);
    }
    this->publish_state();
  }

  // Publish the estimated position every second while moving
  if (now - this->last_publish_time_ > 1000) {
    this->publish_state(false);
    this->last_publish_time_ = now;
  }
}

void SomfyCover::dump_config() {
  LOG_COVER("", "Somfy Cover", this);
  ESP_LOGCONFIG(TAG,
                "  Remote Code: 0x%06" PRIX32 "\n"
                "  Open Duration: %.1fs\n"
                "  Close Duration: %.1fs\n"
                "  Repeat Count: %d",
                this->remote_code_, this->open_duration_ / 1e3f, this->close_duration_ / 1e3f, this->repeat_count_);
}

CoverTraits SomfyCover::get_traits() {
  auto traits = CoverTraits();
  traits.set_supports_stop(true);
  traits.set_supports_position(true);
  traits.set_supports_toggle(true);
  traits.set_supports_tilt(false);
  traits.set_is_assumed_state(true);
  return traits;
}

void SomfyCover::control(const CoverCall &call) {
  if (call.get_stop()) {
    this->start_direction_(COVER_OPERATION_IDLE);
    this->publish_state();
  }

  if (call.get_toggle().has_value()) {
    if (this->current_operation != COVER_OPERATION_IDLE) {
      this->start_direction_(COVER_OPERATION_IDLE);
      this->publish_state();
    } else if (this->position == COVER_CLOSED || this->last_operation_ == COVER_OPERATION_CLOSING) {
      this->target_position_ = COVER_OPEN;
      this->start_direction_(COVER_OPERATION_OPENING);
    } else {
      this->target_position_ = COVER_CLOSED;
      this->start_direction_(COVER_OPERATION_CLOSING);
    }
  }

  auto pos_val = call.get_position();
  if (pos_val.has_value()) {
    const float pos = *pos_val;
    if (pos == this->position) {
      // Already there by our estimate. At an end limit, resend anyway in case the
      // estimate has drifted; the motor's own limit switch stops it.
      if (pos == COVER_OPEN || pos == COVER_CLOSED) {
        this->target_position_ = pos;
        this->current_operation = COVER_OPERATION_IDLE;  // force the command to be sent
        this->start_direction_(pos == COVER_CLOSED ? COVER_OPERATION_CLOSING : COVER_OPERATION_OPENING);
      }
    } else {
      this->target_position_ = pos;
      this->start_direction_(pos < this->position ? COVER_OPERATION_CLOSING : COVER_OPERATION_OPENING);
    }
  }
}

bool SomfyCover::is_at_target_() const {
  switch (this->current_operation) {
    case COVER_OPERATION_OPENING:
      return this->position >= this->target_position_;
    case COVER_OPERATION_CLOSING:
      return this->position <= this->target_position_;
    case COVER_OPERATION_IDLE:
    default:
      return true;
  }
}

void SomfyCover::start_direction_(CoverOperation dir) {
  if (dir == this->current_operation && dir != COVER_OPERATION_IDLE)
    return;

  this->recompute_position_();

  Command command;
  switch (dir) {
    case COVER_OPERATION_IDLE:
      ESP_LOGI(TAG, "'%s': Sending STOP (My)", this->get_name().c_str());
      command = Command::My;
      break;
    case COVER_OPERATION_OPENING:
      ESP_LOGI(TAG, "'%s': Sending OPEN (Up)", this->get_name().c_str());
      command = Command::Up;
      this->last_operation_ = dir;
      break;
    case COVER_OPERATION_CLOSING:
      ESP_LOGI(TAG, "'%s': Sending CLOSE (Down)", this->get_name().c_str());
      command = Command::Down;
      this->last_operation_ = dir;
      break;
    default:
      return;
  }

  this->current_operation = dir;
  this->last_recompute_time_ = millis();
  this->send_command_(command);
}

void SomfyCover::recompute_position_() {
  float dir;
  float duration;
  switch (this->current_operation) {
    case COVER_OPERATION_OPENING:
      dir = 1.0f;
      duration = this->open_duration_;
      break;
    case COVER_OPERATION_CLOSING:
      dir = -1.0f;
      duration = this->close_duration_;
      break;
    default:
      return;
  }

  const uint32_t now = millis();
  if (duration > 0) {
    this->position += dir * (now - this->last_recompute_time_) / duration;
    this->position = clamp(this->position, 0.0f, 1.0f);
  }
  this->last_recompute_time_ = now;
}

void SomfyCover::program_() {
  ESP_LOGW(TAG, "'%s': Sending PROG", this->get_name().c_str());
  this->send_command_(Command::Prog);
}

void SomfyCover::send_command_(Command command) {
  // Route through the CC1101 module so it is switched to TX for the transmission
  this->cc1101_module_->sent_command([this, command] { this->remote_->sendCommand(command, this->repeat_count_); });
}

}  // namespace somfy_cover
}  // namespace esphome
