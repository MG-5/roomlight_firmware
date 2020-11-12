#include "FreeRTOS.h"
#include "StatusLeds.hpp"
#include "dma.h"
#include "gpio.h"
#include "protocol.hpp"
#include "task.h"
#include "tim.h"

auto constexpr PIXELS1 = 47;
auto constexpr PIXELS2 = 84;

// Bit band stuff
auto constexpr RAM_BASE = 0x20000000;
auto constexpr RAM_BB_BASE = 0x22000000;

#define Var_ResetBit_BB(VarAddr, BitNumber)                                                        \
    (*(volatile uint32_t *)(RAM_BB_BASE | ((VarAddr - RAM_BASE) << 5) | ((BitNumber) << 2)) = 0)

#define Var_SetBit_BB(VarAddr, BitNumber)                                                          \
    (*(volatile uint32_t *)(RAM_BB_BASE | ((VarAddr - RAM_BASE) << 5) | ((BitNumber) << 2)) = 1)

#define Var_GetBit_BB(VarAddr, BitNumber)                                                          \
    (*(volatile uint32_t *)(RAM_BB_BASE | ((VarAddr - RAM_BASE) << 5) | ((BitNumber) << 2)))

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
    digitalLEDBufferItem item[2];
    uint8_t transferComplete;
    uint8_t startTranfer;
    uint32_t timerPeriodCounter;
    uint32_t repeatCounter;
};

digitalLEDStruct digitalLED;

// WS2812 framebuffer - buffer for 2 LEDs
uint16_t DMABitBuffer[32 * 2];
auto constexpr BUFFER_SIZE = sizeof(DMABitBuffer) / sizeof(uint16_t);

// Define source arrays with output pins
uint32_t constexpr dLED_IOs[] = {DATA1_Pin, DATA2_Pin, DATA3_Pin};

void setPixelInBuffer(uint8_t column, uint16_t row, uint8_t red, uint8_t green, uint8_t blue,
                      uint8_t white)
{
    uint32_t calcRow = (row * 32);
    uint32_t invRed = ~red;
    uint32_t invGreen = ~green;
    uint32_t invBlue = ~blue;
    uint32_t invWhite = ~white;

    for (uint8_t i = 0; i < 8; i++)
    {
        // Set or clear the data for the pixel
        if (((invRed) << i) & 0x80)
            varSetBit(DMABitBuffer[(calcRow + i)], column);
        else
            varResetBit(DMABitBuffer[(calcRow + i)], column);

        if (((invGreen) << i) & 0x80)
            varSetBit(DMABitBuffer[(calcRow + 8 + i)], column);
        else
            varResetBit(DMABitBuffer[(calcRow + 8 + i)], column);

        if (((invBlue) << i) & 0x80)
            varSetBit(DMABitBuffer[(calcRow + 16 + i)], column);
        else
            varResetBit(DMABitBuffer[(calcRow + 16 + i)], column);

        if (((invWhite) << i) & 0x80)
            varSetBit(DMABitBuffer[(calcRow + 24 + i)], column);
        else
            varResetBit(DMABitBuffer[(calcRow + 24 + i)], column);
    }
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
}

void transferCompleteHandler(DMA_HandleTypeDef *hdma)
{
    digitalLED.repeatCounter++;

    if (digitalLED.repeatCounter == PIXELS2 / 2)
    {
        // Transfer of all LEDs is done, disable timer
        HAL_TIM_Base_Stop(&htim1);

        // Manually set outputs to low to generate 50us reset impulse
        DATA1_GPIO_Port->BRR = *dLED_IOs;
    }
    else
    {
        // Load bitbuffer with next RGB LED values
        loadNextFramebufferData(&digitalLED.item[0], 1, true);
        loadNextFramebufferData(&digitalLED.item[1], 1, false);
    }
}

void sendBuffer()
{
    // 47 Segments - reverse buffer
    digitalLED.item[0].frameBufferCounter = digitalLED.item[0].frameBufferSize - 1;
    loadNextFramebufferData(&digitalLED.item[0], 0, true);
    loadNextFramebufferData(&digitalLED.item[0], 1, true);

    // 84 Segments
    digitalLED.item[1].frameBufferCounter = 0;
    loadNextFramebufferData(&digitalLED.item[1], 0, false);
    loadNextFramebufferData(&digitalLED.item[1], 1, false);

    // start TIM2
    __HAL_TIM_SetCounter(&htim1, htim1.Init.Period - 1);
    HAL_TIM_Base_Start(&htim1);
}

extern "C" void digitalLEDTask(void *)
{
    auto lastWakeTime = xTaskGetTickCount();

    HAL_DMA_RegisterCallback(&hdma_tim1_ch2, HAL_DMA_XFER_HALFCPLT_CB_ID, transferHalfHandler);
    HAL_DMA_RegisterCallback(&hdma_tim1_ch2, HAL_DMA_XFER_CPLT_CB_ID, transferCompleteHandler);

    HAL_DMA_Start(&hdma_tim1_up, reinterpret_cast<uint32_t>(dLED_IOs),
                  reinterpret_cast<uint32_t>(&DATA1_GPIO_Port->BSRR), BUFFER_SIZE);

    HAL_DMA_Start(&hdma_tim1_ch1, reinterpret_cast<uint32_t>(DMABitBuffer),
                  reinterpret_cast<uint32_t>(&DATA1_GPIO_Port->BRR), BUFFER_SIZE);

    HAL_DMA_Start_IT(&hdma_tim1_ch2, reinterpret_cast<uint32_t>(dLED_IOs),
                     reinterpret_cast<uint32_t>(&DATA1_GPIO_Port->BRR), BUFFER_SIZE);

    HAL_GPIO_WritePin(EN_MOS1_GPIO_Port, EN_MOS1_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(EN_MOS2_GPIO_Port, EN_MOS2_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(EN_MOS3_GPIO_Port, EN_MOS3_Pin, GPIO_PIN_SET);
    ledGreen1->mode = StatusLedMode::On;
    ledGreen2->mode = StatusLedMode::On;
    ledGreen3->mode = StatusLedMode::On;

    while (1)
    {
        sendBuffer();
        vTaskDelayUntil(&lastWakeTime, pdMS_TO_TICKS(500));
    }
}