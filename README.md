# SPH 2D Fluid Simulation (C++ & OpenGL)

An interactive 2D Smoothed Particle Hydrodynamics (SPH) fluid simulation built in C++17 and OpenGL. Features custom SPH density/pressure/viscosity kernels, surface tension cohesion, $O(N)$ spatial grid hashing, and OpenMP multithreading acceleration.

---

## Features
- **Physical SPH Solver**: Poly6 density kernel, Spiky pressure gradient kernel, and Viscosity Laplacian kernel.
- **Surface Tension Cohesion**: Realistic droplet formation and fluid stream cohesion.
- **Fast Spatial Hashing**: $90 \times 90$ cell spatial grid for $O(N)$ neighbor searching.
- **OpenMP Multithreading**: Multi-core parallel particle updates running at 120+ FPS.
- **Interactive Mouse Controls**: Attract or repel fluid particles dynamically using the mouse cursor.

---

## Interactive Controls

| Input | Action |
|---|---|
| **Left Mouse Drag** | Attract fluid toward mouse cursor |
| **Right Mouse Drag** | Repel fluid away from mouse cursor |
| **`R` Key** | Reset fluid block to top center |
| **`SPACE` Key** | Pause / Resume physics simulation |
| **`G` Key** | Toggle gravity on / off |

---

## Prerequisites & Installation

### What to Download:
1. **CMake** (v3.16+):
   - Download: [https://cmake.org/download/](https://cmake.org/download/)
   - *Note*: During installation on Windows, select **"Add CMake to system PATH"**.

2. **C++ Compiler Toolchain**:
   - **Windows**: [Visual Studio 2022](https://visualstudio.microsoft.com/vs/community/) (Check *"Desktop development with C++"*).
   - **macOS**: Run `xcode-select --install` in terminal.
   - **Linux (Ubuntu/Debian)**: Run `sudo apt update && sudo apt install build-essential cmake libgl1-mesa-dev libx11-dev libxrandr-dev libxinerama-dev libxcursor-dev libxi-dev`.

---

## Building and Running with CMake

The project automatically fetches and builds GLFW 3.4 via CMake `FetchContent` if not found locally, making it **zero-configuration**.

```bash
# 1. Configure CMake build system
cmake -B build

# 2. Compile the project in Release mode
cmake --build build --config Release

# 3. Run the executable
# On Windows:
.\build\Release\SPH_FluidSimulation.exe

# On Linux / macOS:
./build/SPH_FluidSimulation
```

---

## Codebase Architecture

```
FirstTryOpenGL/
├── CMakeLists.txt           # Cross-platform CMake build configuration
├── Config.hpp               # Global constants, data structures (Particle, Force), & math utilities
├── SphKernels.hpp/.cpp      # Pure SPH 2D smoothing kernels (Poly6, Spiky Gradient, Viscosity Laplacian)
├── SpatialGrid.hpp/.cpp     # Spatial grid hashing for O(N) neighbor searching
├── FluidSimulation.hpp/.cpp # SPH physics solver loops, density/force computation & integration
├── Renderer.hpp/.cpp        # OpenGL rendering procedures (particles, container bounds, mouse cursor)
└── main.cpp                 # Entry point, GLFW window initialization, and input callbacks
```
