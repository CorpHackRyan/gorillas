#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

if ! command -v cmake >/dev/null 2>&1; then
  echo "Error: cmake is not installed." >&2
  exit 1
fi

if ! command -v gcc >/dev/null 2>&1 && ! command -v clang >/dev/null 2>&1; then
  echo "Error: no C compiler found (gcc/clang)." >&2
  exit 1
fi

JOBS=1
if command -v nproc >/dev/null 2>&1; then
  JOBS="$(nproc)"
elif [[ "$(uname -s)" == "Darwin" ]] && command -v sysctl >/dev/null 2>&1; then
  JOBS="$(sysctl -n hw.logicalcpu 2>/dev/null || echo 1)"
fi

cmake -S . -B build
cmake --build build -j"$JOBS"
exec ./build/gorillas
