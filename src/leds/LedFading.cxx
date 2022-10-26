#include "LedFading.hpp"
#include "helpers/freertos.hpp"

#include <climits>

void LedFading::taskMain(void *)
{
    constexpr auto FadingTime = 300.0_ms;
    constexpr auto DelayTime = 8.0_ms;
    constexpr auto NumberOfSteps = (FadingTime / DelayTime).getMagnitude<uint8_t>();

    uint8_t factor;
    bool restart = false;

    while (true)
    {
        if (!restart)
            notifyWait(0, ULONG_MAX, nullptr, portMAX_DELAY);

        restart = false;
        factor = NumberOfSteps - 1;

        // calc difference between current and target data
        for (uint32_t i = 0; i < TotalPixels; i++)
        {
            ledDiffData[i].green = addrLeds.getCurrentLedArray()[i].green - ledTargetData[i].green;
            ledDiffData[i].red = addrLeds.getCurrentLedArray()[i].red - ledTargetData[i].red;
            ledDiffData[i].blue = addrLeds.getCurrentLedArray()[i].blue - ledTargetData[i].blue;
            ledDiffData[i].white = addrLeds.getCurrentLedArray()[i].white - ledTargetData[i].white;
        }

        while (true)
        {
            // apply difference multiplied by factor to current data
            for (uint32_t i = 0; i < TotalPixels; i++)
            {
                addrLeds.getCurrentLedArray()[i].green = // green
                    ledTargetData[i].green + (factor * ledDiffData[i].green) / NumberOfSteps;

                addrLeds.getCurrentLedArray()[i].red = // red
                    ledTargetData[i].red + (factor * ledDiffData[i].red) / NumberOfSteps;

                addrLeds.getCurrentLedArray()[i].blue = // blue
                    ledTargetData[i].blue + (factor * ledDiffData[i].blue) / NumberOfSteps;

                addrLeds.getCurrentLedArray()[i].white = // white
                    ledTargetData[i].white + (factor * ledDiffData[i].white) / NumberOfSteps;
            }

            // trigger task to render led data
            addrLeds.notify(1, util::wrappers::NotifyAction::SetBits);

            if (factor == 0)
                break;

            factor--;

            uint32_t notifiedValue;
            xTaskNotifyWait(0, ULONG_MAX, &notifiedValue, toOsTicks(DelayTime));
            if ((notifiedValue & 0x01U) != 0)
            {
                // restart fading
                restart = true;
                break;
            }
        }
    }
}