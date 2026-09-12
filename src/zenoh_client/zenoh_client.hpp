#pragma once

#include <zenoh-pico.h>
#include <zephyr/kernel.h>

#include <atomic>

namespace zenoh_client
{

class ZenohClient
{
 public:
  ZenohClient();

  void start();

 private:
  static void open_session(struct k_work *work);

  struct k_work open_work_;
  std::atomic<bool> start_requested_{false};
  z_owned_session_t session_{};
};

}  // namespace zenoh_client
