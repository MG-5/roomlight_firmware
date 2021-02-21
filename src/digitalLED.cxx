#include "FreeRTOS.h"
#include "task.h"

#include "dma.h"
#include "gpio.h"
#include "tim.h"

#include "StatusLeds.hpp"
#include "digitalLED.hpp"
#include "protocol.hpp"

#include <climits>
#include <cmath>
#include <cstring>

LEDSegment ledCurrentData[Strip1Pixels + Strip2Pixels + Strip3Pixels]{};
LEDSegment ledTargetData[Strip1Pixels + Strip2Pixels + Strip3Pixels]{};

struct DiffLEDSegment
{
    int16_t green = 0;
    int16_t red = 0;
    int16_t blue = 0;
    int16_t white = 0;
};

DiffLEDSegment ledDiffData[Strip1Pixels + Strip2Pixels + Strip3Pixels];
extern TaskHandle_t zeroCheckerHandle;

bool stripEnabled[3];

Gpio mosfets[3] = {{EN_MOS1_GPIO_Port, EN_MOS1_Pin},
                   {EN_MOS2_GPIO_Port, EN_MOS2_Pin},
                   {EN_MOS3_GPIO_Port, EN_MOS3_Pin}};

StatusLed *mosfetLeds[3] = {ledGreen1, ledGreen2, ledGreen3};
LEDSegment zeroSegment;

// Bit band stuff
constexpr auto RAM_BASE = 0x20000000;
constexpr auto RAM_BB_BASE = 0x22000000;

#define Var_ResetBit_BB(VarAddr, BitNumber)                                                        \
    (*(volatile uint32_t *)(RAM_BB_BASE | ((VarAddr - RAM_BASE) << 5) | ((BitNumber) << 2)) = 0)

#define Var_SetBit_BB(VarAddr, BitNumber)                                                          \
    (*(volatile uint32_t *)(RAM_BB_BASE | ((VarAddr - RAM_BASE) << 5) | ((BitNumber) << 2)) = 1)

#define Var_GetBit_BB(VarAddr, BitNumber)                                                          \
    (*(volatile uint32_t *)(RAM_BB_BASE | ((VarAddr - RAM_BASE) << 5) | ((BitNumber) << 2)))

#define BITBAND_SRAM(address, bit)                                                                 \
    ((__IO uint32_t *)(RAM_BB_BASE + (((uint32_t)address) - RAM_BASE) * 32 + (bit)*4))

#define varSetBit(var, bit) (Var_SetBit_BB((uint32_t)&var, bit))
#define varResetBit(var, bit) (Var_ResetBit_BB((uint32_t)&var, bit))
#define varGetBit(var, bit) (Var_GetBit_BB((uint32_t)&var, bit))

struct digitalLEDBufferItem
{
    LEDSegment *frameBufferPointer;
    uint32_t frameBufferSize;
    uint32_t frameBufferCounter;
    uint8_t channel; // digital output pin
};

struct digitalLEDStruct
{
    digitalLEDBufferItem item[3];
    uint32_t repeatCounter;
};

digitalLEDStruct digitalLED;

// buffer for two LED segments
uint16_t DMABitBuffer[32 * 2];
constexpr auto BUFFER_SIZE = sizeof(DMABitBuffer) / sizeof(uint16_t);

// Define source arrays with output pins
constexpr uint32_t dLED_IOs[] = {DATA1_Pin | DATA2_Pin | DATA3_Pin};

void setPixelInBuffer(uint8_t channel, uint8_t bufferSegment, uint8_t red, uint8_t green,
                      uint8_t blue, uint8_t white)
{
    uint32_t calcRow = bufferSegment * 32;
    uint32_t invRed = ~red;
    uint32_t invGreen = ~green;
    uint32_t invBlue = ~blue;
    uint32_t invWhite = ~white;

    // Bitband optimizations with pure increments, 5us interrupts
    auto *bitBand = BITBAND_SRAM(&DMABitBuffer[(calcRow)], channel);

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

void loadNextFramebufferData(digitalLEDBufferItem *bItem, uint32_t row, bool reverse)
{
    uint32_t r = bItem->frameBufferPointer[bItem->frameBufferCounter].red;
    uint32_t g = bItem->frameBufferPointer[bItem->frameBufferCounter].green;
    uint32_t b = bItem->frameBufferPointer[bItem->frameBufferCounter].blue;
    uint32_t w = bItem->frameBufferPointer[bItem->frameBufferCounter].white;

    if (reverse)
    {
        if (bItem->frameBufferCounter == 0)
            bItem->frameBufferCounter = bItem->frameBufferSize - 1;

        else
            bItem->frameBufferCounter--;
    }
    else
    {
        if (bItem->frameBufferCounter == bItem->frameBufferSize - 1)
            bItem->frameBufferCounter = 0;

        else
            bItem->frameBufferCounter++;
    }

    setPixelInBuffer(bItem->channel, row, r, g, b, w);
}

void transferHalfHandler(DMA_HandleTypeDef *hdma)
{
    loadNextFramebufferData(&digitalLED.item[0], 0, true);
    loadNextFramebufferData(&digitalLED.item[1], 0, false);
    loadNextFramebufferData(&digitalLED.item[2], 0, false);
}

void transferCompleteHandler(DMA_HandleTypeDef *hdma)
{
    digitalLED.repeatCounter++;

    if (digitalLED.repeatCounter == MaximumPixels / 2)
    {
        digitalLED.repeatCounter = 0;

        // Transfer of all LEDs is done, disable timer
        HAL_TIM_Base_Stop(&htim1);

        // Manually set outputs to low to generate 50us reset impulse
        for (uint32_t i = 0; i < 30; ++i)
            asm volatile("nop");

        DATA1_GPIO_Port->BRR = *dLED_IOs;
    }
    else
    {
        // Load bitbuffer with next RGB LED values
        loadNextFramebufferData(&digitalLED.item[0], 1, true);
        loadNextFramebufferData(&digitalLED.item[1], 1, false);
        loadNextFramebufferData(&digitalLED.item[2], 1, false);
    }
}

void sendBuffer()
{
    // 47 Segments - reverse buffer
    digitalLED.item[0].frameBufferCounter = digitalLED.item[0].frameBufferSize - 1;
    loadNextFramebufferData(&digitalLED.item[0], 0, true);
    loadNextFramebufferData(&digitalLED.item[0], 1, true);

    // 38 segments
    digitalLED.item[1].frameBufferCounter = 0;
    loadNextFramebufferData(&digitalLED.item[1], 0, false);
    loadNextFramebufferData(&digitalLED.item[1], 1, false);

    // 46 Segments
    digitalLED.item[2].frameBufferCounter = 0;
    loadNextFramebufferData(&digitalLED.item[2], 0, false);
    loadNextFramebufferData(&digitalLED.item[2], 1, false);

    // start TIM2
    __HAL_TIM_SetCounter(&htim1, htim1.Init.Period - 1);

    // reset DMA counter
    HAL_DMA_Abort_IT(&hdma_tim1_ch1);
    HAL_DMA_Start_IT(&hdma_tim1_ch1, reinterpret_cast<uint32_t>(DMABitBuffer),
                     reinterpret_cast<uint32_t>(&DATA1_GPIO_Port->BRR), BUFFER_SIZE);

    HAL_TIM_Base_Start(&htim1);
}

bool isColorDataAvailable(uint8_t fromSegment, uint8_t toSegment)
{
    for (auto i = fromSegment; i <= toSegment; i++)
    {
        // compare segment with zero
        auto result = std::memcmp(&ledCurrentData[i], &zeroSegment, sizeof(LEDSegment));

        if (result != 0)
            return true;
    }
    return false;
}

void checkStripsForColor()
{
    if (stripEnabled[0])
        stripEnabled[0] = isColorDataAvailable(0, Strip1Pixels - 1);

    if (stripEnabled[1])
        stripEnabled[1] = isColorDataAvailable(Strip1Pixels, Strip1Pixels + Strip2Pixels - 1);

    if (stripEnabled[2])
        stripEnabled[2] = isColorDataAvailable(Strip1Pixels + Strip2Pixels,
                                               Strip1Pixels + Strip2Pixels + Strip3Pixels - 1);

    for (auto i = 0; i < 3; i++)
    {
        mosfets[i].write(stripEnabled[i]);
        mosfetLeds[i]->mode = stripEnabled[i] ? StatusLedMode::On : StatusLedMode::Off;
    }
}

void initDigitalLED()
{
    digitalLED.item[0].channel = std::log2(DATA1_Pin);
    digitalLED.item[0].frameBufferPointer = ledCurrentData;
    digitalLED.item[0].frameBufferSize = Strip1Pixels;

    digitalLED.item[1].channel = std::log2(DATA2_Pin);
    digitalLED.item[1].frameBufferPointer = ledCurrentData + Strip1Pixels;
    digitalLED.item[1].frameBufferSize = Strip2Pixels;

    digitalLED.item[2].channel = std::log2(DATA3_Pin);
    digitalLED.item[2].frameBufferPointer = ledCurrentData + Strip1Pixels + Strip2Pixels;
    digitalLED.item[2].frameBufferSize = Strip3Pixels;

    HAL_DMA_RegisterCallback(&hdma_tim1_ch1, HAL_DMA_XFER_HALFCPLT_CB_ID, transferHalfHandler);
    HAL_DMA_RegisterCallback(&hdma_tim1_ch1, HAL_DMA_XFER_CPLT_CB_ID, transferCompleteHandler);

    HAL_DMA_Start(&hdma_tim1_up, reinterpret_cast<uint32_t>(dLED_IOs),
                  reinterpret_cast<uint32_t>(&DATA1_GPIO_Port->BSRR), 1);

    HAL_DMA_Start(&hdma_tim1_ch2, reinterpret_cast<uint32_t>(dLED_IOs),
                  reinterpret_cast<uint32_t>(&DATA1_GPIO_Port->BRR), 1);

    __HAL_TIM_ENABLE_DMA(&htim1, TIM_DMA_UPDATE);
    __HAL_TIM_ENABLE_DMA(&htim1, TIM_DMA_CC1);
    __HAL_TIM_ENABLE_DMA(&htim1, TIM_DMA_CC2);
}

extern "C" void digitalLEDTask(void *)
{
    initDigitalLED();

    uint8_t seg = 85;
    ledCurrentData[seg++].red = 1;
    ledCurrentData[seg++].green = 1;
    ledCurrentData[seg++].blue = 1;
    ledCurrentData[seg++].white = 1;
    ledCurrentData[seg++].red = 1;
    ledCurrentData[seg++].green = 1;
    ledCurrentData[seg++].blue = 1;
    ledCurrentData[seg++].white = 1;

    HAL_GPIO_WritePin(EN_MOS3_GPIO_Port, EN_MOS3_Pin, GPIO_PIN_SET);
    ledGreen3->mode = StatusLedMode::On;
    stripEnabled[2] = true;

    // let led chips some time to start
    for (int i = 0; i < 750; ++i)
        asm volatile("nop");

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
            stripEnabled[0] = stripEnabled[1] = stripEnabled[2] = true;
            checkStripsForColor();

            // let led chips some time to start
            for (int i = 0; i < 750; ++i)
                asm volatile("nop");
        }
    }
}

extern "C" void ledFadingTask(void *)
{
    uint8_t factor;
    bool restart = false;

    while (true)
    {
        if (!restart)
            xTaskNotifyWait(0, ULONG_MAX, nullptr, portMAX_DELAY);

        restart = false;
        factor = 100;

        // calc difference between current and target data
        for (uint32_t i = 0; i < Strip1Pixels + Strip2Pixels + Strip3Pixels; i++)
        {
            ledDiffData[i].green = ledCurrentData[i].green - ledTargetData[i].green;
            ledDiffData[i].red = ledCurrentData[i].red - ledTargetData[i].red;
            ledDiffData[i].blue = ledCurrentData[i].blue - ledTargetData[i].blue;
            ledDiffData[i].white = ledCurrentData[i].white - ledTargetData[i].white;
        }

        while (true)
        {
            // apply difference multiplied by factor to current data
            for (uint32_t i = 0; i < Strip1Pixels + Strip2Pixels + Strip3Pixels; i++)
            {
                ledCurrentData[i].green = static_cast<uint8_t>(
                    ledTargetData[i].green + (factor * ledDiffData[i].green) / 100); // green

                ledCurrentData[i].red = static_cast<uint8_t>(
                    ledTargetData[i].red + (factor * ledDiffData[i].red) / 100); // red

                ledCurrentData[i].blue = static_cast<uint8_t>(
                    ledTargetData[i].blue + (factor * ledDiffData[i].blue) / 100); // blue

                ledCurrentData[i].white = static_cast<uint8_t>(
                    ledTargetData[i].white + (factor * ledDiffData[i].white) / 100); // white
            }

            // trigger task to render led data
            xTaskNotify(digitalLEDHandle, 1, eSetBits);

            if (factor == 0)
                break;

            factor -= 2;

            uint32_t notifiedValue;
            xTaskNotifyWait(0, ULONG_MAX, &notifiedValue, pdMS_TO_TICKS(8));
            if ((notifiedValue & 0x01U) != 0)
            {
                // restart fading
                restart = true;
                break;
            }
        }

        // trigger mosfet zero check task if needed
        if (!restart)
            xTaskNotify(zeroCheckerHandle, 1, eSetBits);
    }
}

extern "C" void zeroCheckerTask(void *)
{
    while (true)
    {
        uint32_t notifiedValue;
        xTaskNotifyWait(0, ULONG_MAX, &notifiedValue, pdMS_TO_TICKS(5000));

        checkStripsForColor();
    }
}