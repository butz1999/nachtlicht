#include <zephyr/kernel.h>
#include <zephyr/sys/atomic.h>
#include <zephyr/sys/printk.h>

#include <array>
#include <atomic>
#include <cstddef>
#include <cstdint>

#include "connectivity/connectivity.hpp"
#include "rgb_led/rgb_led.hpp"
#include "zenoh_client/zenoh_client.hpp"

namespace
{

// clang-format off
constexpr std::array<rgb_led::Color, 6> kTestColors{{
    { 255U,   0U,   0U },
    { 255U, 255U,   0U },
    {   0U, 255U,   0U },
    {   0U, 255U, 255U },
    {   0U,   0U, 255U },
    { 255U,   0U, 255U },
}};
// clang-format on

rgb_led::RgbLed led;
connectivity::Connectivity network;
zenoh_client::ZenohClient zenoh;
struct k_work_delayable color_work;
std::size_t color_index;

struct k_work render_work;

constexpr std::uint8_t kBrightnessOn = 32U;
constexpr std::uint8_t kBrightnessOff = 0U;
constexpr std::uint16_t kBrightnessMaximum = 255U;
std::atomic<std::uint8_t> brightness{kBrightnessOn};  // std::atomic for clean thread-safe implementation
struct k_timer brightness_timer;

rgb_led::Color current_color;

void render_color(struct k_work *work)
{
  ARG_UNUSED(work);
  rgb_led::Color c = current_color;
  const auto brightness_value = brightness.load();

  c.red = static_cast<std::uint8_t>((static_cast<std::uint16_t>(c.red) * brightness_value) / kBrightnessMaximum);
  c.green = static_cast<std::uint8_t>((static_cast<std::uint16_t>(c.green) * brightness_value) / kBrightnessMaximum);
  c.blue = static_cast<std::uint8_t>((static_cast<std::uint16_t>(c.blue) * brightness_value) / kBrightnessMaximum);

  const auto result = led.set(c);
  if (result != 0)
  {
    printk("RGB LED update failed: %d\n", result);
  }
}

void update_brightness(struct k_timer *timer)
{
  ARG_UNUSED(timer);
  const auto next_brightness = brightness.load() == kBrightnessOn ? kBrightnessOff : kBrightnessOn;
  brightness.store(next_brightness);
  k_work_submit(&render_work);
}

void update_color(struct k_work *work)
{
  ARG_UNUSED(work);
  current_color = kTestColors[color_index];
  color_index = (color_index + 1U) % kTestColors.size();
  k_work_schedule(&color_work, K_MSEC(1000));
  k_work_submit(&render_work);
}

void start_zenoh(void *context)
{
  auto *const client = static_cast<zenoh_client::ZenohClient *>(context);
  client->start();
}

}  // namespace

int main()
{
  printk("Hello World from nachtlicht!\n");

  const auto result = led.initialize();
  if (result != 0)
  {
    printk("RGB LED initialization failed: %d\n", result);
    return result;
  }

  printk("RGB LED test started.\n");
  k_work_init_delayable(&color_work, update_color);
  k_work_init(&render_work, render_color);
  k_timer_init(&brightness_timer, update_brightness, nullptr);

  k_work_schedule(&color_work, K_NO_WAIT);
  k_timer_start(&brightness_timer, K_MSEC(500), K_MSEC(500));

  const auto connectivity_result = network.initialize(start_zenoh, &zenoh);
  if (connectivity_result != 0)
  {
    printk("Connectivity initialization failed: %d\n", connectivity_result);
  }

  return 0;
}
