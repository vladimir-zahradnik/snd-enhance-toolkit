cmake_minimum_required(VERSION 3.3)
project(src)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES
    common.c
    common.h
    i18n.h
    lpc.c
    lpc.h
    noise_est.c
    noise_est.h
    snd_enhance.c
    snd_enhance.h
    tbessi.c
    tbessi.h
    toolkit.c
    toolkit.h
    window.c
    window.h)

add_executable(src ${SOURCE_FILES})