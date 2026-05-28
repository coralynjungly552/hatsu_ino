# hatsu_ino

[![Static Analysis](https://github.com/ArturVRSampaio/hatsu_ino/actions/workflows/cppcheck.yml/badge.svg)](https://github.com/ArturVRSampaio/hatsu_ino/actions/workflows/cppcheck.yml)
[![Unit Tests](https://github.com/ArturVRSampaio/hatsu_ino/actions/workflows/native-tests.yml/badge.svg)](https://github.com/ArturVRSampaio/hatsu_ino/actions/workflows/native-tests.yml)
[![Arduino Build](https://github.com/ArturVRSampaio/hatsu_ino/actions/workflows/arduino-build.yml/badge.svg)](https://github.com/ArturVRSampaio/hatsu_ino/actions/workflows/arduino-build.yml)
[![Coverage](https://codecov.io/gh/ArturVRSampaio/hatsu_ino/branch/master/graph/badge.svg)](https://codecov.io/gh/ArturVRSampaio/hatsu_ino)
[![Deploy Docs](https://github.com/ArturVRSampaio/hatsu_ino/actions/workflows/pages.yml/badge.svg)](https://github.com/ArturVRSampaio/hatsu_ino/actions/workflows/pages.yml)

> **hatsu** (初, first sound) + **ino** (Arduino) — the first sound your car makes.

A JDM car melody box that plays a WAV audio file when you start your car.

## What it does

When your car's ignition turns on, hatsu_ino detects the power-up and plays a WAV file from an SD card through a speaker. Just load any `.wav` files onto the SD card and the device handles the rest.

## Components

| Component | Description | Code |
|---|---|---|
| Arduino Nano Compatible V3 ATmega328 CH340 | Main microcontroller | ATmega328P |
| Micro SD Card Reader Module for Arduino | Reads WAV files via SPI | — |
| PAM8403 Mini Digital Amplifier 2x3W 5V | Drives the speaker | PAM8403 |
| Mini Speaker 0.5W 8Ω 40mm | Audio output | — |
| LM2596 Adjustable Step-Down Buck Converter 3A | Steps 12V car power down to 5V *(optional — only needed if powering from the 12V car line)* | LM2596 |
| Electrolytic Capacitor 10µF 50V 105°C | Audio coupling between D9 and PAM8403 IN+ *(1 required)* | — |
| Micro SD card | Stores WAV files (FAT32 formatted) | — |

### Extra parts needed

| Component | Description |
|---|---|
| Jumper wires / proto board | Connecting components |
| Soldering iron | For permanent connections |
| USB Type-A to Mini-B cable | Uploading firmware to the Nano |
| Car wiring tap or splice connector | Connecting to the ignition-switched power line |
| Patience and time | Required in generous amounts |

## Code structure

| File | Role |
|---|---|
| `hatsu_ino.ino` | Arduino entry point — hardware setup, SD init, sleep, LED error codes. Thin glue; contains no logic of its own. Named after the project because the Arduino IDE requires the sketch file to match its containing folder. |
| `logic.h` | All testable domain logic — config parsing, track selection (random, sequential, shuffle, single), WAV validation, EEPROM index math. Pure C++; no Arduino dependencies. |
| `test/native/test_logic.cpp` | Host-side Catch2 tests for everything in `logic.h`. |

## How it works

1. Car ignition supplies power → either 12V through the LM2596 buck converter (stepped down to 5V), or directly from an existing 5V source
2. Arduino Nano powers on and initializes the SD card module via SPI
3. It picks a random WAV file from the SD card and plays it
4. Audio is sent via PWM (pin D9) → PAM8403 amplifier → speaker
5. Once playback finishes, the board enters deep sleep (~0.1µA) until the next ignition power cycle

## Assembly

### Circuit overview

```
   ┌─────────────────────── Option A — 12V car line ────────────────────────────────┐
   │                   ┌──────────────┐                                             │
   │   CAR 12V+ ──────►│    LM2596    ├──── 5V  ────────────────────── (5V rail)    │
   │   CAR GND  ──────►│  BUCK CONV   ├──── GND ────────────────────── (GND rail)   │
   │                   └──────────────┘                                             │
   └────────────────────────────────────────────────────────────────────────────────┘

                                           ┌──────────────────────────┐
   ┌──────────────────────────────┐        │       SD MODULE          │
   │        ARDUINO NANO          │        │                          │
   │                              │        │  VCC  ◄──── 5V rail      │
   │  5V  ◄──── 5V rail           │        │  GND  ◄──── GND rail     │
   │  GND ◄──── GND rail          │        │                          │
   │  D4  ──────────────────────────────────► CS                      │
   │  D11 ──────────────────────────────────► MOSI                    │
   │  D12 ◄─────────────────────────────────  MISO                    │
   │  D13 ──────────────────────────────────► SCK                     │
   │                              │        └──────────────────────────┘
   │  D9  ──── [+ 10µF ─] ──────────────────────────────────────────────────┐
   └──────────────────────────────┘                                         │
                                                                            │
   ┌──────────────────────────────┐                                         │
   │           PAM8403            │                                         │
   │                              │                                         │
   │  5V+  ◄──── 5V rail          │                                         │
   │  GND  ◄──── GND rail         │                                         │
   │                              │                                         │
   │  R   ◄──────┐ (optional)     │                                         │
   │  L   ◄──────┴──────────────────────────────────────────────────────────┘
   │                              │                                ┌──────────────────┐
   │                              │                                │     SPEAKER      │
   │                              │                                │                  │
   │  L+  ──────────────────────────────────────────────────────────────► POSITIVE    │
   │  L─  ──────────────────────────────────────────────────────────────► NEGATIVE    │
   │                              │      ┌──────────────────┐      └──────────────────┘
   │                              │      │     SPEAKER      │
   │  (optional)                  │      │                  │
   │  R─  ────────────────────────────────────► POSITIVE    │
   │  R+  ────────────────────────────────────► NEGATIVE    │
   │                              │      └──────────────────┘
   └──────────────────────────────┘                          

```

> The diagram above shows **Option A** (12V → LM2596 → 5V rail). For **Option B**, omit the LM2596 and connect your switched 5V source directly to the 5V rail.
>
> On the PAM8403 module, the pin labelled **⏚** (ground symbol, sometimes printed as a `1`) is the audio signal ground — connect it to Arduino GND.

---

### Before you start — choose your power source

**Option A — 12V car line (requires LM2596)**

Do this **before connecting anything else** or you risk frying the Nano.

1. Connect LM2596 IN+ to a 12V source and IN− to GND
2. Put a multimeter on OUT+ and OUT−
3. Turn the small brass trimmer screw until the output reads exactly **5.0V**
4. Disconnect power

**Option B — existing 5V source**

If your car has a switched USB port or a 5V accessory rail that turns on and off with the ignition, you can skip the LM2596 entirely and connect that 5V source directly to the Nano's 5V and GND pins.

---

### Step 1 — Power

All components share a common ground. Every GND point in this guide must be connected together.

**Option A — 12V car line via LM2596**

| From | To |
|---|---|
| Car 12V+ | LM2596 IN+ |
| Car GND | LM2596 IN− |
| LM2596 OUT+ | Arduino Nano **5V** pin |
| LM2596 OUT+ | SD module **VCC** |
| LM2596 OUT+ | PAM8403 **VCC** |
| LM2596 OUT− | Arduino Nano **GND** |
| LM2596 OUT− | SD module **GND** |
| LM2596 OUT− | PAM8403 **GND** |

**Option B — existing 5V source**

| From | To |
|---|---|
| 5V source + | Arduino Nano **5V** pin |
| 5V source + | SD module **VCC** |
| 5V source + | PAM8403 **VCC** |
| 5V source − | Arduino Nano **GND** |
| 5V source − | SD module **GND** |
| 5V source − | PAM8403 **GND** |

> Powering the Nano through the **5V pin** (not VIN) bypasses its onboard regulator — this is correct when supplying a clean 5V externally.

---

### Step 2 — SD card module (SPI)

| SD module pin | Arduino Nano pin |
|---|---|
| VCC | 5V |
| GND | GND |
| CS | D4 |
| MOSI | D11 |
| MISO | D12 |
| SCK | D13 |

---

### Step 3 — Audio (Nano → capacitor → PAM8403 → speaker)

The 10µF capacitor sits between D9 and the amplifier to block the DC offset from the PWM signal. It is **polarized** — the longer leg (+) faces D9, the shorter leg with the stripe (−) faces PAM8403.

| From | To |
|---|---|
| Nano **D9** | Capacitor **+** leg |
| Capacitor **−** leg | PAM8403 **L** |
| Nano **GND** | PAM8403 **⏚** |
| Nano **GND** | PAM8403 **5V−** |
| PAM8403 **5V+** | 5V rail |
| PAM8403 **L+** | Speaker **(+)** |
| PAM8403 **L−** | Speaker **(−)** |

> Pin **⏚** is the audio signal ground — connect it to Arduino GND. Pin **B** is the right channel input — leave it unconnected when using only the left channel.

---

### All connections at a glance

| Wire | From | To |
|---|---|---|
| 12V power *(Option A)* | Car 12V+ | LM2596 IN+ |
| GND *(Option A)* | Car GND | LM2596 IN− |
| 5V rail *(Option A)* | LM2596 OUT+ | Nano 5V, SD VCC, PAM8403 5V+ |
| 5V rail *(Option B)* | 5V source + | Nano 5V, SD VCC, PAM8403 5V+ |
| GND rail | LM2596 OUT− or 5V source − | Nano GND, SD GND, PAM8403 5V− |
| SPI CS | Nano D4 | SD CS |
| SPI MOSI | Nano D11 | SD MOSI |
| SPI MISO | Nano D12 | SD MISO |
| SPI SCK | Nano D13 | SD SCK |
| Audio | Nano D9 | 10µF cap (+) → cap (−) → PAM8403 L |
| Audio GND | Nano GND | PAM8403 ⏚ and PAM8403 5V− |
| Speaker | PAM8403 L+ / L− | Speaker + / − |

## Uploading the firmware

### 1 — Install the Arduino IDE

Download and install the [Arduino IDE](https://www.arduino.cc/en/software) (version 2.x recommended).

### 2 — Install the CH340 driver

The Nano clone listed in the components uses a **CH340** USB chip instead of the genuine FTDI chip. Windows and older macOS versions need an additional driver before the board shows up as a port.

- **Windows / macOS:** download and install the driver from [wch-ic.com](https://www.wch-ic.com/downloads/CH341SER_EXE.html), then replug the USB cable.
- **Linux:** the driver is included in the kernel — no action needed.

### 3 — Install the TMRpcm library

Arduino IDE → **Sketch → Include Library → Manage Libraries** → search [`TMRpcm`](https://github.com/TMRh20/TMRpcm) → install.

### 4 — Open the sketch

File → Open → select `hatsu_ino.ino` from this repository.

### 5 — Select the board and port

Connect the Nano via USB, then in the IDE:

| Setting | Value |
|---|---|
| Board | **Arduino Nano** |
| Processor | **ATmega328P (Old Bootloader)** |
| Port | the COM / tty port that appeared after plugging in |

> Use **Old Bootloader** — most CH340 Nano clones ship with the older bootloader. If the upload fails with a sync error, this is the first thing to try.

### 6 — Upload

Click **Upload** (→ arrow button). The IDE will compile and flash the sketch. The built-in LED will blink briefly during upload, then go dark once the board is ready.

---

## SD card

**Maximum size: 32GB.** Cards larger than 32GB (SDXC) use exFAT by default, which the Arduino SD library does not support. Any card ≤32GB formatted as FAT32 will work — a 4GB or 8GB card is more than enough. At 16kHz 8-bit mono, a WAV file is ~1MB per minute of audio.

### Setup

1. Format the SD card as FAT32
2. Copy any `.wav` files to the root directory
3. Optionally create a `CONFIG.TXT` to customize behaviour (see below)
4. On error, the built-in LED blinks the error code 3 times then the device enters deep sleep to prevent battery drain:
   - **2 blinks** = SD card failed to initialize — either failed after 3 retries or hung the SPI bus (check wiring, reformat, or try a different card)
   - **3 blinks** = no `.wav` files found, file missing, or invalid WAV format
   - **4 blinks** = SD root directory failed to open (try reformatting)
   - **5 blinks** = playback watchdog — track was selected but never started playing

During normal playback the built-in LED pulses briefly once per second to confirm the device is alive.

### File naming rules

The Arduino SD library only supports **8.3 filenames** (a FAT filesystem constraint):
- Max **8 characters** for the name, **3 for the extension**
- Letters, numbers, `_` and `-` only — **no spaces**
- Stored as uppercase internally

| Valid | Invalid |
|---|---|
| `SONG1.WAV` | `my favorite song.wav` (spaces) |
| `STARTUP.WAV` | `startup-melody-2024.wav` (name too long) |
| `MY_TUNE.WAV` | `intro.wave` (extension too long) |

**Maximum 255 files** in the root directory.

### WAV file requirements

The TMRpcm library requires:
- **Format:** PCM WAV (uncompressed)
- **Bit depth:** 8-bit
- **Channels:** Mono
- **Sample rate:** 8000–32000 Hz (16000 Hz recommended)

You can convert any audio file using ffmpeg:
```bash
ffmpeg -i input.mp3 -ar 16000 -ac 1 -acodec pcm_u8 MELODY.WAV
```

### CONFIG.TXT

Place a file named `CONFIG.TXT` in the root of the SD card to customise behaviour without reflashing. The file is optional — if absent, defaults apply. A ready-to-use template is included in the repository as `CONFIG.TXT`.

**Format:** one `KEY=VALUE` pair per line. Lines starting with `#` are treated as comments and ignored. Spaces around the `=` are allowed.

| Key | Values | Default | Description |
|---|---|---|---|
| `VOLUME` | `0` – `6` | `4` | Playback volume. 4 = unity gain. 5–6 = boosted (each step doubles the PWM swing). **Set to 6 when using the PAM8403** — at lower values the PWM average is too low to bias the amplifier input correctly. Values above 6 overflow the PWM timer and cause no audio output. |
| `MODE` | `RANDOM` / `SEQUENTIAL` / `SINGLE` | `RANDOM` | Track selection mode |
| `DELAY` | `0` – `255` | `0` | Seconds to wait after power-up before playing |
| `MIN_SIZE` | `0` – `255` | `0` | Skip WAV files smaller than N kilobytes (0 = no filter) |
| `PLAY_COUNT` | `1` – `255` | `1` | How many times to play the chosen track before sleeping |
| `TRACK` | any valid WAV filename | *(none)* | File to play in `SINGLE` mode |

**Example:**
```
# hatsu_ino config
VOLUME=5
MODE=SEQUENTIAL
```

**MODE=RANDOM** — picks a random WAV file each time the car starts (reservoir sampling, uniform distribution). The last played track is remembered in EEPROM so the same file is never picked twice in a row. If only one file is on the card it plays every time regardless.

**MODE=SEQUENTIAL** — plays WAV files in the order the SD card returns them, advancing by one track each ignition cycle. The current position is stored in the Nano's EEPROM so it survives power-off.

**MODE=SHUFFLE** — plays every track exactly once before any track repeats. Played tracks are tracked via an EEPROM bitmask (2 bytes). Once all tracks have been played the cycle resets. Supports up to 16 tracks; if more are present it falls back to RANDOM mode.

**MODE=SINGLE** — always plays the file specified by `TRACK`. If `TRACK` is not set or the file does not exist on the card, the built-in LED blinks the "no WAV files" error (3 blinks).

## Testing

There are two test suites:

### Native tests (CI)

Tests for all pure logic (`isWav()`, reservoir sampling, config parsing, sequential index resolution) compiled and run on the host with [Catch2](https://github.com/catchorg/Catch2). These run automatically on every push via GitHub Actions.

To run locally:
```bash
cmake -S test/native -B test/native/build
cmake --build test/native/build
./test/native/build/tests
```

Requires CMake ≥ 3.14 and a C++17 compiler. Catch2 is fetched automatically.

### On-hardware tests (AUnit)

Validates the same logic running on the actual Nano using [AUnit](https://github.com/bxparks/AUnit).

**Install AUnit:** Arduino IDE → Library Manager → search `AUnit` → install.

**Run the tests:**
1. Open `test/test_hatsu_ino/test_hatsu_ino.ino` in the Arduino IDE
2. Upload to the Nano
3. Open Serial Monitor at **115200 baud**
4. Results print automatically — all tests should show `PASSED`
