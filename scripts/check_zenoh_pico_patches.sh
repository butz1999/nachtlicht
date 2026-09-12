#!/usr/bin/env bash

set -euo pipefail

source "$(dirname "$0")/lib/zephyr.sh"

module_dir="${project_root}/modules/lib/zenoh-pico"
patch_dir="${project_root}/patches/zenoh-pico"

if ! git -C "${module_dir}" rev-parse --is-inside-work-tree >/dev/null 2>&1; then
  echo "zenoh-pico module is missing: ${module_dir}" >&2
  exit 1
fi

if ! git -C "${module_dir}" diff --quiet; then
  echo "zenoh-pico has local changes. Restore the applied patches before checking a new release." >&2
  exit 1
fi

for patch_file in "${patch_dir}"/*.patch; do
  patch_name="$(basename "${patch_file}")"
  if git -C "${module_dir}" apply --reverse --check "${patch_file}" 2>/dev/null; then
    echo "${patch_name}: included upstream"
  elif git -C "${module_dir}" apply --check "${patch_file}"; then
    echo "${patch_name}: still required"
  else
    echo "${patch_name}: no longer applies cleanly; review required"
  fi
done
