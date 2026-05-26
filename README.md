# hatsu_ino

[![Static Analysis](https://github.com/ArturVRSampaio/hatsu_ino/actions/workflows/cppcheck.yml/badge.svg)](https://github.com/ArturVRSampaio/hatsu_ino/actions/workflows/cppcheck.yml)
[![Unit Tests](https://github.com/ArturVRSampaio/hatsu_ino/actions/workflows/native-tests.yml/badge.svg)](https://github.com/ArturVRSampaio/hatsu_ino/actions/workflows/native-tests.yml)
[![Arduino Build](https://github.com/ArturVRSampaio/hatsu_ino/actions/workflows/arduino-build.yml/badge.svg)](https://github.com/ArturVRSampaio/hatsu_ino/actions/workflows/arduino-build.yml)
[![Coverage](https://codecov.io/gh/ArturVRSampaio/hatsu_ino/branch/master/graph/badge.svg)](https://codecov.io/gh/ArturVRSampaio/hatsu_ino)

> **hatsu** (初, first sound) + **ino** (Arduino) — the first sound your car makes.

A JDM car melody box that plays a WAV audio file when you start your car.

## Repository layout

```
boards/
  v0/
    logic.h      # board logic (tested natively, no hardware dependency)
    test/        # native unit test suite (Catch2 + CMake)
    hatsu_v0/    # Arduino sketch
    README.md    # wiring, components, and setup for v0
    CONFIG.TXT   # example SD card config file for v0
```

## Boards

| Version | Description |
|---|---|
| [v0](boards/v0/README.md) | Arduino Nano + PAM8403 amplifier, first production run |

## Development

### Native tests

```bash
cmake -S test/native -B test/native/build
cmake --build test/native/build
./test/native/build/tests
```

### Arduino build

Compile for Arduino Nano (ATmega328P old bootloader):

```bash
arduino-cli compile --fqbn arduino:avr:nano:cpu=atmega328old boards/v0/hatsu_v0
```
