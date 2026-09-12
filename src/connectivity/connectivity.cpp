#include "connectivity/connectivity.hpp"

#include <zephyr/kernel.h>
#include <zephyr/net/dhcpv4.h>
#include <zephyr/net/net_if.h>
#include <zephyr/net/net_ip.h>
#include <zephyr/net/net_mgmt.h>
#include <zephyr/net/wifi_mgmt.h>
#include <zephyr/sys/printk.h>

#include <cerrno>
#include <cstdint>
#include <cstring>

namespace connectivity
{

namespace
{

constexpr std::uint64_t kWifiEvents = NET_EVENT_WIFI_CONNECT_RESULT | NET_EVENT_WIFI_DISCONNECT_RESULT;
constexpr std::uint64_t kIpv4Events = NET_EVENT_IPV4_ADDR_ADD;

struct net_mgmt_event_callback wifi_callback;
struct net_mgmt_event_callback ipv4_callback;
struct wifi_connect_req_params connection_parameters;

bool report_connect_result(const struct net_mgmt_event_callback *callback)
{
  const auto *status = static_cast<const struct wifi_status *>(callback->info);
  if (status == nullptr)
  {
    printk("Wi-Fi connection result did not include a status.\n");
    return false;
  }

  if (status->status == 0)
  {
    printk("Wi-Fi associated with the access point.\n");
    return true;
  }

  printk("Wi-Fi connection failed: %d\n", status->status);
  return false;
}

void report_ipv4_address(const struct net_mgmt_event_callback *callback)
{
  const auto *address = static_cast<const struct net_in_addr *>(callback->info);
  if (address == nullptr)
  {
    printk("IPv4 address event did not include an address.\n");
    return;
  }

  char address_text[NET_IPV4_ADDR_LEN];
  if (net_addr_ntop(NET_AF_INET, address, address_text, sizeof(address_text)) != nullptr)
  {
    printk("DHCP assigned IPv4 address: %s\n", address_text);
  }
}

void wifi_event_handler(struct net_mgmt_event_callback *callback, std::uint64_t event, struct net_if *interface)
{
  switch (event)
  {
    case NET_EVENT_WIFI_CONNECT_RESULT:
      if (report_connect_result(callback))
      {
        printk("Requesting an IPv4 address via DHCP.\n");
        net_dhcpv4_start(interface);
      }
      break;
    case NET_EVENT_WIFI_DISCONNECT_RESULT:
      printk("Wi-Fi disconnected.\n");
      break;
    default:
      break;
  }
}

void ipv4_event_handler(struct net_mgmt_event_callback *callback, std::uint64_t event, struct net_if *interface)
{
  ARG_UNUSED(interface);

  if (event == NET_EVENT_IPV4_ADDR_ADD)
  {
    report_ipv4_address(callback);
  }
}

}  // namespace

int Connectivity::initialize()
{
  const auto ssid_length = std::strlen(CONFIG_NACHTLICHT_WIFI_SSID);
  const auto psk_length = std::strlen(CONFIG_NACHTLICHT_WIFI_PSK);

  if (ssid_length == 0U || ssid_length > WIFI_SSID_MAX_LEN || psk_length < 8U || psk_length > WIFI_PSK_MAX_LEN)
  {
    printk("Wi-Fi credentials are missing or invalid; see wifi.conf.example.\n");
    return -EINVAL;
  }

  auto *const interface = net_if_get_default();
  if (interface == nullptr)
  {
    printk("No default network interface is available.\n");
    return -ENODEV;
  }

  net_mgmt_init_event_callback(&wifi_callback, wifi_event_handler, kWifiEvents);
  net_mgmt_add_event_callback(&wifi_callback);
  net_mgmt_init_event_callback(&ipv4_callback, ipv4_event_handler, kIpv4Events);
  net_mgmt_add_event_callback(&ipv4_callback);

  connection_parameters = {};
  connection_parameters.ssid = reinterpret_cast<const std::uint8_t *>(CONFIG_NACHTLICHT_WIFI_SSID);
  connection_parameters.ssid_length = static_cast<std::uint8_t>(ssid_length);
  connection_parameters.psk = reinterpret_cast<const std::uint8_t *>(CONFIG_NACHTLICHT_WIFI_PSK);
  connection_parameters.psk_length = static_cast<std::uint8_t>(psk_length);
  connection_parameters.band = WIFI_FREQ_BAND_UNKNOWN;
  connection_parameters.channel = WIFI_CHANNEL_ANY;
  connection_parameters.security = WIFI_SECURITY_TYPE_PSK;
  connection_parameters.mfp = WIFI_MFP_OPTIONAL;

  const auto result =
      net_mgmt(NET_REQUEST_WIFI_CONNECT, interface, &connection_parameters, sizeof(connection_parameters));
  if (result != 0)
  {
    printk("Wi-Fi connection request failed: %d\n", result);
    return result;
  }

  printk("Wi-Fi connection requested.\n");
  return 0;
}

}  // namespace connectivity
