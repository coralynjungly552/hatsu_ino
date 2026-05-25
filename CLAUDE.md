# hatsu_ino — project instructions

## Testing

- Every line of code must have test coverage. Before adding new logic, write the test first or add it immediately after.
- After every code change, run the native test suite and confirm all tests pass before moving on.
- Pure logic belongs in `utils.h` so it can be tested natively without hardware.

## Documentation

- After every file edit, review `README.md` and update it if the change affects hardware, wiring, behavior, or setup instructions.
- After every file edit, review `CONFIG.TXT` and update it if the change adds, removes, or modifies any config key.
