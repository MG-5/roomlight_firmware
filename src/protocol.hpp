#pragma once

#include <stdint.h>

// clang-format off

// packet commands
#define ESP_CONNECTION_TEST         0x01
#define ESP_GET_MAC_ADDR            0x02
#define ESP_RESTART                 0x03
// 0x04 - 0x0F reserved for ESP

#define LED_SET_ALL                 0x10
#define LED_SET_SEGMENTS            0x11
#define LED_FADE_SOFT               0x12
#define LED_FADE_HARD               0x13
#define LED_GET_CURRENT             0x14
#define LED_GET_VOLTAGE             0x15
#define LED_GET_ERROR_CODE          0x16
#define LED_ESP_OTA_STARTED         0x17
#define LED_ESP_OTA_FINISHED        0x18
#define LED_SET_STATUS_LED          0x19
#define LED_DEBUG_PRINT             0x1A

// packet status
#define RESPONSE_MASK               0x80
#define RESPONSE_OKAY               0x00
#define RESPONSE_OPERATION_FAILED   0x01
#define RESPONSE_CRC_FAILED         0x02
#define RESPONSE_NOT_SUPPORTED      0x03
#define RESPONSE_INVALID_COMMAND    0x04

// source/destination
#define SRC_MASK                    0xF0
#define SRC_POS                     4
#define DEST_MASK                   0x0F

#define TCP1                        0x01
#define TCP2                        0x02
#define TCP3                        0x03
#define ESP_PCB                     0x04
#define LED_PCB                     0x05
// clang-format on

constexpr auto PROTOCOL_MAGIC = 0xD7;

// packet header
struct PacketHeader
{
    uint8_t src_dest = 0;
    uint8_t command = 0;
    uint8_t status = 0;
    uint8_t magic = PROTOCOL_MAGIC;
    uint16_t payloadSize = 0;
};

struct LEDSegment
{
    uint8_t green = 0;
    uint8_t red = 0;
    uint8_t blue = 0;
    uint8_t white = 0;
};

struct SetLedSegment
{
    uint8_t position = 0;
    LEDSegment segment;
};