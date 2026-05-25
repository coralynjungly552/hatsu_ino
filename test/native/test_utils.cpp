#include <catch2/catch_test_macros.hpp>
#include <cstdlib>
#include "utils.h"

// --- isWav ---

TEST_CASE("isWav accepts uppercase WAV extension", "[isWav]") {
    REQUIRE(isWav("SONG.WAV"));
}

TEST_CASE("isWav accepts lowercase WAV extension", "[isWav]") {
    REQUIRE(isWav("song.wav"));
}

TEST_CASE("isWav accepts mixed-case WAV extension", "[isWav]") {
    REQUIRE(isWav("Song.Wav"));
}

TEST_CASE("isWav accepts single-character name", "[isWav]") {
    REQUIRE(isWav("A.WAV"));
}

TEST_CASE("isWav accepts max 8-character name", "[isWav]") {
    REQUIRE(isWav("ABCDEFGH.WAV"));
}

TEST_CASE("isWav rejects filename with no extension", "[isWav]") {
    REQUIRE_FALSE(isWav("MELODY"));
}

TEST_CASE("isWav rejects wrong extension", "[isWav]") {
    REQUIRE_FALSE(isWav("SONG.MP3"));
}

TEST_CASE("isWav rejects extension longer than 3 chars", "[isWav]") {
    REQUIRE_FALSE(isWav("SONG.WAVE"));
}

TEST_CASE("isWav rejects empty string", "[isWav]") {
    REQUIRE_FALSE(isWav(""));
}

TEST_CASE("isWav rejects extension with no name", "[isWav]") {
    REQUIRE_FALSE(isWav(".WAV"));
}

TEST_CASE("isWav rejects lone dot", "[isWav]") {
    REQUIRE_FALSE(isWav("."));
}

// --- applyConfigLine ---

TEST_CASE("applyConfigLine ignores blank and comment lines", "[config]") {
    Config cfg = DEFAULT_CONFIG;
    REQUIRE_FALSE(applyConfigLine(cfg, ""));
    REQUIRE_FALSE(applyConfigLine(cfg, "# this is a comment"));
    REQUIRE_FALSE(applyConfigLine(cfg, "# VOLUME=3"));
}

TEST_CASE("applyConfigLine ignores unknown keys", "[config]") {
    Config cfg = DEFAULT_CONFIG;
    REQUIRE_FALSE(applyConfigLine(cfg, "UNKNOWN=VALUE"));
    REQUIRE_FALSE(applyConfigLine(cfg, "NOEQUALS"));
}

TEST_CASE("applyConfigLine VOLUME accepts valid range", "[config]") {
    Config cfg = DEFAULT_CONFIG;
    REQUIRE(applyConfigLine(cfg, "VOLUME=0")); REQUIRE(cfg.volume == 0);
    REQUIRE(applyConfigLine(cfg, "VOLUME=3")); REQUIRE(cfg.volume == 3);
    REQUIRE(applyConfigLine(cfg, "VOLUME=7")); REQUIRE(cfg.volume == 7);
}

TEST_CASE("applyConfigLine VOLUME rejects out-of-range values", "[config]") {
    Config cfg = DEFAULT_CONFIG;
    cfg.volume = 6;
    REQUIRE_FALSE(applyConfigLine(cfg, "VOLUME=8"));
    REQUIRE(cfg.volume == 6);
}

TEST_CASE("applyConfigLine VOLUME is case-insensitive on key", "[config]") {
    Config cfg = DEFAULT_CONFIG;
    REQUIRE(applyConfigLine(cfg, "volume=4")); REQUIRE(cfg.volume == 4);
}

TEST_CASE("applyConfigLine MODE=SEQUENTIAL sets mode", "[config]") {
    Config cfg = DEFAULT_CONFIG;
    REQUIRE(applyConfigLine(cfg, "MODE=SEQUENTIAL"));
    REQUIRE(cfg.mode == MODE_SEQUENTIAL);
}

TEST_CASE("applyConfigLine MODE=RANDOM sets mode", "[config]") {
    Config cfg = DEFAULT_CONFIG;
    cfg.mode = MODE_SEQUENTIAL;
    REQUIRE(applyConfigLine(cfg, "MODE=RANDOM"));
    REQUIRE(cfg.mode == MODE_RANDOM);
}

TEST_CASE("applyConfigLine MODE=SINGLE sets mode", "[config]") {
    Config cfg = DEFAULT_CONFIG;
    REQUIRE(applyConfigLine(cfg, "MODE=SINGLE"));
    REQUIRE(cfg.mode == MODE_SINGLE);
}

TEST_CASE("applyConfigLine MODE rejects unknown values", "[config]") {
    Config cfg = DEFAULT_CONFIG;
    REQUIRE_FALSE(applyConfigLine(cfg, "MODE=LOOP"));
    REQUIRE(cfg.mode == MODE_RANDOM);
}

TEST_CASE("applyConfigLine is case-insensitive on key and value", "[config]") {
    Config cfg = DEFAULT_CONFIG;
    REQUIRE(applyConfigLine(cfg, "mode=sequential"));
    REQUIRE(cfg.mode == MODE_SEQUENTIAL);
}

TEST_CASE("applyConfigLine TRACK accepts valid WAV filename", "[config]") {
    Config cfg = DEFAULT_CONFIG;
    REQUIRE(applyConfigLine(cfg, "TRACK=MYSONG.WAV"));
    REQUIRE(strcmp(cfg.singleTrack, "MYSONG.WAV") == 0);
}

TEST_CASE("applyConfigLine TRACK rejects non-WAV filename", "[config]") {
    Config cfg = DEFAULT_CONFIG;
    REQUIRE_FALSE(applyConfigLine(cfg, "TRACK=MYSONG.MP3"));
    REQUIRE(cfg.singleTrack[0] == '\0');
}

TEST_CASE("applyConfigLine TRACK rejects filename that exceeds 8.3 length", "[config]") {
    Config cfg = DEFAULT_CONFIG;
    REQUIRE_FALSE(applyConfigLine(cfg, "TRACK=TOOLONGNAME.WAV"));
    REQUIRE(cfg.singleTrack[0] == '\0');
}

TEST_CASE("applyConfigLine trims spaces around key and value", "[config]") {
    Config cfg = DEFAULT_CONFIG;
    REQUIRE(applyConfigLine(cfg, "VOLUME = 5 ")); REQUIRE(cfg.volume == 5);
    REQUIRE(applyConfigLine(cfg, "MODE = SEQUENTIAL ")); REQUIRE(cfg.mode == MODE_SEQUENTIAL);
}

TEST_CASE("applyConfigLine applies multiple lines in sequence", "[config]") {
    Config cfg = DEFAULT_CONFIG;
    applyConfigLine(cfg, "VOLUME=2");
    applyConfigLine(cfg, "MODE=SINGLE");
    applyConfigLine(cfg, "TRACK=BOOT.WAV");
    REQUIRE(cfg.volume == 2);
    REQUIRE(cfg.mode == MODE_SINGLE);
    REQUIRE(strcmp(cfg.singleTrack, "BOOT.WAV") == 0);
}

// --- applyConfigLine DELAY ---

TEST_CASE("applyConfigLine DELAY accepts valid range", "[config]") {
    Config cfg = DEFAULT_CONFIG;
    REQUIRE(applyConfigLine(cfg, "DELAY=0"));   REQUIRE(cfg.delaySeconds == 0);
    REQUIRE(applyConfigLine(cfg, "DELAY=5"));   REQUIRE(cfg.delaySeconds == 5);
    REQUIRE(applyConfigLine(cfg, "DELAY=255")); REQUIRE(cfg.delaySeconds == 255);
}

TEST_CASE("applyConfigLine DELAY rejects out-of-range values", "[config]") {
    Config cfg = DEFAULT_CONFIG;
    REQUIRE_FALSE(applyConfigLine(cfg, "DELAY=256"));
    REQUIRE(cfg.delaySeconds == 0);
}

TEST_CASE("applyConfigLine DELAY default is 0", "[config]") {
    Config cfg = DEFAULT_CONFIG;
    REQUIRE(cfg.delaySeconds == 0);
}

// --- resolveSequentialIndex ---

TEST_CASE("resolveSequentialIndex returns stored when within range", "[sequential]") {
    REQUIRE(resolveSequentialIndex(0, 3) == 0);
    REQUIRE(resolveSequentialIndex(2, 3) == 2);
}

TEST_CASE("resolveSequentialIndex wraps to 0 when stored >= total", "[sequential]") {
    REQUIRE(resolveSequentialIndex(3, 3) == 0);
    REQUIRE(resolveSequentialIndex(5, 3) == 0);
    REQUIRE(resolveSequentialIndex(255, 3) == 0); // first-boot EEPROM value
}

TEST_CASE("resolveSequentialIndex returns 0 when total is 0", "[sequential]") {
    REQUIRE(resolveSequentialIndex(0, 0) == 0);
    REQUIRE(resolveSequentialIndex(255, 0) == 0);
}

// --- nextSequentialIndex ---

TEST_CASE("nextSequentialIndex increments within range", "[sequential]") {
    REQUIRE(nextSequentialIndex(0, 3) == 1);
    REQUIRE(nextSequentialIndex(1, 3) == 2);
}

TEST_CASE("nextSequentialIndex wraps at total", "[sequential]") {
    REQUIRE(nextSequentialIndex(2, 3) == 0);
}

TEST_CASE("nextSequentialIndex stays at 0 for single file", "[sequential]") {
    REQUIRE(nextSequentialIndex(0, 1) == 0);
}

TEST_CASE("nextSequentialIndex returns 0 when total is 0", "[sequential]") {
    REQUIRE(nextSequentialIndex(0, 0) == 0);
}

// --- shouldSkipForAntiRepeat ---

TEST_CASE("shouldSkipForAntiRepeat skips when name matches lastPlayed", "[antirepeat]") {
    REQUIRE(shouldSkipForAntiRepeat("SONG.WAV", "SONG.WAV"));
}

TEST_CASE("shouldSkipForAntiRepeat does not skip a different track", "[antirepeat]") {
    REQUIRE_FALSE(shouldSkipForAntiRepeat("SONG.WAV", "OTHER.WAV"));
}

TEST_CASE("shouldSkipForAntiRepeat does not skip on first boot (empty lastPlayed)", "[antirepeat]") {
    REQUIRE_FALSE(shouldSkipForAntiRepeat("SONG.WAV", ""));
}

TEST_CASE("shouldSkipForAntiRepeat is case-sensitive (SD filenames are uppercase)", "[antirepeat]") {
    REQUIRE_FALSE(shouldSkipForAntiRepeat("SONG.WAV", "song.wav"));
}

TEST_CASE("shouldSkipForAntiRepeat single-file edge case: only file is last played", "[antirepeat]") {
    // When the only WAV matches lastPlayed, shouldSkip returns true for it.
    // pickRandomWav detects scanned==0 and falls back to playing it anyway.
    REQUIRE(shouldSkipForAntiRepeat("ONLY.WAV", "ONLY.WAV"));
}

// --- reservoirShouldReplace ---

static long always_zero(long)    { return 0; }
static long always_nonzero(long) { return 1; }
static long seeded_rand(long n)  { return std::rand() % n; }

TEST_CASE("reservoirShouldReplace always selects the first item", "[reservoir]") {
    // randFn(1) can only return 0, so the first item must always be picked
    REQUIRE(reservoirShouldReplace(1, always_zero));
}

TEST_CASE("reservoirShouldReplace replaces when randFn returns 0", "[reservoir]") {
    REQUIRE(reservoirShouldReplace(5, always_zero));
}

TEST_CASE("reservoirShouldReplace keeps current when randFn returns non-zero", "[reservoir]") {
    REQUIRE_FALSE(reservoirShouldReplace(2, always_nonzero));
    REQUIRE_FALSE(reservoirShouldReplace(5, always_nonzero));
}

TEST_CASE("reservoir sampling selects each of N items with equal probability", "[reservoir]") {
    const int TRIALS = 100000;
    const int N      = 5;
    int counts[N + 1] = {};

    std::srand(42);
    for (int t = 0; t < TRIALS; t++) {
        int selected = 1;
        for (int i = 1; i <= N; i++) {
            if (reservoirShouldReplace(i, seeded_rand))
                selected = i;
        }
        counts[selected]++;
    }

    // Each item should land ~TRIALS/N times; allow ±10% tolerance
    double expected = static_cast<double>(TRIALS) / N;
    for (int i = 1; i <= N; i++) {
        REQUIRE(counts[i] > expected * 0.90);
        REQUIRE(counts[i] < expected * 1.10);
    }
}
