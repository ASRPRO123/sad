# CMake integration for the MicroPython "translate" user C module.
#
# This file is picked up automatically when the directory is passed
# to MicroPython's build system via:
#   - the USER_C_MODULES environment variable, or
#   - c_module("...") in a manifest.py
#
# The interface library name must start with "usermod_" per MicroPython
# convention.

add_library(usermod_translate INTERFACE)

target_sources(usermod_translate INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/translate.c
)

target_include_directories(usermod_translate INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}
)

# Link this module into the MicroPython firmware target.
# (Same pattern as ulab's micropython.cmake.)
target_link_libraries(usermod INTERFACE usermod_translate)
