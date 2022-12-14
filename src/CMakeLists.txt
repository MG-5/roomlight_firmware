cmake_minimum_required(VERSION 3.15)
cmake_policy(SET CMP0048 NEW)
project(roomlight_firmware LANGUAGES CXX C ASM)

# boilerplate cmake for setting up compiler flags, extraction from cubemx code
# internal macros and such
include("core/cmake/setupEmbeddedBuild.cmake")
include("core/cmake/setupEmbeddedExtraFiles.cmake")
include("core/cmake/generateCubeMXTargets.cmake")

set(HalDir "cubemx")

# sets up compiler flags, you may change the c++ and c version here
SETUP_BUILD(${HalDir} ${PROJECT_NAME} c11 c++17)

# generates cmake libraries for hal and freertos
GENERATE_CUBEMX_TARGETS(${HalDir} true)

# instructs core to set all internal macros for embedded configuration
# Affects if arm specific code is compiled in which would SEGFAULT on non arm targets
set(core_BUILDCONFIGURATION "embedded")

add_subdirectory(core)
add_subdirectory(src/util)
add_subdirectory(src/leds/gamma/gcem)

set(INCLUDE_FILES
	src
)

set(Sources
	src/analog_to_digital/AnalogToDigital.cxx

	src/button/ButtonHandler.cxx

	src/leds/AddressableLeds.cxx
	src/leds/LedFading.cxx
	src/leds/StateMachine.cxx
	src/leds/StatusLeds.cxx

	src/wireless_gateway/PacketProcessor.cxx
	src/wireless_gateway/Wifi.cxx

	src/Application.cxx

	src/handlers.cxx
)

# Instructs CMake to build your firmware
add_executable(${PROJECT_NAME} ${Sources})
target_include_directories(${PROJECT_NAME} PUBLIC ${INCLUDE_FILES})

# Every call to add_subdirectory() will load the libraries' CMakeLists.txt file. Those will contain one or more
# static / object libraries whose names you have to add here. target_link_libraries will not only see that the
# libraries' code is added to your firmware but also sets up include directories. So you don't have to add them manually.
# Read libraries' README files for information about specifics.
target_link_libraries(${PROJECT_NAME} PRIVATE

        # Core's libraries
        hal
        core
        freertos
        util
        gcem
)

# Sets up generation of .list and .bin files after your firmware has generated.
# Makes firmware image generate as an .elf file.
# Also performs checks for some common bugs that can happen in buildsystem configuration
SETUP_EXTRA_FILE_GENERATION(${PROJECT_NAME} ../core)
