#pragma once

#include <zenoh-pico.h>
#include <zephyr/kernel.h>

#include <atomic>

namespace zenoh_client
{

using ColorNextCallback = void (*)(void *context);

class ZenohClient
{
 public:
  ZenohClient();

  void initialize(ColorNextCallback color_next_callback, void *context);
  void start();

 private:
  static void open_session(struct k_work *work);
  static void handle_color_next(z_loaned_sample_t *sample, void *context);

  struct k_work open_work_;
  std::atomic<bool> start_requested_{false};
  ColorNextCallback color_next_callback_{nullptr};
  void *color_next_context_{nullptr};
  z_owned_session_t session_{};
  z_owned_subscriber_t color_next_subscriber_{};
};

}  // namespace zenoh_client
