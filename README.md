# Particle Physics Simulation

## Overview

This project is an interactive C++ application that simulates physical and chemical behaviors of particles and elements. Using OpenGL and ImGui for rendering, the simulation supports real-time visualization of particle dynamics such as movement, collisions, merging, decay, and chemical reactions.

The system supports two major types of entities:
- **Chemical Elements** (e.g., H, O, Fe)
- **Fundamental Particles** (e.g., Electron, Quark, Photon)

## Features

- **Real-Time Particle Simulation**:
  - Motion governed by temperature and friction
  - Edge bouncing and velocity decay
  - Trail effects that fade over time

- **Chemical Reaction Engine**:
  - Hardcoded reaction table for hundreds of element combinations
  - Merging behavior based on reaction rules
  - Reaction name replaces original particle names upon merge

- **Radioactive Decay**:
  - Heavy elements decay over time using background threads
  - Decayed particles can transform into fundamental particles

- **Collision Physics**:
  - Resolves overlapping particles using elastic collision approximation
  - Supports mass-based velocity updates and momentum conservation

- **Interactive UI (via ImGui)**:
  - Temperature control
  - Air resistance slider
  - Particle count and type selection (ELEMENT, PARTICLE, BOTH)

## Dependencies

- [GLFW](https://www.glfw.org/)
- [OpenGL](https://www.opengl.org/)
- [Dear ImGui](https://github.com/ocornut/imgui)
- C++17 or later
- CMake (for build management)

## Usage

1. **Build** the application with your preferred C++ compiler (see Build section).
2. **Run** the program. It will prompt you to choose a simulation type and particle count.
3. Use the ImGui interface to adjust temperature and air resistance.
4. Observe particle dynamics, reactions, and decay in real-time.

## Build Instructions

### Using CMake (Recommended)

```bash
mkdir build
cd build
cmake ..
make
./ParticleSimulation
