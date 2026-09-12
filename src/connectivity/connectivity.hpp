#pragma once

namespace connectivity
{

class Connectivity
{
 public:
  using Ipv4ReadyCallback = void (*)(void *context);

  int initialize(Ipv4ReadyCallback ipv4_ready_callback, void *context);
};

}  // namespace connectivity
