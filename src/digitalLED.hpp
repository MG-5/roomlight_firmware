#pragma once

#include "protocol.hpp"
#include "task.h"
#include <algorithm>

constexpr auto Strip1Pixels = 47;
constexpr auto Strip2Pixels = 37;
constexpr auto Strip3Pixels = 46;

constexpr auto LongestStrip = std::max(std::max(Strip1Pixels, Strip2Pixels), Strip3Pixels);
constexpr auto TotalPixels = Strip1Pixels + Strip2Pixels + Strip3Pixels;

extern TaskHandle_t digitalLEDHandle;
extern TaskHandle_t ledFadingHandle;
extern LEDSegment ledCurrentData[TotalPixels];
extern LEDSegment ledTargetData[TotalPixels];

enum class LightState
{
    Off,
    FullWhite,
    MediumWhite,
    LowWhite,
    Custom,
    System
};

extern LightState currentLightState;