#!/usr/bin/env bash

set -euo pipefail

project_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
build_dir="${project_root}/build/hello-world"
zephyr_workspace="${ZEPHYR_WORKSPACE:-${HOME}/git/zephyrproject}"
zephyr_base="${zephyr_workspace}/zephyr"
west_bin="${WEST_BIN:-${HOME}/.venvs/zephyr/bin/west}"
python_bin="${PYTHON_BIN:-${HOME}/.venvs/zephyr/bin/python}"
esp_device="${ESP_DEVICE:-/dev/ttyACM0}"

export PATH="$(dirname "${python_bin}"):${PATH}"

require_file()
{
  local path="$1"
  local description="$2"

  if [[ ! -f "${path}" ]]; then
    echo "Missing ${description}: ${path}" >&2
    exit 1
  fi
}

require_executable()
{
  local path="$1"
  local description="$2"

  if [[ ! -x "${path}" ]]; then
    echo "Missing executable ${description}: ${path}" >&2
    exit 1
  fi
}

require_zephyr_environment()
{
  require_executable "${west_bin}" "west"
  require_executable "${python_bin}" "Python"
  require_file "${zephyr_base}/west.yml" "Zephyr manifest"
}
