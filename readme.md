# Mandelbrot Set Viewer in C

_A Multithreaded Mandelbrot set viewer written in C using SDL3._

<p align="center">
  <img src="assets/demo.jpg" width="45%" />
</p>

## Features

- Mouse controls: Dragging, Zooming
- Multi-threaded rendering
- Progressive rendering
- Dynamic resolution
- Colour rendering
- 12 Colour palettes
- Cross-platform (macOS, Linux, Windows)
- Built and released automatically via GitHub Actions
- Not written by AI ;)

## Controls

| Action                      | Input                     |
| :-------------------------- | :------------------------ |
| **Pan Viewport**            | Click + Drag (Mouse)      |
| **Zoom In/Out**             | Mouse Scroll Wheel        |
| **Increase Max Iterations** | `>`                       |
| **Decrease Max Iterations** | `<`                       |
| **Toggle Shading Mode**     | `/` (Standard vs. Smooth) |
| **Cycle Colour Palettes**   | `M`                       |

## How to Build

### Prerequisites

- **CMake** (Version 3.16+)
- **C Compiler** (GCC, Clang, or MSVC)
- **vcpkg** (for managing dependencies)
- **SDL3** and **pthreads** (installed via vcpkg)

### Install Dependencies

The project uses `vcpkg.json` to manage dependencies. You can either use the manifest or install manually:

#### Using vcpkg manifest (Recommended)

The dependencies are automatically installed using the `vcpkg.json` manifest:

```bash
vcpkg install --triplet <triplet>
```

Where `<triplet>` is:
- **Windows:** `x64-windows-static` (for standalone executable)
- **Linux:** `x64-linux-release`
- **macOS:** `arm64-osx-release` (Apple Silicon) or `x64-osx-release` (Intel Mac)

#### Manual installation (Alternative)

**Windows:**
```bash
vcpkg install sdl3:x64-windows-static pthreads:x64-windows-static
```

**macOS (Apple Silicon):**
```bash
vcpkg install sdl3:arm64-osx-release
```

**macOS (Intel):**
```bash
vcpkg install sdl3:x64-osx-release
```

**Linux:**
```bash
vcpkg install sdl3:x64-linux-release
```

### Build Steps

1.  **Clone the repository:**

    ```bash
    git clone https://github.com/a-lothian/Mandelbrot-Viewer
    cd Mandelbrot-Viewer
    ```

2.  **Install dependencies:**

    ```bash
    vcpkg install --triplet <triplet>
    ```

3.  **Configure the project:**

    ##### Windows:
    ```bash
    cmake -B build -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_TOOLCHAIN_FILE=$env:VCPKG_INSTALLATION_ROOT/scripts/buildsystems/vcpkg.cmake \
      -DVCPKG_TARGET_TRIPLET=x64-windows-static
    ```

    ##### Linux:
    ```bash
    cmake -B build -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_TOOLCHAIN_FILE=$VCPKG_INSTALLATION_ROOT/scripts/buildsystems/vcpkg.cmake \
      -DVCPKG_TARGET_TRIPLET=x64-linux-release
    ```

    ##### macOS (Apple Silicon):
    ```bash
    cmake -B build -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_TOOLCHAIN_FILE=$VCPKG_INSTALLATION_ROOT/scripts/buildsystems/vcpkg.cmake \
      -DVCPKG_TARGET_TRIPLET=arm64-osx-release
    ```

    ##### macOS (Intel):
    ```bash
    cmake -B build -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_TOOLCHAIN_FILE=$VCPKG_INSTALLATION_ROOT/scripts/buildsystems/vcpkg.cmake \
      -DVCPKG_TARGET_TRIPLET=x64-osx-release
    ```

4.  **Compile:**

    ```bash
    cmake --build build --config Release
    ```

5.  **Run:**
    - **Linux:** `./build/Mandelbrot`
    - **macOS:** `./build/Mandelbrot`
    - **Windows:** `.\build\Release\Mandelbrot.exe`

## Image Showcase

<p align="center">
  <img src="assets/demo1.jpg" width="45%" />
  <img src="assets/demo2.jpg" width="45%" />
</p>

<p align="center">
  <img src="assets/demo3.jpg" width="45%" />
  <img src="assets/demo4.jpg" width="45%" />
</p>

<p align="center">
  <img src="assets/demo5.jpg" width="45%" />
  <img src="assets/demo6.jpg" width="45%" />
</p>

## Third-Party Libraries

* **SDL3 (Simple DirectMedia Layer)**: Used for windowing, input, and graphics.
    * License: zlib License
    * Source: https://github.com/libsdl-org/SDL
    * Copyright (C) 1997-2025 Sam Lantinga

* **pthreads4w** _(Windows only)_: Used for multithreading support on Windows.
    * License: Apache License 2.0
    * Source: https://sourceforge.net/projects/pthreads4w/
    * Copyright (C) 1998 Ross Johnson