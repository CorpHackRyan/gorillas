#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

log() {
  printf '%s\n' "$*"
}

need_cmd() {
  command -v "$1" >/dev/null 2>&1
}

install_with_apt() {
  log "Detected apt (Debian/Ubuntu). Installing dependencies..."
  sudo apt update
  sudo apt install -y build-essential cmake libsdl2-dev libsdl2-ttf-dev ffmpeg
}

install_with_dnf() {
  log "Detected dnf (Fedora). Installing dependencies..."
  sudo dnf install -y gcc gcc-c++ make cmake SDL2-devel SDL2_ttf-devel ffmpeg
}

install_with_pacman() {
  log "Detected pacman (Arch). Installing dependencies..."
  sudo pacman -Sy --needed base-devel cmake sdl2 sdl2_ttf ffmpeg
}

install_with_brew() {
  log "Detected Homebrew (macOS). Installing dependencies..."
  brew install cmake sdl2 sdl2_ttf ffmpeg
}

main() {
  local os
  os="$(uname -s)"

  if [[ "$os" == "Darwin" ]]; then
    if need_cmd brew; then
      install_with_brew
      log ""
      log "Done. Next step:"
      log "  ./build.sh"
      exit 0
    fi
    log "Error: Homebrew is not installed."
    log "Install Homebrew first: https://brew.sh"
    exit 1
  fi

  if need_cmd apt; then
    install_with_apt
  elif need_cmd dnf; then
    install_with_dnf
  elif need_cmd pacman; then
    install_with_pacman
  else
    log "Error: unsupported package manager."
    log "Please install dependencies manually: cmake, SDL2, SDL2_ttf, ffmpeg, and a C compiler."
    exit 1
  fi

  log ""
  log "Done. Next step:"
  log "  ./build.sh"
}

main "$@"
