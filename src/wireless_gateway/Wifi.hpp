#pragma once

#include "leds/ledConstants.hpp"
#include "protocol.hpp"
#include "util/gpio.hpp"
#include "wrappers/EventGroup.hpp"
#include "wrappers/StreamBuffer.hpp"
#include "wrappers/Task.hpp"

#include "main.h"

class Wifi : public util::wrappers::TaskWithMemberFunctionBase
{
public:
    static constexpr auto EventConnection = 1 << 0;

    static constexpr auto EventWifiUpgradeStarted = 1 << 1;
    static constexpr auto EventWifiUpgradeFinished = 1 << 2;

    enum class Mode
    {
        Disabled,
        Programming,
        Normal
    };

    enum class Result
    {
        Success = 0,
        Failure,
        Timeout,
        TooShort,
        InvalidResponse
    };

    util::wrappers::EventGroup wifiEvents{};

    Wifi(util::wrappers::StreamBuffer &txMessageBuffer)
        : TaskWithMemberFunctionBase("wifiDaemonTask", 128, osPriorityLow), //
          txMessageBuffer(txMessageBuffer){};

    Mode getMode();

    /// Set the mode of the Wifi module.
    /// Requires restarting the module if mode differs from the mode the
    /// module is currently in unless forceRestart is set to true.
    void setMode(Wifi::Mode mode, bool forceRestart = false);

    bool checkConnection();
    void sendResponsePacket(PacketHeader *header, const uint8_t *payload);

protected:
    [[noreturn]] void taskMain(void *) override;

private:
    util::wrappers::StreamBuffer &txMessageBuffer;

    Mode mode = Wifi::Mode::Disabled;

    // TX
    static constexpr auto TxPacketBufferSize = 64;
    uint8_t finalFrame[TxPacketBufferSize];

    util::Gpio espGpio0{ESP_GPIO0_GPIO_Port, ESP_GPIO0_Pin};
    util::Gpio espEnable{ESP_EN_GPIO_Port, ESP_EN_Pin};
    util::Gpio espReset{ESP_RST_GPIO_Port, ESP_RST_Pin};

    void swapSrcDest(uint8_t &val);
    void initWifi();

    void sendPacket(const PacketHeader *header, const uint8_t *payload);
};