#include "Button.hpp"

void Button::update(const units::si::Time timePassed)
{
    State state = buttonGpio.read() ? State::Pressed : State::NotPressed;

    switch (internalState)
    {
    case InternalState::Idle:
        if (state == State::Pressed)
        {
            internalState = InternalState::Pressed;
            loadTimer();
        }
        break;

    case InternalState::Pressed:
        updateTimer(timePassed);

        if (state == State::NotPressed)
        {
            if (getPassedTime() >= DebounceTime)
                callback(Action::ShortPress);

            internalState = InternalState::Idle;
        }
        else if (getPassedTime() >= longPressTime)
        {
            callback(Action::LongPress);
            internalState = InternalState::LongPress;
        }
        break;

    case InternalState::LongPress:
        if (state == State::NotPressed)
        {
            callback(Action::StopLongPress);
            internalState = InternalState::Idle;
        }

        break;
    }
}

void Button::loadTimer()
{
    pressTimer = TimerReloadValue;
}

void Button::updateTimer(const units::si::Time timePassed)
{
    pressTimer += timePassed;
}

units::si::Time Button::getPassedTime() const
{
    return pressTimer;
}
