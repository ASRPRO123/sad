/*
 * mpconfigboard.h - Board-specific config for ESP32-S3 N16R8
 *
 * Hardware: ESP32-S3-WROOM-1 N16R8
 *   - 16 MB Quad SPI Flash
 *   - 8 MB  Octal SPI PSRAM (OPI)
 */

#define MICROPY_HW_BOARD_NAME       "ESP32-S3 N16R8 (16MB Flash, 8MB Octal PSRAM)"
#define MICROPY_HW_MCU_NAME         "ESP32S3"

/* Default hostname when WiFi is configured */
#define MICROPY_HW_ESP32_HOSTNAME  "mp-translate"

/* UART REPL (via on-board USB-UART bridge) */
#define MICROPY_HW_ENABLE_UART_REPL   (1)
#define MICROPY_HW_UART_REPL_BAUD     115200

/* Flash size is 16 MB, PSRAM is 8 MB octal - configured via sdkconfig */
