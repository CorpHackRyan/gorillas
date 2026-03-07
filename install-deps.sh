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
  local -a required_packages=(
    build-essential
    cmake
    python3
    libsdl2-dev
    libsdl2-ttf-dev
  )

  local optional_package="ffmpeg"
  local -a apt_arch_opt=()
  local host_arch
  host_arch="$(dpkg --print-architecture 2>/dev/null || true)"
  if [[ "$host_arch" == "amd64" ]]; then
    # Some old Pop!/Ubuntu systems have stale i386 metadata that breaks resolver.
    # Force installs to native arch for build dependencies.
    apt_arch_opt=(-o APT::Architectures::=amd64)
  fi

  if sudo apt-get update; then
    sudo DEBIAN_FRONTEND=noninteractive apt-get "${apt_arch_opt[@]}" install -y --no-install-recommends "${required_packages[@]}"
    if ! sudo DEBIAN_FRONTEND=noninteractive apt-get "${apt_arch_opt[@]}" install -y --no-install-recommends "$optional_package"; then
      log "Warning: optional package '$optional_package' could not be installed. Build can continue without it."
    fi
    return 0
  fi

  log "System apt sources are failing. Retrying with isolated Ubuntu mirrors..."

  local codename=""
  local distro_id=""
  if [[ -r /etc/os-release ]]; then
    # shellcheck disable=SC1091
    source /etc/os-release
    codename="${UBUNTU_CODENAME:-${VERSION_CODENAME:-}}"
    distro_id="${ID:-}"
  fi
  if [[ -z "$codename" ]] && need_cmd lsb_release; then
    codename="$(lsb_release -sc 2>/dev/null || true)"
  fi

  if [[ -z "$codename" ]]; then
    log "Error: could not detect distro codename for apt fallback."
    log "Please fix apt sources manually, then rerun this script."
    exit 1
  fi

  if [[ "$distro_id" != "ubuntu" && "$distro_id" != "pop" ]]; then
    log "Error: automatic apt fallback currently supports Ubuntu/Pop!_OS derivatives."
    log "Please fix apt sources manually, then rerun this script."
    exit 1
  fi

  local tmp_sources
  tmp_sources="$(mktemp)"
  trap 'rm -f "$tmp_sources"' RETURN

  for mirror in "http://archive.ubuntu.com/ubuntu" "http://old-releases.ubuntu.com/ubuntu"; do
    local security_mirror="http://security.ubuntu.com/ubuntu"
    if [[ "$mirror" == "http://old-releases.ubuntu.com/ubuntu" ]]; then
      security_mirror="$mirror"
    fi

    cat >"$tmp_sources" <<EOF
deb $mirror $codename main restricted universe multiverse
deb $mirror $codename-updates main restricted universe multiverse
deb $mirror $codename-backports main restricted universe multiverse
deb $security_mirror $codename-security main restricted universe multiverse
EOF

    if sudo apt-get \
      -o Dir::Etc::sourcelist="$tmp_sources" \
      -o Dir::Etc::sourceparts="-" \
      -o APT::Get::List-Cleanup="0" \
      update; then
      sudo DEBIAN_FRONTEND=noninteractive apt-get \
        "${apt_arch_opt[@]}" \
        -o Dir::Etc::sourcelist="$tmp_sources" \
        -o Dir::Etc::sourceparts="-" \
        -o APT::Get::List-Cleanup="0" \
        install -y --no-install-recommends "${required_packages[@]}"

      if ! sudo DEBIAN_FRONTEND=noninteractive apt-get \
        "${apt_arch_opt[@]}" \
        -o Dir::Etc::sourcelist="$tmp_sources" \
        -o Dir::Etc::sourceparts="-" \
        -o APT::Get::List-Cleanup="0" \
        install -y --no-install-recommends "$optional_package"; then
        log "Warning: optional package '$optional_package' could not be installed. Build can continue without it."
      fi

      return 0
    fi
  done

  log "Error: failed to install dependencies via apt fallback mirrors."
  log "Please repair apt repositories on this machine and rerun this script."
  exit 1
}

install_with_dnf() {
  log "Detected dnf (Fedora). Installing dependencies..."
  sudo dnf install -y gcc gcc-c++ make cmake python3 SDL2-devel SDL2_ttf-devel
  if ! sudo dnf install -y ffmpeg; then
    log "Warning: optional package 'ffmpeg' could not be installed. Build can continue without it."
  fi
}

install_with_pacman() {
  log "Detected pacman (Arch). Installing dependencies..."
  sudo pacman -Sy --needed base-devel cmake python sdl2 sdl2_ttf
  if ! sudo pacman -Sy --needed ffmpeg; then
    log "Warning: optional package 'ffmpeg' could not be installed. Build can continue without it."
  fi
}

install_with_brew() {
  log "Detected Homebrew (macOS). Installing dependencies..."
  brew install cmake python sdl2 sdl2_ttf
  if ! brew install ffmpeg; then
    log "Warning: optional package 'ffmpeg' could not be installed. Build can continue without it."
  fi
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

  if need_cmd apt-get; then
    install_with_apt
  elif need_cmd dnf; then
    install_with_dnf
  elif need_cmd pacman; then
    install_with_pacman
  else
    log "Error: unsupported package manager."
    log "Please install dependencies manually: cmake, SDL2, SDL2_ttf, and a C compiler."
    exit 1
  fi

  log ""
  log "Done. Next step:"
  log "  ./build.sh"
}

main "$@"
