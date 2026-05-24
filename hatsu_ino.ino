#include <SD.h>
#include <SPI.h>
#include <TMRpcm.h>
#include <avr/sleep.h>
#include <avr/power.h>
#include "utils.h"

const uint8_t SD_CS_PIN              = 4;
const uint8_t SPEAKER_PIN            = 9;
const uint8_t VOLUME                 = 6;    // 0 (silent) to 7 (max)
const int NOISE_PIN                  = A0;   // intentionally unconnected — reads electrical noise for random seed
const unsigned long PLAYBACK_START_DELAY_MS = 100;

enum ErrorCode {
  SD_INIT_FAILED  = 2,
  NO_WAV_FILES    = 3,
  ROOT_DIR_FAILED = 4
};

const unsigned long BLINK_DURATION_MS = 200;
const unsigned long BLINK_PAUSE_MS    = 800;

TMRpcm player;

void setup() {
  initStatusLed();
  seedRandom();
  initSD();

  char trackName[TRACK_NAME_LEN];
  pickRandomWav(trackName);
  configureAndPlay(trackName);

  delay(PLAYBACK_START_DELAY_MS);
}

void loop() {
  if (!player.isPlaying()) {
    enterPowerDownSleep();
  }
}

void initStatusLed() {
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
}

void seedRandom() {
  randomSeed(analogRead(NOISE_PIN));
}

void initSD() {
  if (!SD.begin(SD_CS_PIN)) haltWithErrorCode(SD_INIT_FAILED);
}

void pickRandomWav(char* trackName) {
  uint8_t wavFilesScanned = 0;
  File rootDir = SD.open("/");
  if (!rootDir) haltWithErrorCode(ROOT_DIR_FAILED);
  while (true) {
    File entry = rootDir.openNextFile();
    if (!entry) break;
    if (!entry.isDirectory() && isWav(entry.name())) {
      wavFilesScanned++;
      if (random(wavFilesScanned) == 0) {
        strncpy(trackName, entry.name(), TRACK_NAME_LEN - 1);
        trackName[TRACK_NAME_LEN - 1] = '\0';
      }
    }
    entry.close();
  }
  rootDir.close();
  if (wavFilesScanned == 0) haltWithErrorCode(NO_WAV_FILES);
}

void configureAndPlay(const char* trackName) {
  player.speakerPin = SPEAKER_PIN;
  player.volume(VOLUME);
  player.play(trackName);
}

// ~20mA active → ~0.1µA in power-down. Wakes on next ignition power cycle.
void enterPowerDownSleep() {
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  sleep_enable();
  power_all_disable();
  sleep_mode();
}

void haltWithErrorCode(ErrorCode code) {
  while (true) {
    for (uint8_t i = 0; i < code; i++) {
      digitalWrite(LED_BUILTIN, HIGH);
      delay(BLINK_DURATION_MS);
      digitalWrite(LED_BUILTIN, LOW);
      delay(BLINK_DURATION_MS);
    }
    delay(BLINK_PAUSE_MS);
  }
}
