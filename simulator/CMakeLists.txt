cmake_minimum_required(VERSION 3.16)
project(clawdmeter_sim CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_STANDARD 11)

# SDL2 via pkg-config (cmake 3.22 on Ubuntu 22.04 does not ship FindSDL2.cmake)
find_package(PkgConfig REQUIRED)
pkg_check_modules(SDL2 REQUIRED sdl2)

# LVGL 9.3+ — fetched from GitHub on first build, cached thereafter.
# lv_conf.h is picked up automatically from CMAKE_SOURCE_DIR (this directory).
include(FetchContent)
FetchContent_Declare(
    lvgl
    GIT_REPOSITORY https://github.com/lvgl/lvgl.git
    GIT_TAG        v9.3.0
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(lvgl)

# LVGL v9.3+ globs all .S files including ARM NEON/Helium assembly that
# does not assemble on x86_64. Strip those sources from the lvgl target.
get_target_property(LVGL_ALL_SOURCES lvgl SOURCES)
list(FILTER LVGL_ALL_SOURCES EXCLUDE REGEX ".*(neon|helium).*\\.S$")
set_target_properties(lvgl PROPERTIES SOURCES "${LVGL_ALL_SOURCES}")

# Firmware source files — platform-independent subset only (no ble.cpp,
# no main.cpp). Arduino-specific headers are satisfied by stubs/.
set(FIRMWARE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../firmware/src")

set(FIRMWARE_SRCS
    ${FIRMWARE_DIR}/ui.cpp
    ${FIRMWARE_DIR}/splash.cpp
    ${FIRMWARE_DIR}/usage_rate.cpp
    ${FIRMWARE_DIR}/idle.cpp
    ${FIRMWARE_DIR}/font_mono_18.c
    ${FIRMWARE_DIR}/font_mono_32.c
    ${FIRMWARE_DIR}/font_styrene_12.c
    ${FIRMWARE_DIR}/font_styrene_14.c
    ${FIRMWARE_DIR}/font_styrene_16.c
    ${FIRMWARE_DIR}/font_styrene_20.c
    ${FIRMWARE_DIR}/font_styrene_24.c
    ${FIRMWARE_DIR}/font_styrene_28.c
    ${FIRMWARE_DIR}/font_styrene_48.c
    ${FIRMWARE_DIR}/font_tiempos_34.c
    ${FIRMWARE_DIR}/font_tiempos_56.c
)

set(SIM_SRCS
    main.cpp
    ApiPoller.cpp
    ble_stub.cpp
    boards/simulator/board_init.cpp
    boards/simulator/caps.cpp
    boards/simulator/display.cpp
    boards/simulator/touch.cpp
    boards/simulator/input.cpp
    boards/simulator/power.cpp
    boards/simulator/imu.cpp
)

add_executable(simulator ${FIRMWARE_SRCS} ${SIM_SRCS})

target_include_directories(simulator PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/stubs        # Arduino.h, esp_heap_caps.h
    ${CMAKE_CURRENT_SOURCE_DIR}              # lv_conf_sim.h, ble.h (via firmware)
    ${FIRMWARE_DIR}                          # firmware headers
    ${SDL2_INCLUDE_DIRS}
)

target_compile_definitions(simulator PRIVATE
    BOARD_SIMULATOR=1
)

# Suppress LVGL-internal warnings that are noise on desktop.
target_compile_options(simulator PRIVATE
    -Wno-unused-parameter
    -Wno-missing-field-initializers
)

find_package(Threads REQUIRED)

target_link_libraries(simulator PRIVATE
    lvgl::lvgl
    ${SDL2_LIBRARIES}
    Threads::Threads
    m
)
