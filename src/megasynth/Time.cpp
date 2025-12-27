#include "Time.hpp"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/atomic.h>

#ifndef F_CPU
#define F_CPU 16000000UL
#endif

namespace {
volatile unsigned long s_elapsedMs = 0;
}

// Timer2 Compare Match A -> 1ms tick
ISR(TIMER2_COMPA_vect) {
  s_elapsedMs++;
}

void setupTime() {
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
    // Stop timer
    TCCR2A = 0;
    TCCR2B = 0;
    TCNT2  = 0;

    // CTC mode (TOP = OCR2A)
    TCCR2A |= _BV(WGM21);

    // Prescaler 64 -> 16MHz / 64 = 250kHz
    // 250kHz / 1000Hz = 250 counts -> OCR2A = 249
    TCCR2B |= _BV(CS22);  // /64

    OCR2A = 249;

    TIMSK2 |= _BV(OCIE2A);

    s_elapsedMs = 0;
  }

  // ensure global interrupts enabled
  interrupts();
}

unsigned long getElapsedMillis() {
  unsigned long ms;
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) { ms = s_elapsedMs; }
  return ms;
}

void sleepMillis(unsigned long milliseconds) {
  const unsigned long start = getElapsedMillis();
  while ((unsigned long)(getElapsedMillis() - start) < milliseconds) {
    // busy wait; interrupts stay enabled
  }
}
