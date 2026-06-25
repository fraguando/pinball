# 3D Pinball Designer

A 3D pinball game with an integrated level designer, written in C++ with a clean,
layered architecture (SOLID). Design a board by placing elements with the mouse,
then press **Play** to launch the ball with real 3D physics.

## Elements

- **Flippers** – rotating bats controlled by the **Left / Right arrow keys**.
- **Bumpers** – round posts that energetically bounce the ball away and score.
- **Walls** – static blocks the ball reflects off.
- **Hole** – a cup that captures the ball and holds it for **2 seconds**, then ejects it.
- **Rail / Slider** – a 3D polyline the ball latches onto and rides to the exit
  (a rising rail launches the ball into the air).
- **Spawn** – where the ball starts / respawns.

## Controls

**Editor**
- `1`–`7` select a tool (Select, Wall, Bumper, Flipper, Hole, Rail, Spawn), or click the toolbar.
- **Left click** places the current element (Rail: click each node, **Enter** to finish).
- **Select** tool: click an element to select, drag to move, **Del** to remove.
- **Right-drag** orbits the camera, **mouse wheel** zooms.
- **Ctrl+S** / **Ctrl+O** save / load the board (`assets/levels/board.json`).
- **Tab** (or `P`) enters Play mode.

**Play**
- **Left / Right arrows** flip the flippers.
- **Space** launches the ball (plunger).
- **Right-drag / wheel** orbit / zoom. **Esc** returns to the editor.

## Architecture

Dependencies point inward; the domain knows nothing of SDL/OpenGL/JSON.

| Layer | Contents |
|-------|----------|
| `domain` | Ball, elements, `Board`, 3D collision math (GLM only) |
| `application` | ports (`IRenderer`, `IInputProvider`, `ILevelRepository`, `IClock`), `PinballSimulation`, `LevelEditor`, `AppController`, `Camera` |
| `infrastructure` | OpenGL renderer, SDL input/clock, JSON repository |
| `main.cpp` | composition root (wires concrete adapters into the ports) |

Elements expose themselves as data (collision shapes + render items) and opt into
special behaviour via capability interfaces (`IControllable`, `ICaptureZone`,
`IGuidePath`). Adding a new element type means one new class plus one line in
`ElementFactory` — no changes to the simulation, renderer or editor.

## Building (Windows)

Requires CMake ≥ 3.20, Ninja, and an MSVC toolchain. SDL2, GLM and nlohmann/json
are fetched automatically via CMake `FetchContent` (needs network on first configure).

```sh
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build build
./build/pinball.exe
```

(Run inside a Developer/`vcvars64` environment so the MSVC compiler is on `PATH`.)

Run the headless logic tests with:

```sh
./build/pinball_selftest.exe
```
