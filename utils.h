#pragma once
#include <string.h>
#include <stdlib.h>

const size_t TRACK_NAME_LEN = 13;   // 8.3 format: max 12 chars + null

inline bool isWav(const char* filename) {
  size_t len = strlen(filename);
  return len > 4 && strcasecmp(filename + len - 4, ".WAV") == 0;
}

// Reservoir sampling: P(replace) = 1/n, giving uniform selection over all candidates.
// Accepts a random function so the algorithm can be tested deterministically.
using RandomFn = long (*)(long);

inline bool reservoirShouldReplace(uint8_t n, RandomFn randFn) {
  return randFn(n) == 0;
}

struct Config {
  uint8_t volume;
  bool randomMode;
};

static const Config DEFAULT_CONFIG = {6, true};

// Parses one "KEY=VALUE" line from CONFIG.TXT and applies it to cfg.
// Returns true if the key was recognized and the value was valid.
// Skips blank lines and lines starting with '#'.
// Trims leading/trailing spaces around key and value.
inline bool applyConfigLine(Config& cfg, const char* line) {
  if (!line || line[0] == '#' || line[0] == '\0') return false;

  const char* eq = strchr(line, '=');
  if (!eq) return false;

  size_t keyLen = (size_t)(eq - line);
  if (keyLen == 0 || keyLen >= 16) return false;

  char key[16];
  strncpy(key, line, keyLen);
  key[keyLen] = '\0';
  for (int i = (int)keyLen - 1; i >= 0 && key[i] == ' '; i--) key[i] = '\0';

  const char* vp = eq + 1;
  while (*vp == ' ') vp++;
  char value[16];
  strncpy(value, vp, 15);
  value[15] = '\0';
  for (int i = (int)strlen(value) - 1; i >= 0 && value[i] == ' '; i--) value[i] = '\0';

  if (strcasecmp(key, "VOLUME") == 0) {
    int v = atoi(value);
    if (v >= 0 && v <= 7) { cfg.volume = (uint8_t)v; return true; }
    return false;
  }
  if (strcasecmp(key, "MODE") == 0) {
    if (strcasecmp(value, "SEQUENTIAL") == 0) { cfg.randomMode = false; return true; }
    if (strcasecmp(value, "RANDOM") == 0)     { cfg.randomMode = true;  return true; }
    return false;
  }
  return false;
}

// Returns true when name should be excluded from random selection.
// An empty or uninitialized lastPlayed (first boot: 0xFF bytes) will never match a valid filename.
inline bool shouldSkipForAntiRepeat(const char* name, const char* lastPlayed) {
  return lastPlayed[0] != '\0' && strcmp(name, lastPlayed) == 0;
}

// Maps a stored EEPROM index to a valid play index, handling first-boot (0xFF) and wrap.
inline uint8_t resolveSequentialIndex(uint8_t stored, uint8_t total) {
  if (total == 0) return 0;
  return stored >= total ? 0 : stored;
}

// Returns the index to persist after playing current, wrapping at total.
inline uint8_t nextSequentialIndex(uint8_t current, uint8_t total) {
  if (total == 0) return 0;
  return (uint8_t)((current + 1) % total);
}
