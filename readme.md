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
- Not written by AI

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
- **SDL3 Development Libraries**:
  - **Windows:** install via a package manager like `vcpkg`
  - **macOS:** Install via Homebrew
  - **Linux:** Install via your package manager (e.g., `sudo apt install libsdl3-dev`) or build from source.

### Build Steps

1.  **Clone the repository:**

    ```bash
    git clone [https://github.com/a-lothian/Mandelbrot-Viewer](https://github.com/a-lothian/Mandelbrot-Viewer)
    cd Mandelbrot-Viewer
    ```

2.  **Configure the project:**

    ```bash
    cmake -S . -B build
    ```

    _(Note: If CMake cannot find SDL3, you may need to specify the path: `cmake -S . -B build -DSDL3_DIR=/path/to/sdl3`)_

3.  **Compile:**

    ```bash
    cmake --build build --config Release
    ```

4.  **Run:**
    - **Linux/macOS:** `./build/Mandelbrot`
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
