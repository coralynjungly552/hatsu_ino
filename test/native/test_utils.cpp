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
