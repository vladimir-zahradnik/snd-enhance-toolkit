cmake_minimum_required(VERSION 2.7)
project(snd_enhance_toolkit)

# Path to custom CMake modules
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmake/Modules/")

# Detect sndfile presence
find_package(SndFile REQUIRED)

# Detect FFTW presence
find_package(FFTW REQUIRED)

# Detect presence of GNU Math library
find_package(MATH REQUIRED)

include_directories(${SNDFILE_INCLUDE_DIRS} ${FFTW_INCLUDES} ${MATH_INCLUDE_DIR})

# Required libraries
set(CORELIBS ${SNDFILE_LIBRARY} ${FFTW_LIBRARIES} ${MATH_LIBRARIES})

set(CMAKE_C_FLAGS ${CMAKE_C_FLAGS} "-g -Wall -std=gnu99")
add_subdirectory(src)
