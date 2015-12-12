include_directories(${PROJECT_SOURCE_DIR}/src)
link_directories(${PROJECT_BINARY_DIR}/src)

# The version number
set (snd_tk_VERSION_MAJOR 0)
set (snd_tk_VERSION_MINOR 11)

# Pass detected settings to the source code
configure_file (
        "${PROJECT_SOURCE_DIR}/src/config.h.in"
        "${PROJECT_BINARY_DIR}/config.h"
)

# Path to generated config.h file
include_directories(${PROJECT_BINARY_DIR})

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

add_executable(snd_enhance_tk ${SOURCE_FILES})

# Link to sndfile fftw3 and GNU Math library
target_link_libraries(snd_enhance_tk ${CORELIBS})
