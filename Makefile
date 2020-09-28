TARGET	:= roomlightV2
RTOS    := freertos
DEVICE	:= stm32f103cb

INCDIRS			:=  \
src 				\

SOURCES			:=	\
src/StatusLeds.cxx

# Actual build engine
include core/mk/include.mk