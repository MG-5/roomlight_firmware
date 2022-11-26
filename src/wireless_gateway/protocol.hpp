#pragma once

#include <cstdint>

namespace operation
{
constexpr auto Set = 1;
constexpr auto Get = 2;
constexpr auto State = 3;
} // namespace operation

namespace command
{
// commands without payload
constexpr auto FadeSoft = 0x1;
constexpr auto FadeHard = 0x2;
constexpr auto WarmWhite = 0x3;
constexpr auto NeutralWhite = 0x4;
constexpr auto AllStrips = 0x5;
constexpr auto OneStrips = 0x6;
constexpr auto LightsLow = 0x7;
constexpr auto LightsMedium = 0x8;
constexpr auto LightsHigh = 0x9;
constexpr auto LightsOff = 0xA;
// 0xB - 0xD are free
constexpr auto EspOtaStarted = 0xE;
constexpr auto EspOtaFinished = 0xF;

// commands with dedicated payload
constexpr auto FullLedData = 0x10;
constexpr auto SetSegment = 0x11;
constexpr auto GetCurrent = 0x12;
constexpr auto GetVoltage = 0x13;
constexpr auto GetErrorCode = 0x14;
constexpr auto SetDebugPrint = 0x15;
constexpr auto Brightness = 0x16;
// 0x17 - 0x1F are free
} // namespace command

constexpr auto CommandWithPayloadStartIndex = 0x10;

namespace reponse
{
constexpr auto ShiftPosition = 5;
constexpr auto Okay = 0x00;
constexpr auto OperationFailed = 0x01;
constexpr auto CrcFailed = 0x02;
constexpr auto NotSupported = 0x03;
} // namespace reponse

constexpr auto ProtocolMagic = 0xBEEF;

// packet header
struct PacketHeader
{
    uint8_t operation = 0;
    uint8_t command = 0;
    uint16_t magic = ProtocolMagic;
    uint16_t payloadSize = 0;
} __attribute__((packed));

constexpr auto Foo = sizeof(PacketHeader);