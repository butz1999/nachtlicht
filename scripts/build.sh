#!/usr/bin/env bash

set -euo pipefail

source "$(dirname "$0")/lib/zephyr.sh"

require_zephyr_environment

cd "${zephyr_workspace}"
exec "${west_bin}" build -p "${WEST_PRISTINE:-auto}" \
  -d "${build_dir}" \
  -b esp32s3_devkitc/esp32s3/procpu \
  -S espressif-flash-4M \
  "${project_root}"
