# hatsu_ino — project instructions

## Repository layout

Each board version lives entirely under `boards/<version>/`:
- `logic.h` — pure logic for that board (tested natively, no hardware)
- `test/` — native test suite for that board
- `<sketch>/` — Arduino sketch
- `README.md` — wiring, components, and setup
- `CONFIG.TXT` — SD card config reference

Shared code lives at the repo root only when two or more boards actually need it. Do not add shared infrastructure speculatively.

## Testing

- Every line of logic added to `logic.h` must have test coverage in `test/native/test_logic.cpp`.
- Write the test first, or add it immediately after the logic change.
- After every code change, run the board's native test suite and confirm all tests pass before moving on.
- To run tests for v0: `cmake -S boards/v0/test/native -B boards/v0/test/native/build && cmake --build boards/v0/test/native/build && ./boards/v0/test/native/build/tests`

## Documentation

- After every file edit, review the board's `README.md` (e.g. `boards/v0/README.md`) and update it if the change affects hardware, wiring, behavior, or setup instructions.
- After every file edit, review the board's `CONFIG.TXT` and update it if the change adds, removes, or modifies any config key.
- Update the root `README.md` only if the overall project structure or board list changes.
