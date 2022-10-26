#pragma once

#include <cstdint>

constexpr intptr_t RamBase = 0x20000000;
constexpr intptr_t RamBaseBB = 0x22000000;

inline auto getBitBandingPointer(uint32_t address, uint8_t bit)
{
    return reinterpret_cast<volatile uint32_t *>(RamBaseBB + (address - RamBase) * 32 + bit * 4);
}