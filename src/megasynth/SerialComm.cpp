#include "SerialComm.hpp"

#include <avr/io.h>

#ifndef F_CPU
#define F_CPU 16000000UL
#endif

#ifndef UART_BAUD
#define UART_BAUD 9600UL
#endif

static uint16_t computeUbr0() {
  // UBRR = round(F_CPU / (16*BAUD)) - 1
  const uint32_t denom = 16UL * (uint32_t)UART_BAUD;
  return (uint16_t)((F_CPU + (denom / 2)) / denom - 1UL);
}

void setupSerialComm() {
  const uint16_t ubrr = computeUbr0();
  UBRR0H = (uint8_t)(ubrr >> 8);
  UBRR0L = (uint8_t)ubrr;

  // 8N1
  UCSR0A = 0;
  UCSR0B = _BV(RXEN0) | _BV(TXEN0);
  UCSR0C = _BV(UCSZ01) | _BV(UCSZ00);
}

void uartWriteChar(uint8_t data) {
  while (!(UCSR0A & _BV(UDRE0))) {
    // wait for empty transmit buffer
  }
  UDR0 = data;
}

void uartWriteString(const char* s) {
  while (*s) uartWriteChar((uint8_t)*s++);
  uartWriteChar((uint8_t)'\n');
}
