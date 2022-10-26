#pragma once

#include "AddressableLeds.hpp"
#include "ledConstants.hpp"
#include "wrappers/Task.hpp"

class LedFading : public util::wrappers::TaskWithMemberFunctionBase
{
public:
    LedFading(AddressableLeds &addressableLeds)
        : TaskWithMemberFunctionBase("ledFadingTask", 128, osPriorityAboveNormal7), //
          addrLeds(addressableLeds){};

    LedArray &getTargetArray()
    {
        return ledTargetData;
    }

protected:
    [[noreturn]] void taskMain(void *) override;

private:
    AddressableLeds &addrLeds;
    LedArray ledTargetData{};

    struct DiffLEDSegment
    {
        int16_t green = 0;
        int16_t red = 0;
        int16_t blue = 0;
        int16_t white = 0;
    };

    std::array<DiffLEDSegment, TotalPixels> ledDiffData{};
};