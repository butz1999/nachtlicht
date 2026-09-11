#!/usr/bin/env bash

set -euo pipefail

source "$(dirname "$0")/lib/zephyr.sh"

require_zephyr_environment
require_file "${build_dir}/zephyr/zephyr.elf" "firmware ELF"

export ZEPHYR_BASE="${zephyr_base}"

cd "${build_dir}"
exec "${python_bin}" \
  "${zephyr_workspace}/modules/hal/espressif/tools/idf_monitor/idf_monitor.py" \
  -p "${esp_device}" \
  -b 115200 \
  zephyr/zephyr.elf \
  -d
