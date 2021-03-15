TARGET	:= roomlightV2
RTOS    := freertos
DEVICE	:= stm32f103cb

INCDIRS			:=  \
src                 \
src/util/include

SOURCES :=                      \
src/button/button_handler.cxx   \
src/button/Button.cxx           \
src/adc.cxx                     \
src/digitalLED.cxx              \
src/handlers.cxx                \
src/StatusLeds.cxx              \
src/uart.cxx                    \
src/Wifi.cxx

# build engine
include core/mk/include.mk