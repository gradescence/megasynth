#ifndef SERIALCOMM_HPP
#define SERIALCOMM_HPP

#include <avr/io.h> // Include AVR I/O definitions

// Configure the UART for:
// 9600 baud rate, 8 data bits, 1 stop bit, no parity.
void setupSerialComm() {
  // Set baud rate to 9600
  // For 16MHz clock and 9600 baud, UBRR value is 103
  //  (calculated as (F_CPU / 16 / BAUD) - 1)
  UBRR0H = (unsigned char)(103 >> 8);
  UBRR0L = (unsigned char)103;

  // Enable transmitter (TXEN0) and receiver (RXEN0)
  UCSR0B = (1 << RXEN0) | (1 << TXEN0);

  // Set frame format: 8 data bits, 1 stop bit, no parity
  UCSR0C = (3 << UCSZ00); // 8 data bits (UCSZ00 and UCSZ01 set)
}

// Write a single character.
void uartWriteChar(unsigned char data) {
  // Wait for empty transmit buffer
  while (!(UCSR0A & (1 << UDRE0)));

  // Put data into buffer, sends the data
  UDR0 = data;

  // Wait for transmission complete (optional)
  while (!(UCSR0A & (1 << TXC0)));
  UCSR0A |= (1 << TXC0); // Clear TXC flag
}

// Write several characters (string)
// and new line at the end.
void uartWriteString(const char *s) {
  while (*s) {
    uartWriteChar(*s++);
  }
  uartWriteChar('\n');
}


#endif