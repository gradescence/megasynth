#pragma once
#include <stdint.h>

/**
 * UART helper.
 * Def baud is 9600 unless UART_BAUD defined.
 */
void setupSerialComm();
void uartWriteChar(uint8_t data);

/**
 * Writes a null-terminated string followed by '\n'.
 */
void uartWriteString(const char* s);
