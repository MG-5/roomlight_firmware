#pragma once

#include "protocol.hpp"
#include <array>

static constexpr auto Strip1Pixels = 47;
static constexpr auto Strip2Pixels = 37;
static constexpr auto Strip3Pixels = 46;

static constexpr auto NumberOfDataPins = 2;

static constexpr auto LongestStrip =
    Strip2Pixels + Strip3Pixels; // std::max(std::max(Strip1Pixels, Strip2Pixels), Strip3Pixels);

static constexpr auto TotalPixels = Strip1Pixels + Strip2Pixels + Strip3Pixels;

using LedArray = std::array<LedSegment, TotalPixels>;