cmake_minimum_required(VERSION 3.16)
project(MandelbrotViewer)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED True)

# SDL3 package required for static linking
find_package(SDL3 REQUIRED CONFIG)

add_executable(Mandelbrot
    src/main.c
    src/mandelbrot.c
    src/inputHandler.c
    src/core_count.c
    src/colour_palette.c
)

target_link_libraries(Mandelbrot PRIVATE SDL3::SDL3-static)

if(WIN32)
    find_package(pthreads REQUIRED CONFIG)
    target_link_libraries(Mandelbrot PRIVATE PThreads4W::PThreads4W)
else()
    find_package(Threads REQUIRED)
    target_link_libraries(Mandelbrot PRIVATE Threads::Threads)
    if(NOT APPLE)
        target_link_libraries(Mandelbrot PRIVATE m)
    endif()
endif()

# compiler optimisations for Release builds
if(MSVC)
    target_compile_options(Mandelbrot PRIVATE $<$<CONFIG:Release>:/O2 /fp:fast>)
else()
    target_compile_options(Mandelbrot PRIVATE $<$<CONFIG:Release>:-O3 -ffast-math -march=native>)
endif()

# add include directory explicitly so <SDL3/SDL.h> works everywhere
target_include_directories(Mandelbrot PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

if(WIN32)
    # prerequisites for SDL3 compilation
    target_link_libraries(Mandelbrot PRIVATE
        winmm
        imm32
        version
        setupapi
    )
endif()