#pragma once

#include <array>
#include <cstdint>

static constexpr auto Strip1Pixels = 47;
static constexpr auto Strip2Pixels = 37;
static constexpr auto Strip3Pixels = 46;

static constexpr auto NumberOfDataPins = 2;

static constexpr auto LongestStrip =
    Strip2Pixels + Strip3Pixels; // std::max(std::max(Strip1Pixels, Strip2Pixels), Strip3Pixels);

static constexpr auto TotalPixels = Strip1Pixels + Strip2Pixels + Strip3Pixels;

struct LedSegment
{
    uint8_t green = 0;
    uint8_t red = 0;
    uint8_t blue = 0;
    uint8_t white = 0;
};

using LedArray = std::array<LedSegment, TotalPixels>;