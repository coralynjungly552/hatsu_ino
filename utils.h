#pragma once
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

const size_t  TRACK_NAME_LEN     = 13;  // 8.3 format: max 12 chars + null
const size_t  WAV_HEADER_MIN_SIZE = 12; // RIFF(4) + size(4) + WAVE(4)
const uint8_t SHUFFLE_MAX_TRACKS  = 8;  // bitmask fits one EEPROM byte

inline bool isWav(const char* filename) {
  size_t len = strlen(filename);
  return len > 4 && strcasecmp(filename + len - 4, ".WAV") == 0;
}

inline bool isValidWavHeader(const uint8_t* header) {
  return header[0]=='R' && header[1]=='I' && header[2]=='F' && header[3]=='F' &&
         header[8]=='W' && header[9]=='A' && header[10]=='V' && header[11]=='E';
}

inline bool meetsMinSize(uint32_t fileSize, uint8_t minSizeKb) {
  return minSizeKb == 0 || fileSize >= (uint32_t)minSizeKb * 1024;
}

// P(replace)=1/n for uniform selection; injectable fn enables deterministic testing.
using RandomFn = long (*)(long);

inline bool reservoirShouldReplace(uint8_t n, RandomFn randFn) {
  return randFn(n) == 0;
}

// --- Shuffle helpers (EEPROM bitmask, max SHUFFLE_MAX_TRACKS tracks) ---

inline bool shufflePlayed(uint8_t mask, uint8_t index) {
  return (mask & (uint8_t)(1u << index)) != 0;
}

inline uint8_t shuffleMarkPlayed(uint8_t mask, uint8_t index) {
  return mask | (uint8_t)(1u << index);
}

inline bool shuffleAllPlayed(uint8_t mask, uint8_t total) {
  if (total == 0 || total > SHUFFLE_MAX_TRACKS) return false;
  uint8_t fullMask = (uint8_t)((1u << total) - 1u);
  return (mask & fullMask) == fullMask;
}

enum PlayMode : uint8_t {
  MODE_RANDOM,
  MODE_SEQUENTIAL,
  MODE_SINGLE,
  MODE_SHUFFLE
};

struct Config {
  uint8_t volume;
  PlayMode mode;
  char singleTrack[TRACK_NAME_LEN];
  uint8_t delaySeconds;
  uint8_t minSizeKb;
  uint8_t repeat;       // how many times to play the chosen track (minimum 1)
};

static const Config DEFAULT_CONFIG = {6, MODE_RANDOM, "", 0, 0, 1};

inline void copyTrackName(char* dst, const char* src) {
  strncpy(dst, src, TRACK_NAME_LEN - 1);
  dst[TRACK_NAME_LEN - 1] = '\0';
}

inline void trimRight(char* s, int len) {
  for (int i = len - 1; i >= 0 && s[i] == ' '; i--) s[i] = '\0';
}

// Parses "KEY=VALUE" lines; skips blank lines and '#' comments; trims spaces.
inline bool applyConfigLine(Config& cfg, const char* line) {
  if (!line || line[0] == '#' || line[0] == '\0') return false;

  const char* eq = strchr(line, '=');
  if (!eq) return false;

  size_t keyLen = (size_t)(eq - line);
  char key[16];
  if (keyLen == 0 || keyLen >= sizeof(key)) return false;

  strncpy(key, line, keyLen);
  key[keyLen] = '\0';
  trimRight(key, (int)keyLen);

  const char* valueStart = eq + 1;
  while (*valueStart == ' ') valueStart++;
  char value[16];
  strncpy(value, valueStart, sizeof(value) - 1);
  value[sizeof(value) - 1] = '\0';
  trimRight(value, (int)strlen(value));

  if (strcasecmp(key, "VOLUME") == 0) {
    int v = atoi(value);
    if (v >= 0 && v <= 7) { cfg.volume = (uint8_t)v; return true; }
    return false;
  }
  if (strcasecmp(key, "MODE") == 0) {
    if (strcasecmp(value, "RANDOM") == 0)     { cfg.mode = MODE_RANDOM;     return true; }
    if (strcasecmp(value, "SEQUENTIAL") == 0) { cfg.mode = MODE_SEQUENTIAL; return true; }
    if (strcasecmp(value, "SINGLE") == 0)     { cfg.mode = MODE_SINGLE;     return true; }
    if (strcasecmp(value, "SHUFFLE") == 0)    { cfg.mode = MODE_SHUFFLE;    return true; }
    return false;
  }
  if (strcasecmp(key, "DELAY") == 0) {
    int v = atoi(value);
    if (v >= 0 && v <= 255) { cfg.delaySeconds = (uint8_t)v; return true; }
    return false;
  }
  if (strcasecmp(key, "MIN_SIZE") == 0) {
    int v = atoi(value);
    if (v >= 0 && v <= 255) { cfg.minSizeKb = (uint8_t)v; return true; }
    return false;
  }
  if (strcasecmp(key, "REPEAT") == 0) {
    int v = atoi(value);
    if (v >= 1 && v <= 255) { cfg.repeat = (uint8_t)v; return true; }
    return false;
  }
  if (strcasecmp(key, "TRACK") == 0) {
    if (!isWav(value) || strlen(value) >= TRACK_NAME_LEN) return false;
    copyTrackName(cfg.singleTrack, value);
    return true;
  }
  return false;
}

// 0xFF first-boot EEPROM bytes are not valid filenames — lastPlayed never falsely matches.
inline bool shouldSkipForAntiRepeat(const char* name, const char* lastPlayed) {
  return lastPlayed[0] != '\0' && strcmp(name, lastPlayed) == 0;
}

inline uint8_t resolveSequentialIndex(uint8_t stored, uint8_t total) {
  if (total == 0) return 0;
  return stored >= total ? 0 : stored;
}

inline uint8_t nextSequentialIndex(uint8_t current, uint8_t total) {
  if (total == 0) return 0;
  return (uint8_t)((current + 1) % total);
}
