#pragma once
#include <string.h>

bool isWav(const char* filename) {
  int len = strlen(filename);
  return len > 4 && strcasecmp(filename + len - 4, ".WAV") == 0;
}
