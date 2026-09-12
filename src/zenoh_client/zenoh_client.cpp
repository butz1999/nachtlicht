#include "zenoh_client/zenoh_client.hpp"

#include <zephyr/autoconf.h>
#include <zephyr/sys/printk.h>

#include <cstring>

namespace zenoh_client
{

ZenohClient::ZenohClient()
{
  k_work_init(&open_work_, open_session);
  z_internal_null(&session_);
}

void ZenohClient::start()
{
  if (std::strlen(CONFIG_NACHTLICHT_ZENOH_ROUTER) == 0U)
  {
    printk("Zenoh router locator is missing; see zenoh.conf.example.\n");
    return;
  }

  if (start_requested_.exchange(true))
  {
    return;
  }

  printk("Opening Zenoh session to %s.\n", CONFIG_NACHTLICHT_ZENOH_ROUTER);
  k_work_submit(&open_work_);
}

void ZenohClient::open_session(struct k_work *work)
{
  auto *const client = CONTAINER_OF(work, ZenohClient, open_work_);

  z_owned_config_t config{};
  auto result = z_config_default(&config);
  if (result == 0)
  {
    result = zp_config_insert(z_loan_mut(config), Z_CONFIG_CONNECT_KEY, CONFIG_NACHTLICHT_ZENOH_ROUTER);
  }

  if (result == 0)
  {
    result = z_open(&client->session_, z_move(config), nullptr);
  }
  else
  {
    z_drop(z_move(config));
  }

  if (result == 0)
  {
    printk("Zenoh session opened.\n");
    return;
  }

  printk("Zenoh session failed: %d\n", static_cast<int>(result));
}

}  // namespace zenoh_client
