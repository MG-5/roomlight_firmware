#pragma once

#include <cstdint>

// clang-format off

// packet commands
constexpr auto EspConnectionTest    = 0x01;
constexpr auto EspGetMacAddress     = 0x02;
constexpr auto EspRestart           = 0x03;
// = 0x04 - = 0x0F reserved for ESP

constexpr auto LedSetAll            = 0x10;
constexpr auto LedSetSegments       = 0x11;
constexpr auto LedFadeSoft          = 0x12;
constexpr auto LedFadeHard          = 0x13;
constexpr auto LedGetCurrent        = 0x14;
constexpr auto LedGetVoltage        = 0x15;
constexpr auto LedGetErrorCode      = 0x16;
constexpr auto LedEspOtaStarted     = 0x17;
constexpr auto LedEspOtaFinished    = 0x18;
constexpr auto LedSetStatusLed      = 0x19;
constexpr auto LedDebugPrint        = 0x1A;

// packet status
constexpr auto ResponseMask             = 0x80;
constexpr auto ResponseOkay             = 0x00;
constexpr auto ResponseOperationFailed  = 0x01;
constexpr auto ResponseCrcFailed        = 0x02;
constexpr auto ResponseNotSupported     = 0x03;
constexpr auto ResponseInvalidCommand   = 0x04;

// source/destination
constexpr auto SrcMask  = 0xF0;
constexpr auto SrcPos   = 4;
constexpr auto DestMask = 0x0F;

constexpr auto Tcp1     = 0x01;
constexpr auto Tcp2     = 0x02;
constexpr auto Tcp3     = 0x03;
constexpr auto EspPcb   = 0x04;
constexpr auto LedPcb   = 0x05;
// clang-format on

constexpr auto ProtocolMagic = 0xD7;

// packet header
struct PacketHeader
{
    uint8_t src_dest = 0;
    uint8_t command = 0;
    uint8_t status = 0;
    uint8_t magic = ProtocolMagic;
    uint16_t payloadSize = 0;
};

struct LedSegment
{
    uint8_t green = 0;
    uint8_t red = 0;
    uint8_t blue = 0;
    uint8_t white = 0;
};

struct SetLedSegment
{
    uint8_t position = 0;
    LedSegment segment;
};