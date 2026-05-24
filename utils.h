#pragma once
#include <string.h>

const size_t TRACK_NAME_LEN = 13;   // 8.3 format: max 12 chars + null

bool isWav(const char* filename) {
  size_t len = strlen(filename);
  return len > 4 && strcasecmp(filename + len - 4, ".WAV") == 0;
}
