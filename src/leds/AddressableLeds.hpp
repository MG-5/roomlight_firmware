#pragma once

#include "core/SafeAssert.h"
#include "ledConstants.hpp"
#include "protocol.hpp"
#include "util/gpio.hpp"
#include "wrappers/Task.hpp"

#include "dma.h"
#include "tim.h"

#include <algorithm>
#include <array>

class AddressableLeds : public util::wrappers::TaskWithMemberFunctionBase
{
public:
    static constexpr auto LedTimer = &htim1;
    static constexpr auto DmaTimerUpdate = &hdma_tim1_up;
    static constexpr auto DmaTimerChannel2 = &hdma_tim1_ch2;

    explicit AddressableLeds(DMA_HandleTypeDef *dmaLedTimerChannel1)
        : TaskWithMemberFunctionBase("addrLedsTask", 128, osPriorityHigh), //
          dmaLedTimerChannel1(dmaLedTimerChannel1)
    {
        SafeAssert(dmaLedTimerChannel1 != nullptr);
    }

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

    void setWarmWhite(bool value)
    {
        warmWhite = value;
    }

    [[nodiscard]] bool isWarmWhite() const
    {
        return warmWhite;
    }

    void setLightState(LightState newState)
    {
        currentLightState = newState;
    }

    LightState getLightState() const
    {
        return currentLightState;
    }

    void setLightMode(LightMode newMode)
    {
        currentLightMode = newMode;
    }

    LightMode getLightMode() const
    {
        return currentLightMode;
    }

    LedArray &getCurrentLedArray()
    {
        return ledCurrentData;
    }

    void transferHalfHandler();
    void transferCompleteHandler();

    std::array<bool, NumberOfDataPins> stripEnabled{};

protected:
    [[noreturn]] void taskMain(void *) override;

private:
    DMA_HandleTypeDef *dmaLedTimerChannel1 = nullptr;

    LedArray ledCurrentData{};
    LightState currentLightState = LightState::Off;
    LightMode currentLightMode = LightMode::BothStrips;
    LightState prevLightState = currentLightState;
    LightMode prevLightMode = currentLightMode;

    bool warmWhite = true;

    util::Gpio mosfets[NumberOfDataPins] = {{EN_MOS1_GPIO_Port, EN_MOS1_Pin},
                                            {EN_MOS2_GPIO_Port, EN_MOS2_Pin}};

    LedSegment zeroSegment{};

    struct LedBufferItem
    {
        LedSegment *frameBufferPointer = nullptr;
        uint32_t frameBufferSize = 0;
        uint32_t frameBufferCounter = 0;
        uint8_t channel = 0; // digital output pin
    };

    struct LedStruct
    {
        LedBufferItem item[NumberOfDataPins];
        uint32_t repeatCounter = 0;
    };

    LedStruct ledStruct;

    // buffer for two LED segments
    static constexpr auto DMABitBufferSize = (sizeof(LedSegment) * 8) * 2;
    uint16_t DMABitBuffer[DMABitBufferSize]{};

    // Define source arrays with output pins
    static constexpr uint32_t dLED_IOs[] = {DATA1_Pin | DATA2_Pin};

    void setPixelInBuffer(uint8_t channel, uint8_t bufferSegment, uint8_t red, uint8_t green,
                          uint8_t blue, uint8_t white);
    void loadNextFramebufferData(LedBufferItem &bufferItem, uint32_t row, bool reverse);
    void sendBuffer();
    bool isColorDataAvailable(uint8_t fromSegment, uint8_t toSegment);
    void checkStripsForColor();
    void initDigitalLED();
    void showTestColors();
};