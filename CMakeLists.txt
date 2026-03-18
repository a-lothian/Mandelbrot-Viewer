cmake_minimum_required(VERSION 3.16)
project(MandelbrotViewer LANGUAGES C CXX)

# use static multitheading library on windows
if (MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded")
endif()

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED True)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# SDL3 package required for static linking
find_package(SDL3 REQUIRED CONFIG)

add_executable(Mandelbrot
    src/main.c
    src/benchmark.c
    src/mandelbrot.c
    src/simd_handler.cpp
    src/inputHandler.c
    src/core_count.c
    src/colour_palette.c
)

find_package(hwy CONFIG REQUIRED)

if (WIN32)
    target_link_libraries(Mandelbrot PRIVATE SDL3::SDL3-static hwy::hwy)
else()
    target_link_libraries(Mandelbrot PRIVATE SDL3::SDL3 hwy::hwy)
endif()

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
    target_compile_options(Mandelbrot PRIVATE $<$<CONFIG:Release>:-O3 -ffast-math>)
endif()

# add include directory explicitly so <SDL3/SDL.h> works everywhere
target_include_directories(Mandelbrot PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}
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