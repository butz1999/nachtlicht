#!/usr/bin/env bash

set -euo pipefail

dilbert_dir="${HOME}/git/nachtlicht/dilbert"

if [[ ! -d "${dilbert_dir}" ]]; then
  echo "Dilbert was not found at: ${dilbert_dir}" >&2
  exit 1
fi

if ! command -v cmd.exe >/dev/null 2>&1 || ! command -v wslpath >/dev/null 2>&1; then
  echo "This script must run inside WSL with Windows integration enabled." >&2
  exit 1
fi

windows_local_appdata="$(cmd.exe /C echo %LOCALAPPDATA% 2>/dev/null | tr -d '\r')"
windows_venv="$(wslpath -u "${windows_local_appdata}")/Nachtlicht/dilbert-venv"
windows_python="${windows_venv}/Scripts/python.exe"

if [[ ! -f "${windows_python}" ]]; then
  echo "Windows Dilbert environment was not found at: ${windows_venv}" >&2
  echo "Create it first as documented in dilbert/README.md." >&2
  exit 1
fi

windows_notebook_dir="$(wslpath -w "${dilbert_dir}")"
exec "${windows_python}" -m jupyter lab --no-browser --notebook-dir "${windows_notebook_dir}"
