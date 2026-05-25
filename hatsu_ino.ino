#include <SD.h>
#include <SPI.h>
#include <TMRpcm.h>
#include <EEPROM.h>
#include <avr/sleep.h>
#include <avr/power.h>
#include "utils.h"

const uint8_t  SD_CS_PIN              = 4;
const uint8_t  SPEAKER_PIN            = 9;
const int      NOISE_PIN              = A0;   // intentionally unconnected — reads electrical noise for random seed
const uint8_t  EEPROM_TRACK_ADDR      = 0;    // sequential mode: play index (1 byte)
const uint8_t  EEPROM_LAST_PLAYED_ADDR = 1;   // random mode: last played filename (TRACK_NAME_LEN bytes)

const uint8_t      SD_INIT_RETRIES      = 3;
const unsigned long SD_RETRY_DELAY_MS   = 500;
const unsigned long PLAYBACK_WATCHDOG_MS = 2000;
const uint8_t      ERROR_BLINK_CYCLES   = 3;

enum ErrorCode {
  SD_INIT_FAILED   = 2,
  NO_WAV_FILES     = 3,
  ROOT_DIR_FAILED  = 4,
  PLAYBACK_TIMEOUT = 5
};

const unsigned long BLINK_DURATION_MS = 200;
const unsigned long BLINK_PAUSE_MS    = 800;

TMRpcm player;
bool playbackObserved      = false;
unsigned long playbackDeadline = 0;

void haltWithErrorCode(ErrorCode code) __attribute__((noreturn));

void setup() {
  initStatusLed();
  seedRandom();
  initSD();

  Config cfg = loadConfig();
  if (cfg.delaySeconds > 0) delay((unsigned long)cfg.delaySeconds * 1000UL);
  char trackName[TRACK_NAME_LEN];
  pickWav(trackName, cfg);
  validateWavFile(trackName);
  configureAndPlay(trackName, cfg.volume);
}

void loop() {
  if (player.isPlaying()) {
    playbackObserved = true;
    digitalWrite(LED_BUILTIN, (millis() % 1000UL < 100UL) ? HIGH : LOW);
    return;
  }
  digitalWrite(LED_BUILTIN, LOW);
  if (playbackObserved)            enterPowerDownSleep();
  if (millis() > playbackDeadline) haltWithErrorCode(PLAYBACK_TIMEOUT);
}

void initStatusLed() {
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
}

void seedRandom() {
  randomSeed(analogRead(NOISE_PIN));
}

void initSD() {
  for (uint8_t attempt = 0; attempt < SD_INIT_RETRIES; attempt++) {
    if (SD.begin(SD_CS_PIN)) return;
    delay(SD_RETRY_DELAY_MS);
  }
  haltWithErrorCode(SD_INIT_FAILED);
}

Config loadConfig() {
  Config cfg = DEFAULT_CONFIG;
  File f = SD.open("CONFIG.TXT");
  if (!f) return cfg;
  char line[32];
  uint8_t pos = 0;
  while (f.available()) {
    char c = (char)f.read();
    if (c == '\n' || c == '\r') {
      if (pos > 0) { line[pos] = '\0'; applyConfigLine(cfg, line); pos = 0; }
    } else if (pos < (uint8_t)(sizeof(line) - 1)) {
      line[pos++] = c;
    }
  }
  if (pos > 0) { line[pos] = '\0'; applyConfigLine(cfg, line); }
  f.close();
  return cfg;
}

void validateWavFile(const char* trackName) {
  File f = SD.open(trackName);
  if (!f) haltWithErrorCode(NO_WAV_FILES);
  uint8_t header[WAV_HEADER_MIN_SIZE];
  bool valid = (f.read(header, WAV_HEADER_MIN_SIZE) == (int)WAV_HEADER_MIN_SIZE)
               && isValidWavHeader(header);
  f.close();
  if (!valid) haltWithErrorCode(NO_WAV_FILES);
}

void pickWav(char* trackName, const Config& cfg) {
  switch (cfg.mode) {
    case MODE_RANDOM: {
      char lastPlayed[TRACK_NAME_LEN];
      readLastPlayed(lastPlayed);
      pickRandomWav(trackName, lastPlayed, cfg.minSizeKb);
      writeLastPlayed(trackName);
      break;
    }
    case MODE_SEQUENTIAL:
      pickSequentialWav(trackName, cfg.minSizeKb);
      break;
    case MODE_SINGLE:
      pickSingleWav(trackName, cfg.singleTrack);
      break;
  }
}

void readLastPlayed(char* name) {
  for (uint8_t i = 0; i < TRACK_NAME_LEN; i++)
    name[i] = (char)EEPROM.read(EEPROM_LAST_PLAYED_ADDR + i);
  name[TRACK_NAME_LEN - 1] = '\0';
}

void writeLastPlayed(const char* name) {
  for (uint8_t i = 0; i < TRACK_NAME_LEN; i++)
    EEPROM.write(EEPROM_LAST_PLAYED_ADDR + i, (uint8_t)name[i]);
}

void pickRandomWav(char* trackName, const char* lastPlayed, uint8_t minSizeKb) {
  uint8_t scanned = 0;
  char fallback[TRACK_NAME_LEN] = "";

  File rootDir = SD.open("/");
  if (!rootDir) haltWithErrorCode(ROOT_DIR_FAILED);
  while (true) {
    File entry = rootDir.openNextFile();
    if (!entry) break;
    if (!entry.isDirectory() && isWav(entry.name()) && meetsMinSize(entry.size(), minSizeKb)) {
      if (fallback[0] == '\0') {
        strncpy(fallback, entry.name(), TRACK_NAME_LEN - 1);
        fallback[TRACK_NAME_LEN - 1] = '\0';
      }
      if (!shouldSkipForAntiRepeat(entry.name(), lastPlayed)) {
        scanned++;
        if (reservoirShouldReplace(scanned, random)) {
          strncpy(trackName, entry.name(), TRACK_NAME_LEN - 1);
          trackName[TRACK_NAME_LEN - 1] = '\0';
        }
      }
    }
    entry.close();
  }
  rootDir.close();

  if (fallback[0] == '\0') haltWithErrorCode(NO_WAV_FILES);
  // Only one qualifying file and it was the last played — no alternative, repeat it
  if (scanned == 0) {
    strncpy(trackName, fallback, TRACK_NAME_LEN - 1);
    trackName[TRACK_NAME_LEN - 1] = '\0';
  }
}

uint8_t countWavFiles(uint8_t minSizeKb) {
  File rootDir = SD.open("/");
  if (!rootDir) haltWithErrorCode(ROOT_DIR_FAILED);
  uint8_t count = 0;
  while (true) {
    File entry = rootDir.openNextFile();
    if (!entry) break;
    if (!entry.isDirectory() && isWav(entry.name()) && meetsMinSize(entry.size(), minSizeKb)) count++;
    entry.close();
  }
  rootDir.close();
  return count;
}

void pickSequentialWav(char* trackName, uint8_t minSizeKb) {
  uint8_t total = countWavFiles(minSizeKb);
  if (total == 0) haltWithErrorCode(NO_WAV_FILES);

  uint8_t stored = EEPROM.read(EEPROM_TRACK_ADDR);
  uint8_t idx    = resolveSequentialIndex(stored, total);

  trackName[0] = '\0';
  File rootDir = SD.open("/");
  if (!rootDir) haltWithErrorCode(ROOT_DIR_FAILED);
  uint8_t current = 0;
  while (true) {
    File entry = rootDir.openNextFile();
    if (!entry) break;
    if (!entry.isDirectory() && isWav(entry.name()) && meetsMinSize(entry.size(), minSizeKb)) {
      if (current == idx) {
        strncpy(trackName, entry.name(), TRACK_NAME_LEN - 1);
        trackName[TRACK_NAME_LEN - 1] = '\0';
        entry.close();
        break;
      }
      current++;
    }
    entry.close();
  }
  rootDir.close();

  if (trackName[0] == '\0') haltWithErrorCode(NO_WAV_FILES);
  EEPROM.write(EEPROM_TRACK_ADDR, nextSequentialIndex(idx, total));
}

void pickSingleWav(char* trackName, const char* singleTrack) {
  if (singleTrack[0] == '\0') haltWithErrorCode(NO_WAV_FILES);
  File f = SD.open(singleTrack);
  if (!f) haltWithErrorCode(NO_WAV_FILES);
  f.close();
  strncpy(trackName, singleTrack, TRACK_NAME_LEN - 1);
  trackName[TRACK_NAME_LEN - 1] = '\0';
}

void configureAndPlay(const char* trackName, uint8_t volume) {
  player.speakerPin = SPEAKER_PIN;
  player.volume(volume);
  player.play(trackName);
  playbackDeadline = millis() + PLAYBACK_WATCHDOG_MS;
}

// ~20mA active → ~0.1µA in power-down. Wakes on next ignition power cycle.
void enterPowerDownSleep() {
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  sleep_enable();
  power_all_disable();
  sleep_mode();
}

void haltWithErrorCode(ErrorCode code) {
  for (uint8_t cycle = 0; cycle < ERROR_BLINK_CYCLES; cycle++) {
    for (uint8_t i = 0; i < (uint8_t)code; i++) {
      digitalWrite(LED_BUILTIN, HIGH);
      delay(BLINK_DURATION_MS);
      digitalWrite(LED_BUILTIN, LOW);
      delay(BLINK_DURATION_MS);
    }
    delay(BLINK_PAUSE_MS);
  }
  enterPowerDownSleep();
}
