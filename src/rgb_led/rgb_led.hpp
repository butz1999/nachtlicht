#pragma once

#include <cstdint>

namespace rgb_led
{

struct Color
{
  std::uint8_t red;
  std::uint8_t green;
  std::uint8_t blue;
};

class RgbLed
{
 public:
  int initialize();
  int set(const Color &color);
};

}  // namespace rgb_led
