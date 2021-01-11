#include "FreeRTOS.h"
#include "task.h"

#include "dma.h"
#include "gpio.h"
#include "tim.h"

#include "StatusLeds.hpp"
#include "defines.hpp"
#include "protocol.hpp"

#include <cmath>

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

    if (digitalLED.repeatCounter == MAX_PIXELS / 2)
    {
        digitalLED.repeatCounter = 0;

        // Transfer of all LEDs is done, disable timer
        HAL_TIM_Base_Stop(&htim1);
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

void initDigitalLED()
{
    digitalLED.item[0].channel = std::log2(DATA1_Pin);
    digitalLED.item[0].frameBufferPointer = ledCurrentData;
    digitalLED.item[0].frameBufferSize = PIXELS1;

    digitalLED.item[1].channel = std::log2(DATA2_Pin);
    digitalLED.item[1].frameBufferPointer = ledCurrentData + PIXELS1;
    digitalLED.item[1].frameBufferSize = PIXELS2;

    digitalLED.item[2].channel = std::log2(DATA3_Pin);
    digitalLED.item[2].frameBufferPointer = ledCurrentData + PIXELS1 + PIXELS2;
    digitalLED.item[2].frameBufferSize = PIXELS3;

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

    ledCurrentData[86].red = 1;

    HAL_GPIO_WritePin(EN_MOS1_GPIO_Port, EN_MOS1_Pin, GPIO_PIN_SET);
    ledGreen1->mode = StatusLedMode::On;

    // let led chips some time to start
    for (int i = 0; i < 750; ++i)
        asm volatile("nop");

    auto lastWakeTime = xTaskGetTickCount();

    while (1)
    {
        sendBuffer();
        vTaskDelayUntil(&lastWakeTime, pdMS_TO_TICKS(500));
    }
}