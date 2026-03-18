cmake_minimum_required(VERSION 3.16)

# ensure CMAKE_MSVC_RUNTIME_LIBRARY is honored
cmake_policy(SET CMP0091 NEW)

# use static multithreading library on windows
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

project(MandelbrotViewer LANGUAGES C CXX)

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

# enable AVX2 for the SIMD translation unit so Highway can emit wider vectors
if(MSVC)
    set_source_files_properties(src/simd_handler.cpp PROPERTIES COMPILE_FLAGS "/arch:AVX2")
elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64|AMD64|i[3-6]86")
    set_source_files_properties(src/simd_handler.cpp PROPERTIES COMPILE_FLAGS "-mavx2 -mfma")
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