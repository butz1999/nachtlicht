#!/usr/bin/env bash

set -euo pipefail

dilbert_dir="${HOME}/git/nachtlicht/dilbert"

if [[ ! -d "${dilbert_dir}" ]]; then
  echo "Dilbert was not found at: ${dilbert_dir}" >&2
  exit 1
fi

dilbert_venv="${dilbert_dir}/.venv/bin/activate"

if [[ ! -f "${dilbert_venv}" ]]; then
  echo ".venv was not found at: ${dilbert_venv}" >&2
  exit 1
fi

cd "${dilbert_dir}"
source "${dilbert_venv}"
jupyter lab --no-browser