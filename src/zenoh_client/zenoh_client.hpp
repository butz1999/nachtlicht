#pragma once

#include <zenoh-pico.h>
#include <zephyr/kernel.h>

#include <atomic>
#include <cstdint>

namespace zenoh_client
{

using ColorNextCallback = void (*)(void *context);

class ZenohClient
{
 public:
  ZenohClient();

  void initialize(ColorNextCallback color_next_callback, void *context);
  void start();
  void publish_color_changed(std::uint32_t sequence, const char *color);
  void publish_brightness_changed(std::uint32_t sequence, std::uint8_t brightness);

 private:
  static void open_session(struct k_work *work);
  static void handle_color_next(z_loaned_sample_t *sample, void *context);
  void publish_event(z_owned_publisher_t *publisher, const char *payload);

  struct k_work open_work_;
  std::atomic<bool> start_requested_{false};
  std::atomic<bool> events_ready_{false};
  ColorNextCallback color_next_callback_{nullptr};
  void *color_next_context_{nullptr};
  z_owned_session_t session_{};
  z_owned_subscriber_t color_next_subscriber_{};
  z_owned_publisher_t color_changed_publisher_{};
  z_owned_publisher_t brightness_changed_publisher_{};
};

}  // namespace zenoh_client
