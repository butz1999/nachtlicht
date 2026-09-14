#!/usr/bin/env bash

set -euo pipefail

source "$(dirname "$0")/lib/zephyr.sh"

require_zephyr_environment
"${project_root}/scripts/prepare_zenoh_pico.sh"

extra_conf_file=""
if [[ -f "${project_root}/wifi.conf" ]]; then
  extra_conf_file="${project_root}/wifi.conf"
fi

cd "${zephyr_workspace}"
exec "${west_bin}" build -p "${WEST_PRISTINE:-auto}" \
  -d "${build_dir}" \
  -b esp32s3_devkitc/esp32s3/procpu \
  -S espressif-flash-4M \
  "${project_root}" \
  -- "-DEXTRA_CONF_FILE=${extra_conf_file}" \
     "-DZEPHYR_EXTRA_MODULES=${project_root}/modules/lib/zenoh-pico"
