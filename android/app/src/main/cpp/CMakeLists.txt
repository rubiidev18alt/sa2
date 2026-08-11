cmake_minimum_required(VERSION 3.22)
project(sa2_android C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

set(SDL2_DIR "${CMAKE_CURRENT_LIST_DIR}/../../../../../ext/SDL2")
set(SA2_ROOT "${CMAKE_CURRENT_LIST_DIR}/../../../../../")
set(PREPROC_ROOT "${SA2_ROOT}/tools/preproc")

if(NOT EXISTS "${SDL2_DIR}/CMakeLists.txt")
    message(FATAL_ERROR "SDL2 is missing. Run ./gradlew fetchSDL2 first.")
endif()

set(SDL_TESTS OFF CACHE BOOL "" FORCE)
set(SDL_TEST_LIBRARY OFF CACHE BOOL "" FORCE)
set(SDL_SHARED ON CACHE BOOL "" FORCE)
set(SDL_STATIC OFF CACHE BOOL "" FORCE)
add_subdirectory("${SDL2_DIR}" "${CMAKE_CURRENT_BINARY_DIR}/SDL2" EXCLUDE_FROM_ALL)

add_executable(sa2_preproc_host
    "${PREPROC_ROOT}/preproc.cpp"
    "${PREPROC_ROOT}/asm_file.cpp"
    "${PREPROC_ROOT}/c_file.cpp"
    "${PREPROC_ROOT}/charmap.cpp"
    "${PREPROC_ROOT}/string_parser.cpp"
    "${PREPROC_ROOT}/utf8.cpp"
    "${PREPROC_ROOT}/ext/gbagfx/jasc_pal.cpp"
)
target_include_directories(sa2_preproc_host PRIVATE
    "${PREPROC_ROOT}"
    "${PREPROC_ROOT}/ext/gbagfx"
)
target_compile_options(sa2_preproc_host PRIVATE -O2 -Wno-unused-parameter)

file(GLOB_RECURSE SA2_SOURCES CONFIGURE_DEPENDS
    "${SA2_ROOT}/src/*.c"
)

list(FILTER SA2_SOURCES EXCLUDE REGEX ".*/src/platform/win32/.*")
list(FILTER SA2_SOURCES EXCLUDE REGEX ".*/src/platform/ps2/.*")
list(FILTER SA2_SOURCES EXCLUDE REGEX ".*/src/platform/pret_sdl/.*")
list(APPEND SA2_SOURCES
    "${SA2_ROOT}/src/platform/pret_sdl/sdl2.c"
    "${SA2_ROOT}/libagbsyscall/libagbsyscall.c"
    "${CMAKE_CURRENT_LIST_DIR}/android_bridge.c"
)

set(SA2_GENERATED_DIR "${CMAKE_CURRENT_BINARY_DIR}/generated")
set(SA2_GENERATED_SOURCES "")

set(SA2_CPP_DEFINES
    -DGAME=GAME_SA2
    -DGAME_REGION=USA
    -DGAME_VARIANT=DEFAULT
    -DPLATFORM_GBA=0
    -DPLATFORM_SDL=1
    -DPLATFORM_WIN32=0
    -DCPU_ARCH_X86=0
    -DCPU_ARCH_ARM=0
    -DPORTABLE=1
    -DDEBUG=0
    -DENABLE_DECOMP_CREDITS=1
    -DTITLE_BAR=sa2_android
)

foreach(SOURCE IN LISTS SA2_SOURCES)
    file(RELATIVE_PATH REL_SOURCE "${SA2_ROOT}" "${SOURCE}")
    string(REPLACE "/" "_" GENERATED_NAME "${REL_SOURCE}")
    set(PP_FILE "${SA2_GENERATED_DIR}/${GENERATED_NAME}.i.c")
    set(GENERATED_FILE "${SA2_GENERATED_DIR}/${GENERATED_NAME}.c")

    add_custom_command(
        OUTPUT "${GENERATED_FILE}"
        COMMAND ${CMAKE_COMMAND} -E make_directory "${SA2_GENERATED_DIR}"
        COMMAND ${CMAKE_C_COMPILER} -E -P
            ${SA2_CPP_DEFINES}
            -I${SA2_ROOT}/include
            -I${SDL2_DIR}/include
            "${SOURCE}" -o "${PP_FILE}"
        COMMAND $<TARGET_FILE:sa2_preproc_host> "${PP_FILE}" sdl "" "${GENERATED_FILE}"
        DEPENDS "${SOURCE}" sa2_preproc_host
        WORKING_DIRECTORY "${SA2_ROOT}"
        VERBATIM
    )
    set_source_files_properties("${GENERATED_FILE}" PROPERTIES GENERATED TRUE)
    list(APPEND SA2_GENERATED_SOURCES "${GENERATED_FILE}")
endforeach()

add_library(main SHARED ${SA2_GENERATED_SOURCES})

target_compile_definitions(main PRIVATE
    GAME=GAME_SA2
    GAME_REGION=USA
    GAME_VARIANT=DEFAULT
    PLATFORM_GBA=0
    PLATFORM_SDL=1
    PLATFORM_WIN32=0
    CPU_ARCH_X86=0
    CPU_ARCH_ARM=0
    PORTABLE=1
    DEBUG=0
    ENABLE_DECOMP_CREDITS=1
    TITLE_BAR=sa2_android
)

target_include_directories(main PRIVATE
    "${SA2_ROOT}/include"
    "${SA2_ROOT}/libagbsyscall"
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
