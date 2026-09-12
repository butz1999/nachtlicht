#!/usr/bin/env bash

set -euo pipefail

source "$(dirname "$0")/lib/zephyr.sh"

module_dir="${project_root}/modules/lib/zenoh-pico"
expected_revision="e1ab223a28aaebb5dec1e70d98eab152332f777a"
patch_dir="${project_root}/patches/zenoh-pico"

if ! git -C "${module_dir}" rev-parse --is-inside-work-tree >/dev/null 2>&1; then
  echo "zenoh-pico module is missing: ${module_dir}" >&2
  exit 1
fi

actual_revision="$(git -C "${module_dir}" rev-parse HEAD)"
if [[ "${actual_revision}" != "${expected_revision}" ]]; then
  echo "Unexpected zenoh-pico revision: ${actual_revision}" >&2
  echo "Expected the pinned 1.10.1 revision: ${expected_revision}" >&2
  exit 1
fi

apply_patch() {
  local patch_file="$1"

  if git -C "${module_dir}" apply --reverse --check "${patch_file}" 2>/dev/null; then
    return
  fi

  git -C "${module_dir}" apply --check "${patch_file}"
  git -C "${module_dir}" apply "${patch_file}"
}

apply_patch "${patch_dir}/0001-zephyr-version-header.patch"
apply_patch "${patch_dir}/0002-zephyr-generate-config.patch"
