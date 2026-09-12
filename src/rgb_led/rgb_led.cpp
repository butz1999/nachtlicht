#include "rgb_led/rgb_led.hpp"

#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/led_strip.h>

#include <array>
#include <cerrno>

namespace rgb_led
{

namespace
{

const struct device *const kLedStrip = DEVICE_DT_GET(DT_ALIAS(status_led));

}  // namespace

int RgbLed::initialize()
{
  return device_is_ready(kLedStrip) ? 0 : -ENODEV;
}

int RgbLed::set(const Color &color)
{
  std::array<led_rgb, 1> pixels{};
  pixels[0].r = color.red;
  pixels[0].g = color.green;
  pixels[0].b = color.blue;

  return led_strip_update_rgb(kLedStrip, pixels.data(), pixels.size());
}

}  // namespace rgb_led
