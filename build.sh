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

ensure_sdl_deps() {
  if ! command -v pkg-config >/dev/null 2>&1; then
    return 0
  fi

  if pkg-config --exists sdl2 SDL2_ttf; then
    return 0
  fi

  echo "Missing SDL2 development packages (sdl2, SDL2_ttf)." >&2
  if [[ -x "$SCRIPT_DIR/install-deps.sh" ]]; then
    echo "Attempting to install dependencies via ./install-deps.sh ..." >&2
    "$SCRIPT_DIR/install-deps.sh"
    if pkg-config --exists sdl2 SDL2_ttf; then
      return 0
    fi
  fi

  echo "Error: SDL2 dependencies are still missing." >&2
  echo "Run ./install-deps.sh and ensure libsdl2-dev and libsdl2-ttf-dev are installed." >&2
  exit 1
}

ensure_sdl_deps

JOBS=1
if command -v nproc >/dev/null 2>&1; then
  JOBS="$(nproc)"
elif [[ "$(uname -s)" == "Darwin" ]] && command -v sysctl >/dev/null 2>&1; then
  JOBS="$(sysctl -n hw.logicalcpu 2>/dev/null || echo 1)"
fi

cmake -S . -B build
cmake --build build -j"$JOBS"
exec ./build/gorillas
