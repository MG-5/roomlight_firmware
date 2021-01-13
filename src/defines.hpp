#pragma once
#include "protocol.hpp"
#include <algorithm>

constexpr auto PIXELS1 = 47;
constexpr auto PIXELS2 = 38;
constexpr auto PIXELS3 = 46;
constexpr auto MAX_PIXELS = std::max(std::max(PIXELS1, PIXELS2), PIXELS3);

inline LEDSegment ledCurrentData[PIXELS1 + PIXELS2 + PIXELS3]{0};
inline LEDSegment ledTargetData[PIXELS1 + PIXELS2 + PIXELS3]{0};

inline uint16_t ledVoltage;
inline uint16_t ledCurrent;