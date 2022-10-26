#pragma once

#include "gpio.hpp"
#include <units/si/time.hpp>

#include <functional>

namespace util
{
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

    using Callback = std::function<void(Action action)>;

private:
    enum class InternalState
    {
        Idle,
        Pressed,
        LongPress
    };

public:
    explicit Button(util::Gpio &buttonGpio, Callback callback, bool invert = false,
                    const units::si::Time longPressTime = 500.0_ms)
        : buttonGpio{buttonGpio},   //
          ButtonCallback{callback}, //
          InvertedInput{invert},    //
          LongPressTime{longPressTime}
    {
    }

    void update(units::si::Time timePassed);
    bool isPressing() const;

private:
    void loadTimer();
    void updateTimer(units::si::Time timePassed);
    [[nodiscard]] units::si::Time getPassedTime() const;

    static constexpr units::si::Time TimerReloadValue = 0.0_s;
    static constexpr units::si::Time DebounceTime = 50.0_ms;

    util::Gpio &buttonGpio;
    const Callback ButtonCallback;
    const bool InvertedInput; //! true means that pulled up pin levels will be detected as pressed
    const units::si::Time LongPressTime;
    InternalState internalState = InternalState::Idle;
    units::si::Time pressTimer = TimerReloadValue;
};
} // namespace util