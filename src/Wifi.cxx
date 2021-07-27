#include "FreeRTOS.h"
#include "event_groups.h"
#include "message_buffer.h"
#include "queue.h"
#include "task.h"

#include <climits>
#include <cstring>

#include "StatusLeds.hpp"
#include "Wifi.hpp"
#include "digitalLED.hpp"
#include "uart.hpp"

#include "units/si/current.hpp"
#include "units/si/voltage.hpp"

extern TaskHandle_t wifiDaemonHandle;
extern units::si::Voltage ledVoltage;
extern units::si::Current ledCurrent;

namespace
{
EventGroupHandle_t wifiEvents = xEventGroupCreate();
constexpr auto EventConnection = 1 << 0;
constexpr auto EventWifiUpgradeStarted = 1 << 1;
constexpr auto EventWifiUpgradeFinished = 1 << 2;

Wifi::Mode mode_ = Wifi::Mode::Disabled;

// TX
constexpr auto TxPacketBufferSize = 64;
uint8_t finalFrame[TxPacketBufferSize];

// RX
size_t bufferPosition = 0;
constexpr auto RxPacketBufferSize = 512 + 32;
uint8_t dataBuffer[RxPacketBufferSize];
uint8_t packetFrame[RxPacketBufferSize];
auto packetBuffer = xMessageBufferCreate(1024 + 512);

util::Gpio espGpio0{ESP_GPIO0_GPIO_Port, ESP_GPIO0_Pin};
util::Gpio espEnable{ESP_EN_GPIO_Port, ESP_EN_Pin};
util::Gpio espReset{ESP_RST_GPIO_Port, ESP_RST_Pin};

} // namespace

namespace Wifi
{

Wifi::Mode getMode()
{
    return mode_;
}

// -------------------------------------------------------------------------------------------------
void setMode(Wifi::Mode mode, bool forceRestart)
{
    if (mode == mode_ && !forceRestart)
        return;

    espEnable.write(util::Gpio::Low);
    espReset.write(util::Gpio::Low);

    xMessageBufferReset(packetBuffer);

    if (mode == Mode::Programming)
        espGpio0.write(util::Gpio::Low);

    else if (mode == Mode::Normal)
        espGpio0.write(util::Gpio::High);

    vTaskDelay(pdMS_TO_TICKS(10));

    espEnable.write(util::Gpio::High);
    espReset.write(util::Gpio::High);

    mode_ = mode;
}

// -------------------------------------------------------------------------------------------------
void sendPacket(const PacketHeader *header, const uint8_t *payload)
{
    std::memcpy(finalFrame, header, sizeof(PacketHeader));
    std::memcpy(finalFrame + sizeof(PacketHeader), payload, header->payloadSize);

    uartSendData(finalFrame, sizeof(PacketHeader) + header->payloadSize);
}

// -------------------------------------------------------------------------------------------------
void swapSrcDest(uint8_t &val)
{
    uint8_t tmp = val;
    val <<= SRC_POS;
    val |= (tmp >> SRC_POS) & DEST_MASK;
}

// -------------------------------------------------------------------------------------------------
void sendResponsePacket(PacketHeader *const header, const uint8_t *payload)
{
    header->command |= RESPONSE_MASK;

    if (!payload)
        header->payloadSize = 0;

    swapSrcDest(header->src_dest);
    sendPacket(header, payload);
}

// -------------------------------------------------------------------------------------------------
bool checkConnection()
{
    PacketHeader h;
    h.src_dest = (LED_PCB << SRC_POS) | ESP_PCB;
    h.command = ESP_CONNECTION_TEST;
    h.payloadSize = 0;

    xEventGroupClearBits(wifiEvents, EventConnection);

    sendPacket(&h, nullptr);

    EventBits_t uxBits =
        xEventGroupWaitBits(wifiEvents, EventConnection, pdTRUE, pdTRUE, pdMS_TO_TICKS(10));

    return (uxBits & EventConnection);
}

// -------------------------------------------------------------------------------------------------
void receiveData(const uint8_t *data, uint32_t length)
{
    if (bufferPosition + length > RxPacketBufferSize)
    {
        // Remaining data is too large, so we need to clear the entire buffer
        bufferPosition = 0;
        return;
    }

    std::memcpy(dataBuffer + bufferPosition, data, length);
    bufferPosition += length;

    if (bufferPosition >= sizeof(PacketHeader))
    {
        PacketHeader header;
        std::memcpy(&header, dataBuffer, sizeof(header));
        if (header.magic == PROTOCOL_MAGIC)
        {
            auto messageLength = sizeof(header) + header.payloadSize;
            if (messageLength == bufferPosition)
            {
                xMessageBufferSend(packetBuffer, dataBuffer, messageLength, 0);
                bufferPosition = 0;
            }
            else if (messageLength < bufferPosition)
            {
                // more than one packet available - split it
                xMessageBufferSend(packetBuffer, dataBuffer, messageLength, 0);
                bufferPosition -= messageLength;
                std::memcpy(dataBuffer, dataBuffer + messageLength, bufferPosition);
            }

            xTaskNotify(wifiDaemonHandle, 1, eSetBits); // trigger wifi alive event
        }
        else
        {
            // invalid packet - reset buffer
            bufferPosition = 0;
        }
    }
}

// -------------------------------------------------------------------------------------------------
void initWifi()
{
    setMode(Wifi::Mode::Normal, true);
}

} // namespace Wifi

// -------------------------------------------------------------------------------------------------
extern "C" void processPacketsTask(void *)
{
    vTaskSuspend(nullptr);
    uint8_t *payload = nullptr;

    while (true)
    {
        payload = nullptr;
        auto length =
            xMessageBufferReceive(packetBuffer, packetFrame, RxPacketBufferSize, portMAX_DELAY);

        if (length == 0)
        {
            xMessageBufferReset(packetBuffer);
            continue;
        }

        PacketHeader header;
        std::memcpy(&header, packetFrame, sizeof(PacketHeader));

        if ((header.src_dest & DEST_MASK) != LED_PCB)
            continue; // packet is not relevant to us

        if (header.payloadSize > 0)
            payload = packetFrame + sizeof(PacketHeader);

        if (header.command & RESPONSE_MASK)
        {
            header.command &= ~RESPONSE_MASK; // remove bit for further processing
            switch (header.command)
            {
            case ESP_CONNECTION_TEST:
                if (header.status == RESPONSE_OKAY)
                    xEventGroupSetBits(wifiEvents, EventConnection);
                break;

            case ESP_GET_MAC_ADDR:
            default:
                // do nothing with unsupported repsonses
                break;
            }
            continue;
        }

        switch (header.command)
        {
        case LED_SET_ALL:
            if (header.payloadSize == 4 * TotalPixels)
            {
                std::memcpy(reinterpret_cast<uint8_t *>(ledTargetData), payload,
                            header.payloadSize);
                header.status = RESPONSE_OKAY;
            }
            else
                header.status = RESPONSE_OPERATION_FAILED;
            break;

        case LED_SET_SEGMENTS:
            if (header.payloadSize == sizeof(SetLedSegment))
            {
                auto *segmentToSet = reinterpret_cast<SetLedSegment *>(payload);
                ledTargetData[segmentToSet->position] = segmentToSet->segment;
                header.status = RESPONSE_OKAY;
            }
            else
                header.status = RESPONSE_OPERATION_FAILED;
            break;

        case LED_FADE_SOFT:
            header.status = RESPONSE_OKAY;
            currentLightState = LightState::Custom;
            xTaskNotify(ledFadingHandle, 1, eSetBits);
            break;

        case LED_FADE_HARD:
            std::memcpy(ledCurrentData, ledTargetData, TotalPixels * sizeof(uint32_t));
            header.status = RESPONSE_OKAY;
            currentLightState = LightState::Custom;
            xTaskNotify(digitalLEDHandle, 1, eSetBits);
            break;

        case LED_GET_CURRENT:
            header.status = RESPONSE_OKAY;
            header.payloadSize = 2;
            Wifi::sendResponsePacket(&header, reinterpret_cast<uint8_t *>(&ledCurrent));
            continue;
            break;

        case LED_GET_VOLTAGE:
            header.status = RESPONSE_OKAY;
            header.payloadSize = 2;
            Wifi::sendResponsePacket(&header, reinterpret_cast<uint8_t *>(&ledVoltage));
            continue;
            break;

        case LED_GET_ERROR_CODE:
            header.status = RESPONSE_NOT_SUPPORTED;
            break;

        case LED_ESP_OTA_STARTED:
            xEventGroupSetBits(wifiEvents, EventWifiUpgradeStarted);
            // do not disturb ESP
            continue;
            break;

        case LED_ESP_OTA_FINISHED:
            xEventGroupSetBits(wifiEvents, EventWifiUpgradeFinished);
            continue;
            break;

        case LED_SET_STATUS_LED:
            header.status = RESPONSE_NOT_SUPPORTED;
            break;

        default:
            header.status = RESPONSE_INVALID_COMMAND;
            break;
        }
        Wifi::sendResponsePacket(&header, nullptr);
    }
}

// -------------------------------------------------------------------------------------------------
extern "C" void wifiDaemonTask(void *)
{
    vTaskSuspend(nullptr);
    Wifi::initWifi();
    bool result = false;

    while (true)
    {
        ledRed->mode = result ? StatusLedMode::Off : StatusLedMode::Blink1Hz;

        uint32_t notifiedValue;
        xTaskNotifyWait(0, ULONG_MAX, &notifiedValue, pdMS_TO_TICKS(2000));
        if ((notifiedValue & 1) != 0)
            result = true;

        else
            result = Wifi::checkConnection();
    }
}