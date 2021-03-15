#include "Button.hpp"

void Button::update(const units::si::Time timePassed)
{
    State state = buttonGpio.read() ? State::NotPressed : State::Pressed;

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
                    callback(id, Action::ShortPress);

                internalState = InternalState::Idle;
            }
            else if (getPassedTime() >= longPressTime)
            {
                callback(id, Action::LongPress);
                internalState = InternalState::LongPress;
            }
            break;

        case InternalState::LongPress:
            if (state == State::NotPressed)
            {
                callback(id, Action::StopLongPress);
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
