TARGET	:= roomlightV2
RTOS    := freertos
DEVICE	:= stm32f103cb

INCDIRS			:=  \
src 				\

SOURCES			:=		\
src/adc.cxx				\
src/digitalLED.cxx		\
src/StatusLeds.cxx		\
src/uart.cxx			\
src/Wifi.cxx

# Actual build engine
include core/mk/include.mk