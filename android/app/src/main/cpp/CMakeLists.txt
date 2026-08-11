cmake_minimum_required(VERSION 3.22)
project(sa2_android C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

set(SDL2_DIR "${CMAKE_CURRENT_LIST_DIR}/../../../../../ext/SDL2")
if(NOT EXISTS "${SDL2_DIR}/CMakeLists.txt")
    message(FATAL_ERROR "SDL2 is missing. Run ./gradlew fetchSDL2 first.")
endif()

set(SDL_TESTS OFF CACHE BOOL "" FORCE)
set(SDL_TEST_LIBRARY OFF CACHE BOOL "" FORCE)
set(SDL_SHARED ON CACHE BOOL "" FORCE)
set(SDL_STATIC OFF CACHE BOOL "" FORCE)
add_subdirectory("${SDL2_DIR}" "${CMAKE_CURRENT_BINARY_DIR}/SDL2" EXCLUDE_FROM_ALL)

file(GLOB_RECURSE SA2_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_LIST_DIR}/../../../../../src/*.c"
)

list(FILTER SA2_SOURCES EXCLUDE REGEX ".*/src/platform/win32/.*")
list(FILTER SA2_SOURCES EXCLUDE REGEX ".*/src/platform/ps2/.*")
list(FILTER SA2_SOURCES EXCLUDE REGEX ".*/src/platform/pret_sdl/.*")

list(APPEND SA2_SOURCES
    "${CMAKE_CURRENT_LIST_DIR}/../../../../../src/platform/pret_sdl/sdl2.c"
    "${CMAKE_CURRENT_LIST_DIR}/../../../../../libagbsyscall/libagbsyscall.c"
)

add_library(main SHARED ${SA2_SOURCES})

target_compile_definitions(main PRIVATE
    PLATFORM_GBA=0
    PLATFORM_SDL=1
    PLATFORM_WIN32=0
    CPU_ARCH_X86=0
    CPU_ARCH_ARM=0
    GAME=GAME_SA2
    GAME_REGION=USA
    GAME_VARIANT=DEFAULT
    GAME_NAME=sa2
    PORTABLE=1
    DEBUG=0
    ENABLE_DECOMP_CREDITS=1
    TITLE_BAR=sa2_android
)

target_include_directories(main PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/../../../../../include"
    "${CMAKE_CURRENT_LIST_DIR}/../../../../../libagbsyscall"
    "${SDL2_DIR}/include"
)

target_compile_options(main PRIVATE
    -O2
    -Wno-parentheses-equality
    -Wno-unused-value
    -Wno-unused-function
    -Wno-unused-variable
)

target_link_libraries(main PRIVATE SDL2 m log android)
set_target_properties(main PROPERTIES OUTPUT_NAME "main")
