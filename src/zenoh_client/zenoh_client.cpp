#include "zenoh_client/zenoh_client.hpp"

#include <zephyr/autoconf.h>
#include <zephyr/sys/printk.h>

#include <cstddef>
#include <cstdio>

namespace zenoh_client
{

namespace
{

constexpr char kColorNextKeyexpr[] = "nachtlicht/led/color/next";
constexpr char kColorChangedKeyexpr[] = "nachtlicht/led/color/changed";
constexpr char kBrightnessChangedKeyexpr[] = "nachtlicht/led/brightness/changed";

}  // namespace

ZenohClient::ZenohClient()
{
  k_work_init(&open_work_, open_session);
  z_internal_null(&session_);
  z_internal_null(&color_next_subscriber_);
  z_internal_null(&color_changed_publisher_);
  z_internal_null(&brightness_changed_publisher_);
}

void ZenohClient::initialize(ColorNextCallback color_next_callback, void *context)
{
  color_next_callback_ = color_next_callback;
  color_next_context_ = context;
}

void ZenohClient::start()
{
  if (start_requested_.exchange(true))
  {
    return;
  }

  printk("Scouting for a Zenoh router on UDP multicast.\n");
  k_work_submit(&open_work_);
}

void ZenohClient::open_session(struct k_work *work)
{
  auto *const client = CONTAINER_OF(work, ZenohClient, open_work_);

  z_owned_config_t config{};
  auto result = z_config_default(&config);

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

    z_owned_closure_sample_t color_next_callback{};
    result = z_closure_sample(&color_next_callback, handle_color_next, nullptr, client);
    if (result == 0)
    {
      z_view_keyexpr_t keyexpr;
      z_view_keyexpr_from_str_unchecked(&keyexpr, kColorNextKeyexpr);
      result = z_declare_subscriber(z_loan(client->session_), &client->color_next_subscriber_, z_loan(keyexpr),
                                    z_move(color_next_callback), nullptr);
    }

    if (result == 0)
    {
      printk("Zenoh subscribed to %s.\n", kColorNextKeyexpr);

      z_publisher_options_t publisher_options{};
      z_publisher_options_default(&publisher_options);

      z_view_keyexpr_t color_changed_keyexpr;
      z_view_keyexpr_from_str_unchecked(&color_changed_keyexpr, kColorChangedKeyexpr);
      result = z_declare_publisher(z_loan(client->session_), &client->color_changed_publisher_,
                                   z_loan(color_changed_keyexpr), &publisher_options);
    }
    if (result == 0)
    {
      z_publisher_options_t publisher_options{};
      z_publisher_options_default(&publisher_options);

      z_view_keyexpr_t brightness_changed_keyexpr;
      z_view_keyexpr_from_str_unchecked(&brightness_changed_keyexpr, kBrightnessChangedKeyexpr);
      result = z_declare_publisher(z_loan(client->session_), &client->brightness_changed_publisher_,
                                   z_loan(brightness_changed_keyexpr), &publisher_options);
    }
    if (result == 0)
    {
      client->events_ready_.store(true);
      printk("Zenoh publishes %s and %s.\n", kColorChangedKeyexpr, kBrightnessChangedKeyexpr);
    }
    else
    {
      printk("Zenoh endpoint declaration failed: %d\n", static_cast<int>(result));
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

void ZenohClient::publish_color_changed(std::uint32_t sequence, const char *color)
{
  char payload[48];
  const auto length = std::snprintf(payload, sizeof(payload), "{\"sequence\":%u,\"color\":\"%s\"}",
                                    static_cast<unsigned int>(sequence), color);
  if (length < 0 || static_cast<std::size_t>(length) >= sizeof(payload))
  {
    printk("Zenoh colour-change event payload could not be formatted.\n");
    return;
  }

  publish_event(&color_changed_publisher_, payload);
}

void ZenohClient::publish_brightness_changed(std::uint32_t sequence, std::uint8_t brightness)
{
  char payload[48];
  const auto length = std::snprintf(payload, sizeof(payload), "{\"sequence\":%u,\"brightness\":%u}",
                                    static_cast<unsigned int>(sequence), static_cast<unsigned int>(brightness));
  if (length < 0 || static_cast<std::size_t>(length) >= sizeof(payload))
  {
    printk("Zenoh brightness event payload could not be formatted.\n");
    return;
  }

  publish_event(&brightness_changed_publisher_, payload);
}

void ZenohClient::publish_event(z_owned_publisher_t *publisher, const char *payload)
{
  if (!events_ready_.load())
  {
    return;
  }

  z_owned_bytes_t bytes{};
  auto result = z_bytes_copy_from_str(&bytes, payload);
  if (result == 0)
  {
    result = z_publisher_put(z_loan(*publisher), z_move(bytes), nullptr);
  }

  if (result != 0)
  {
    printk("Zenoh event publication failed: %d\n", static_cast<int>(result));
  }
}

}  // namespace zenoh_client
