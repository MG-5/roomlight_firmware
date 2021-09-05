#pragma once

#include "gcem/include/gcem.hpp"
#include <array>

constexpr auto PwmResolutionInBit = 8;

constexpr auto GammaFactor = 2.5;
constexpr auto MaximumIn = 255;
constexpr auto MaximumOut = (1 << PwmResolutionInBit) - 1;

using GammaTable = std::array<uint16_t, MaximumIn + 1>;

constexpr GammaTable createGammaTable()
{
    GammaTable gammaTable{};

    for (auto i = 0; i <= MaximumIn; i++)
    {
        const auto Logarithm =
            gcem::pow(static_cast<float>(i) / static_cast<float>(MaximumIn), GammaFactor);

        gammaTable[i] = gcem::round(Logarithm * MaximumOut);
    }

    return gammaTable;
}

constexpr GammaTable GammaLUT = createGammaTable();
