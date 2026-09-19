# mpconfigboard.cmake - Board configuration for ESP32-S3 N16R8
#
# Based on ESP32_GENERIC_S3 but with:
#   - Octal PSRAM (R8 = 8MB Octal) instead of Quad
#   - 16 MB Flash
#   - Custom partition table
#   - Our own manifest.py with c_module() calls

set(IDF_TARGET esp32s3)

set(SDKCONFIG_DEFAULTS
    boards/sdkconfig.base
    boards/sdkconfig.ble
    boards/sdkconfig.flash_qio_80m
    boards/sdkconfig.240mhz
    boards/sdkconfig.spiram_oct
    # Board-specific overrides (16MB flash size + custom partition table)
    ${MICROPY_BOARD_DIR}/sdkconfig
)

# Use our own manifest that registers translate + ulab C modules.
set(MICROPY_FROZEN_MANIFEST ${MICROPY_BOARD_DIR}/manifest.py)
