#!/usr/bin/env bash

set -euo pipefail

source "$(dirname "$0")/lib/zephyr.sh"

require_zephyr_environment
require_file "${build_dir}/zephyr/zephyr.bin" "firmware image"

cd "${zephyr_workspace}"
exec "${west_bin}" flash \
  -d "${build_dir}" \
  --esp-device "${esp_device}" \
  --esp-no-progress
