#!/usr/bin/env bash

set -euo pipefail

zenoh_router="/mnt/c/Program Files/zenoh/zenohd.exe"

if [[ ! -x "${zenoh_router}" ]]; then
  echo "zenohd.exe was not found at: ${zenoh_router}" >&2
  exit 1
fi

exec "${zenoh_router}" -l tcp/0.0.0.0:7447
