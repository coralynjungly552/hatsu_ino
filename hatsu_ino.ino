#include <SD.h>
#include <SPI.h>
#include <TMRpcm.h>
#include "utils.h"

#define SD_CS_PIN    4
#define SPEAKER_PIN  9
#define VOLUME       6        // 0 (silent) to 7 (max)

TMRpcm audio;
char selectedFile[13];        // 8.3 filename format: max 12 chars + null

void blinkError(uint8_t times) {
  pinMode(LED_BUILTIN, OUTPUT);
  while (true) {
    for (uint8_t i = 0; i < times; i++) {
      digitalWrite(LED_BUILTIN, HIGH);
      delay(200);
      digitalWrite(LED_BUILTIN, LOW);
      delay(200);
    }
    delay(800);
  }
}

void pickRandomWav() {
  uint8_t count = 0;

  File root = SD.open("/");
  while (true) {
    File f = root.openNextFile();
    if (!f) break;
    if (!f.isDirectory() && isWav(f.name())) count++;
    f.close();
  }
  root.close();

  if (count == 0) blinkError(3);

  uint8_t pick = random(count);

  root = SD.open("/");
  uint8_t index = 0;
  while (true) {
    File f = root.openNextFile();
    if (!f) break;
    if (!f.isDirectory() && isWav(f.name())) {
      if (index == pick) {
        strncpy(selectedFile, f.name(), sizeof(selectedFile) - 1);
        selectedFile[sizeof(selectedFile) - 1] = '\0';
        f.close();
        break;
      }
      index++;
    }
    f.close();
  }
  root.close();
}

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);

  randomSeed(analogRead(A0)); // floating pin reads electrical noise for seed

  if (!SD.begin(SD_CS_PIN)) blinkError(2);

  pickRandomWav();

  audio.speakerPin = SPEAKER_PIN;
  audio.volume(VOLUME);
  audio.play(selectedFile);
}

void loop() {
  // TMRpcm plays via hardware interrupt — nothing needed here
}
