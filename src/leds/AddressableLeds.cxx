#include "FreeRTOS.h"
#include "task.h"

#include "dma.h"
#include "gpio.h"
#include "tim.h"

#include "AddressableLeds.hpp"
#include "BitBanding.hpp"
#include "StatusLeds.hpp"
#include "gamma/GammaLUT.hpp"
#include "helpers/freertos.hpp"
#include "protocol.hpp"
#include "units/si/frequency.hpp"

#include <climits>
#include <cmath>
#include <cstring>

void AddressableLeds::setPixelInBuffer(uint8_t channel, uint8_t bufferSegment, uint8_t red,
                                       uint8_t green, uint8_t blue, uint8_t white)
{
    uint32_t calcRow = bufferSegment * 32;
    uint32_t invRed = ~red;
    uint32_t invGreen = ~green;
    uint32_t invBlue = ~blue;
    uint32_t invWhite = ~white;

    // Bitband optimizations with pure increments
    const auto dataAddress = reinterpret_cast<uint32_t>(&DMABitBuffer[(calcRow)]);
    auto *bitBand = getBitBandingPointer(dataAddress, channel);

    // RED
    *bitBand = (invRed >> 7);
    bitBand += 16;

    *bitBand = (invRed >> 6);
    bitBand += 16;

    *bitBand = (invRed >> 5);
    bitBand += 16;

    *bitBand = (invRed >> 4);
    bitBand += 16;

    *bitBand = (invRed >> 3);
    bitBand += 16;

    *bitBand = (invRed >> 2);
    bitBand += 16;

    *bitBand = (invRed >> 1);
    bitBand += 16;

    *bitBand = (invRed >> 0);
    bitBand += 16;

    // GREEN
    *bitBand = (invGreen >> 7);
    bitBand += 16;

    *bitBand = (invGreen >> 6);
    bitBand += 16;

    *bitBand = (invGreen >> 5);
    bitBand += 16;

    *bitBand = (invGreen >> 4);
    bitBand += 16;

    *bitBand = (invGreen >> 3);
    bitBand += 16;

    *bitBand = (invGreen >> 2);
    bitBand += 16;

    *bitBand = (invGreen >> 1);
    bitBand += 16;

    *bitBand = (invGreen >> 0);
    bitBand += 16;

    // BLUE
    *bitBand = (invBlue >> 7);
    bitBand += 16;

    *bitBand = (invBlue >> 6);
    bitBand += 16;

    *bitBand = (invBlue >> 5);
    bitBand += 16;

    *bitBand = (invBlue >> 4);
    bitBand += 16;

    *bitBand = (invBlue >> 3);
    bitBand += 16;

    *bitBand = (invBlue >> 2);
    bitBand += 16;

    *bitBand = (invBlue >> 1);
    bitBand += 16;

    *bitBand = (invBlue >> 0);
    bitBand += 16;

    // WHITE
    *bitBand = (invWhite >> 7);
    bitBand += 16;

    *bitBand = (invWhite >> 6);
    bitBand += 16;

    *bitBand = (invWhite >> 5);
    bitBand += 16;

    *bitBand = (invWhite >> 4);
    bitBand += 16;

    *bitBand = (invWhite >> 3);
    bitBand += 16;

    *bitBand = (invWhite >> 2);
    bitBand += 16;

    *bitBand = (invWhite >> 1);
    bitBand += 16;

    *bitBand = (invWhite >> 0);
    bitBand += 16;
}

void AddressableLeds::loadNextFramebufferData(LedBufferItem &bufferItem, uint32_t row, bool reverse)
{
    uint32_t red = GammaLUT[bufferItem.frameBufferPointer[bufferItem.frameBufferCounter].red];
    uint32_t green = GammaLUT[bufferItem.frameBufferPointer[bufferItem.frameBufferCounter].green];
    uint32_t blue = GammaLUT[bufferItem.frameBufferPointer[bufferItem.frameBufferCounter].blue];
    uint32_t white = GammaLUT[bufferItem.frameBufferPointer[bufferItem.frameBufferCounter].white];

    if (reverse)
    {
        if (bufferItem.frameBufferCounter == 0)
            bufferItem.frameBufferCounter = bufferItem.frameBufferSize - 1;

        else
            bufferItem.frameBufferCounter--;
    }
    else
    {
        if (bufferItem.frameBufferCounter == bufferItem.frameBufferSize - 1)
            bufferItem.frameBufferCounter = 0;

        else
            bufferItem.frameBufferCounter++;
    }

    setPixelInBuffer(bufferItem.channel, row, red, green, blue, white);
}

void AddressableLeds::transferHalfHandler()
{
    loadNextFramebufferData(ledStruct.item[0], 0, true);
    loadNextFramebufferData(ledStruct.item[1], 0, false);
}

void AddressableLeds::transferCompleteHandler()
{
    ledStruct.repeatCounter++;

    constexpr auto MaxLeds = []() -> auto
    {
        if constexpr (LongestStrip % 2 == 0)
            return LongestStrip / 2;

        else
            return (LongestStrip / 2) + 1;
    }
    ();

    if (ledStruct.repeatCounter == MaxLeds)
    {
        ledStruct.repeatCounter = 0;

        // Transfer of all LEDs is done, disable timer
        HAL_TIM_Base_Stop(LedTimer);

        // Manually set outputs to low to generate 50us reset impulse
        for (uint32_t i = 0; i < 30; ++i)
            asm volatile("nop");

        DATA1_GPIO_Port->BRR = *dLED_IOs;
    }
    else
    {
        // Load bitbuffer with next RGB LED values
        loadNextFramebufferData(ledStruct.item[0], 1, true);
        loadNextFramebufferData(ledStruct.item[1], 1, false);
    }
}

void AddressableLeds::sendBuffer()
{
    // 47 Segments - reverse buffer
    ledStruct.item[0].frameBufferCounter = ledStruct.item[0].frameBufferSize - 1;
    loadNextFramebufferData(ledStruct.item[0], 0, true);
    loadNextFramebufferData(ledStruct.item[0], 1, true);

    // 37 + 46 segments
    ledStruct.item[1].frameBufferCounter = 0;
    loadNextFramebufferData(ledStruct.item[1], 0, false);
    loadNextFramebufferData(ledStruct.item[1], 1, false);

    // start TIM2
    __HAL_TIM_SetCounter(LedTimer, htim1.Init.Period - 1);

    // reset DMA counter
    HAL_DMA_Abort_IT(dmaLedTimerChannel1);
    HAL_DMA_Start_IT(dmaLedTimerChannel1, reinterpret_cast<uint32_t>(DMABitBuffer),
                     reinterpret_cast<uint32_t>(&DATA1_GPIO_Port->BRR), DMABitBufferSize);

    HAL_TIM_Base_Start(LedTimer);
}

bool AddressableLeds::isColorDataAvailable(uint8_t fromSegment, uint8_t toSegment)
{
    for (auto i = fromSegment; i <= toSegment; i++)
    {
        // compare segment with zero
        auto result = std::memcmp(&ledCurrentData[i], &zeroSegment, sizeof(LedSegment));

        if (result != 0)
            return true;
    }
    return false;
}

void AddressableLeds::checkStripsForColor()
{
    stripEnabled[0] = isColorDataAvailable(0, Strip1Pixels - 1);

    if (currentLightMode == LightMode::BothStrips || currentLightState == LightState::Custom)
        stripEnabled[1] =
            isColorDataAvailable(Strip1Pixels, Strip1Pixels + Strip2Pixels + Strip3Pixels - 1);
    else
        stripEnabled[1] = false;

    for (auto i = 0; i < NumberOfDataPins; i++)
        mosfets[i].write(stripEnabled[i]);

    if (!stripEnabled[0] && !stripEnabled[1])
        currentLightState = LightState::Off;

    else if (prevLightState == LightState::Off ||
             (currentLightMode == LightMode::BothStrips && prevLightMode == LightMode::OnlyStrip1))
    {
        // let LED chips some time to start
        for (int i = 0; i < 750; ++i)
            asm volatile("nop");
    }

    prevLightState = currentLightState;
    prevLightMode = currentLightMode;
}

void AddressableLeds::initDigitalLED()
{
    ledStruct.item[0].channel = std::log2(DATA1_Pin);
    ledStruct.item[0].frameBufferPointer = ledCurrentData.data();
    ledStruct.item[0].frameBufferSize = Strip1Pixels;

    ledStruct.item[1].channel = std::log2(DATA2_Pin);
    ledStruct.item[1].frameBufferPointer = ledCurrentData.data() + Strip1Pixels;
    ledStruct.item[1].frameBufferSize = Strip2Pixels + Strip3Pixels;

    HAL_DMA_Start(DmaTimerUpdate, reinterpret_cast<uint32_t>(dLED_IOs),
                  reinterpret_cast<uint32_t>(&DATA1_GPIO_Port->BSRR), 1);

    HAL_DMA_Start(DmaTimerChannel2, reinterpret_cast<uint32_t>(dLED_IOs),
                  reinterpret_cast<uint32_t>(&DATA1_GPIO_Port->BRR), 1);

    __HAL_TIM_ENABLE_DMA(LedTimer, TIM_DMA_UPDATE);
    __HAL_TIM_ENABLE_DMA(LedTimer, TIM_DMA_CC1);
    __HAL_TIM_ENABLE_DMA(LedTimer, TIM_DMA_CC2);
}

void AddressableLeds::showTestColors()
{
    ledCurrentData[0].red = 255;
    ledCurrentData[Strip1Pixels - 1].green = 255;
    ledCurrentData[Strip1Pixels].blue = 255;
    ledCurrentData[Strip1Pixels + Strip2Pixels - 1].white = 255;
    ledCurrentData[Strip1Pixels + Strip2Pixels].red = 255;
    ledCurrentData[TotalPixels - 1].green = 255;

    currentLightState = LightState::Custom;

    checkStripsForColor();
}

void AddressableLeds::taskMain(void *)
{
    initDigitalLED();
    showTestColors();

    auto lastWakeTime = xTaskGetTickCount();

    while (true)
    {
        sendBuffer();

        // wait for 500 ms unless a signal is occured
        uint32_t notifiedValue;
        xTaskNotifyWait(0, ULONG_MAX, &notifiedValue, pdMS_TO_TICKS(500));
        if ((notifiedValue & 0x01U) != 0)
        {
            // turn on mosfet if needed
            checkStripsForColor();
        }
    }
}