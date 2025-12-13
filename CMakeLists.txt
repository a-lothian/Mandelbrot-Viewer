cmake_minimum_required(VERSION 3.12)
project(MandelbrotViewer)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED True)

find_package(SDL2 REQUIRED)

set(SOURCES
    main.c
    mandelbrot.c
    inputHandler.c
)

add_executable(Mandelbrot ${SOURCES})

target_link_libraries(Mandelbrot PRIVATE SDL2::SDL2main SDL2::SDL2)

if(UNIX AND NOT APPLE)
  target_link_libraries(Mandelbrot PRIVATE m)
endif()

# add include directory explicitly so <SDL2/SDL.h> works everywhere
target_include_directories(Mandelbrot PRIVATE ${SDL2_INCLUDE_DIRS})

# copy the SDL2.dll to the executable folder on windows
if(WIN32)
    add_custom_command(TARGET Mandelbrot POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:SDL2::SDL2>
        $<TARGET_FILE_DIR:Mandelbrot>
    )
endif()