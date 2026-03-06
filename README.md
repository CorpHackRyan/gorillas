# Gorillas in C

Modern C rewrite of classic Gorillas, currently using SDL2/SDL2_ttf.

## Demo Video

[![Watch on YouTube](https://img.youtube.com/vi/nvx3Epcc4tg/maxresdefault.jpg)](https://www.youtube.com/watch?v=nvx3Epcc4tg)

Direct link: https://www.youtube.com/watch?v=nvx3Epcc4tg

## Build and Run

From the repo root:

```bash
./install-deps.sh
./build.sh
```

`install-deps.sh` installs system dependencies for macOS/Homebrew, Debian/Ubuntu, Fedora, or Arch.
`build.sh` configures, builds, and runs the game in one step.

Equivalent manual commands:

```bash
cmake -S . -B build
cmake --build build -j
./build/gorillas
```

What each command does:

- `cmake -S . -B build`
  - Configures the project using `CMakeLists.txt` in the current directory (`-S .`).
  - Generates build files into the `build/` directory (`-B build`) so source files stay clean.
- `cmake --build build -j`
  - Compiles the project using the generated build files in `build/`.
  - `-j` enables parallel compilation for faster builds.
- `./build/gorillas`
  - Runs the compiled game executable.

If dependencies are missing on Debian/Ubuntu:

```bash
sudo apt update
sudo apt install -y build-essential cmake python3 libsdl2-dev libsdl2-ttf-dev ffmpeg
```

If dependencies are missing on macOS (Homebrew):

```bash
brew install cmake python sdl2 sdl2_ttf ffmpeg
```

## Notes

- The game tries both `assets/fonts/...` and `../assets/fonts/...`, so you can run it from repo root (`./build/gorillas`) or from inside `build/` (`./gorillas`).
- In headless/sandbox environments (no real display), SDL may fail to open a graphics device.
