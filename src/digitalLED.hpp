#pragma once

#include "protocol.hpp"
#include "task.h"
#include <algorithm>
#include <array>

constexpr auto Strip1Pixels = 47;
constexpr auto Strip2Pixels = 37;
constexpr auto Strip3Pixels = 46;

constexpr auto NumberOfDataPins = 2;

constexpr auto LongestStrip =
    Strip2Pixels + Strip3Pixels; // std::max(std::max(Strip1Pixels, Strip2Pixels), Strip3Pixels);
constexpr auto TotalPixels = Strip1Pixels + Strip2Pixels + Strip3Pixels;

extern TaskHandle_t digitalLEDHandle;
extern TaskHandle_t ledFadingHandle;
extern TaskHandle_t zeroCheckerHandle;
extern std::array<LEDSegment, TotalPixels> ledCurrentData;
extern std::array<LEDSegment, TotalPixels> ledTargetData;

enum class LightState
{
    Off,
    FullWhite,
    MediumWhite,
    LowWhite,
    Custom,
    System
};

enum class LightMode
{
    BothStrips,
    OnlyStrip1
};

extern LightState currentLightState;
extern LightMode currentLightMode;