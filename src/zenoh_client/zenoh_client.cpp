#include "zenoh_client/zenoh_client.hpp"

#include <zephyr/autoconf.h>
#include <zephyr/sys/printk.h>

#include <cstring>

namespace zenoh_client
{

namespace
{

constexpr char kColorNextKeyexpr[] = "nachtlicht/led/color/next";

}  // namespace

ZenohClient::ZenohClient()
{
  k_work_init(&open_work_, open_session);
  z_internal_null(&session_);
  z_internal_null(&color_next_subscriber_);
}

void ZenohClient::initialize(ColorNextCallback color_next_callback, void *context)
{
  color_next_callback_ = color_next_callback;
  color_next_context_ = context;
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

    z_owned_closure_sample_t callback{};
    result = z_closure_sample(&callback, handle_color_next, nullptr, client);
    if (result == 0)
    {
      z_view_keyexpr_t keyexpr;
      z_view_keyexpr_from_str_unchecked(&keyexpr, kColorNextKeyexpr);
      result = z_declare_subscriber(z_loan(client->session_), &client->color_next_subscriber_, z_loan(keyexpr),
                                    z_move(callback), nullptr);
    }

    if (result == 0)
    {
      printk("Zenoh subscribed to %s.\n", kColorNextKeyexpr);
    }
    else
    {
      printk("Zenoh subscriber declaration failed: %d\n", static_cast<int>(result));
    }
    return;
  }

  printk("Zenoh session failed: %d\n", static_cast<int>(result));
}

void ZenohClient::handle_color_next(z_loaned_sample_t *sample, void *context)
{
  ARG_UNUSED(sample);

  auto *const client = static_cast<ZenohClient *>(context);
  if (client->color_next_callback_ == nullptr)
  {
    printk("Zenoh colour-change request ignored: callback is missing.\n");
    return;
  }

  printk("Zenoh colour-change request received.\n");
  client->color_next_callback_(client->color_next_context_);
}

}  // namespace zenoh_client
