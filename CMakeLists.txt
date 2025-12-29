cmake_minimum_required(VERSION 3.16)
project(MandelbrotViewer)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED True)

find_package(SDL3 REQUIRED CONFIG)

add_executable(Mandelbrot
    src/main.c
    src/mandelbrot.c
    src/inputHandler.c
    src/core_count.c
    src/colour_palette.c
)

target_link_libraries(Mandelbrot PRIVATE SDL3::SDL3)

if(UNIX AND NOT APPLE)
  target_link_libraries(Mandelbrot PRIVATE m)
endif()

# add include directory explicitly so <SDL3/SDL.h> works everywhere
target_include_directories(Mandelbrot PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# copy the SDL3.dll to the executable folder on windows
if(WIN32)
    set(PTHREAD_DIR "${CMAKE_CURRENT_SOURCE_DIR}/dependencies/pthread4w/x64")

    # Add headers
    target_include_directories(Mandelbrot PRIVATE "${PTHREAD_DIR}/include")

    # link .lib's
    target_link_libraries(Mandelbrot PRIVATE "${PTHREAD_DIR}/lib/pthreadVC2.lib")

    # Copy .dll's to build
    # SDL3 .dll
    add_custom_command(TARGET Mandelbrot POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:SDL3::SDL3>
        $<TARGET_FILE_DIR:Mandelbrot>
    )

    # Pthread .dll
    add_custom_command(TARGET Mandelbrot POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${PTHREAD_DIR}/dll/pthreadVC2.dll"
        $<TARGET_FILE_DIR:Mandelbrot>
    )
endif()