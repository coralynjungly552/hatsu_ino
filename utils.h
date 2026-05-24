#pragma once
#include <string.h>

const size_t TRACK_NAME_LEN = 13;   // 8.3 format: max 12 chars + null

bool isWav(const char* filename) {
  size_t len = strlen(filename);
  return len > 4 && strcasecmp(filename + len - 4, ".WAV") == 0;
}

// Reservoir sampling: P(replace) = 1/n, giving uniform selection over all candidates.
// Accepts a random function so the algorithm can be tested deterministically.
using RandomFn = long (*)(long);

inline bool reservoirShouldReplace(uint8_t n, RandomFn randFn) {
  return randFn(n) == 0;
}
