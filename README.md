# hatsu_ino

> **hatsu** (初, first sound) + **ino** (Arduino) — the first sound your car makes.

A device that plays a WAV audio file when you start your car.

## What it does

When your car's ignition turns on, hatsu_ino detects the power-up and plays a WAV file from an SD card through a speaker. Just load any `.wav` files onto the SD card and the device handles the rest.

## Components

| Component | Description | Link |
|---|---|---|
| Arduino Nano Compatible V3 ATmega328 CH340 | Main microcontroller | [Mercado Livre](https://www.mercadolivre.com.br/arduino-nano-compativel-v3-atmega328-ch340-com/p/MLB38280753) |
| Micro SD Card Reader Module for Arduino | Reads WAV files via SPI | [Mercado Livre](https://www.mercadolivre.com.br/modulo-leitor-carto-micro-sd-card-leituraescrita-p-arduino/p/MLB2039725040) |
| PAM8403 Mini Digital Amplifier 2x3W 5V (pack of 3) | Drives the speaker | [Mercado Livre](https://www.mercadolivre.com.br/3x-amplificador-mini-digital-2x3w-5v-pam8403/up/MLBU1717645519) |
| Mini Speaker 0.5W 8Ω 40mm (pack of 2) | Audio output | [Mercado Livre](https://www.mercadolivre.com.br/2-x-mini-alto-falante-05w-8-ohms-8r-40mm-para-arduino-esp/up/MLBU1436328953) |
| LM2596 Adjustable Step-Down Buck Converter 3A | Steps 12V car power down to 5V efficiently | [Mercado Livre](https://www.mercadolivre.com.br/regulador-de-tenso-ajustavel-lm2596-step-down-3a-qualidade/p/MLB34351875) |
| Micro SD card | Stores `MELODY.WAV` (FAT32 formatted) | — |

### Extra parts needed

| Component | Description | Link |
|---|---|---|
| Electrolytic Capacitor 10µF 50V 105°C (pack of 100) | Audio coupling between D9 and PAM8403 IN+ | [Mercado Livre](https://www.mercadolivre.com.br/capacitor-eletrolitico-10f-50v-econd-105c-100-pecas-5x11mm/p/MLB49758637) |
| Jumper wires / proto board | Connecting components | — |
| Soldering iron | For permanent connections | — |
| Patience and time | Required in generous amounts | — |

## How it works

1. Car ignition supplies 12V → LM2596 buck converter steps it down to 5V
2. Arduino Nano powers on and initializes the SD card module via SPI
3. It reads `MELODY.WAV` from the SD card
4. Audio is sent via PWM (pin D9) → PAM8403 amplifier → speaker

## Wiring

### Power — 12V car → 5V

Use a DC-DC buck converter module (LM2596 or Mini360). Before connecting anything, **adjust the output to 5V using a multimeter** by turning the trimmer potentiometer on the module.

```
Car 12V ──── Buck converter IN+
Car GND ──── Buck converter IN−
             Buck converter OUT+ ──── Arduino Nano 5V pin
             Buck converter OUT− ──── GND
```

No capacitors needed. Buck converters are far more efficient than linear regulators (LM7805) and won't heat up.

### SD card module (SPI)

| SD module pin | Arduino Nano pin |
|---|---|
| VCC | 5V |
| GND | GND |
| CS  | D4 |
| MOSI | D11 |
| MISO | D12 |
| SCK | D13 |

### Audio — Nano → PAM8403 → Speaker

```
Nano D9 ──── 10µF cap (blocking DC) ──── PAM8403 IN+
Nano GND ──────────────────────────────── PAM8403 IN−
Nano 5V  ──────────────────────────────── PAM8403 VCC
Nano GND ──────────────────────────────── PAM8403 GND

PAM8403 OUT+ ──── Speaker (+)
PAM8403 OUT− ──── Speaker (−)
```

## SD card

**Maximum size: 32GB.** Cards larger than 32GB (SDXC) use exFAT by default, which the Arduino SD library does not support. Any card ≤32GB formatted as FAT32 will work — a 4GB or 8GB card is more than enough. At 16kHz 8-bit mono, a WAV file is ~1MB per minute of audio.

## SD card setup

1. Format the SD card as FAT32
2. Copy any `.wav` files to the root directory — a random one will be picked each time the car starts
3. On error, the built-in LED will blink in a repeating pattern:
   - **2 blinks** = SD card failed to initialize (check wiring or reformat)
   - **3 blinks** = no `.wav` files found on the SD card

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

## Testing

Unit tests cover the `isWav()` filename validation logic using [AUnit](https://github.com/bxparks/AUnit).

**Install AUnit:** Arduino IDE → Library Manager → search `AUnit` → install.

**Run the tests:**
1. Open `test/test_hatsu_ino/test_hatsu_ino.ino` in the Arduino IDE
2. Upload to the Nano
3. Open Serial Monitor at **115200 baud**
4. Results print automatically — all tests should show `PASSED`

## Dependencies

- [TMRpcm](https://github.com/TMRh20/TMRpcm) — install via Arduino Library Manager or manually

## Project status

Work in progress.
