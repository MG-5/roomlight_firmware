#pragma once

#include "util/gpio.hpp"
#include <units/si/time.hpp>

class Button
{
public:
    enum class State
    {
        NotPressed = 0,
        Pressed = 1
    };

    enum class Action
    {
        ShortPress,
        LongPress,
        StopLongPress
    };

    using Callback = void (*)(Action action);

private:
    enum class InternalState
    {
        Idle,
        Pressed,
        LongPress
    };

public:
    constexpr Button(util::Gpio buttonGpio, Callback callback, const units::si::Time longPressTime)
        : buttonGpio{buttonGpio}, callback{callback}, longPressTime{longPressTime}
    {
    }

    void update(units::si::Time timePassed);

private:
    void loadTimer();
    void updateTimer(units::si::Time timePassed);
    [[nodiscard]] units::si::Time getPassedTime() const;

    static constexpr units::si::Time TimerReloadValue = 0.0_s;
    static constexpr units::si::Time DebounceTime = 50.0_ms;

    util::Gpio buttonGpio;
    const Callback callback;
    const units::si::Time longPressTime;
    InternalState internalState = InternalState::Idle;
    units::si::Time pressTimer = TimerReloadValue;
};
