# Cascade

Real-time SPH (Smoothed Particle Hydrodynamics) fluid simulator. Each particle
is a blob of fluid — density, pressure, and viscosity forces are computed from
nearby neighbors every frame, making the fluid flow, splash, and settle
naturally.

## Features
- Real-time SPH fluid simulation with up to 8,000 particles
- 3 render modes: solid color, velocity heatmap, density
- Interactive controls: spawn with mouse, pause, reset, draw walls
- Spatial hash grid for O(n) neighbor lookup
- Cross-platform: macOS, Linux, Windows (via MSYS2)

## Build & Run

### Prerequisites
- raylib 6.0 (`brew install raylib` / `sudo apt install libraylib-dev` / MSYS2: `pacman -S mingw-w64-x86_64-raylib`)
- gcc (C99)

### Compile
```
make clean && make
./cascade
```

## Controls

| Key | Action |
|-----|--------|
| Left click | Spawn particles |
| Space | Pause/Resume |
| R | Reset |
| D | Toggle draw mode |

## Project Structure

```
src/
├── cascade.h          # shared types (Particle, SimState, UIState, Obstacle)
├── main.c             # entry point + game loop
├── input.[ch]         # mouse spawn, keyboard shortcuts
├── spatial_hash.[ch]  # fast neighbor lookup (O(n) vs O(n²))
├── sph.[ch]           # SPH fluid physics (density, pressure, viscosity)
├── renderer.[ch]      # 3 render modes + HUD
├── ui.[ch]            # sidebar controls (raygui)
└── obstacles.[ch]     # ramps, boxes, walls
```


