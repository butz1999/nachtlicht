#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

#include <array>
#include <cstddef>

#include "rgb_led/rgb_led.hpp"

namespace
{

constexpr std::array<rgb_led::Color, 3> kTestColors{{
    {255U, 0U, 0U},
    {0U, 255U, 0U},
    {0U, 0U, 255U},
}};

rgb_led::RgbLed led;
struct k_work_delayable color_work;
std::size_t color_index;

void update_color(struct k_work *work)
{
  ARG_UNUSED(work);

  const auto result = led.set(kTestColors[color_index]);
  if (result != 0)
  {
    printk("RGB LED update failed: %d\n", result);
  }

  color_index = (color_index + 1U) % kTestColors.size();
  k_work_schedule(&color_work, K_SECONDS(1));
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
  k_work_schedule(&color_work, K_NO_WAIT);

  return 0;
}
