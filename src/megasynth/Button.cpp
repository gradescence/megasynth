#include "Button.hpp"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/atomic.h>

namespace {
volatile bool s_buttonPressed = false;
}

// External interrupt INT4 
ISR(INT4_vect) {
  s_buttonPressed = true;
}

void setupButton() {
  // PE4 as input with pull-up
  DDRE &= (uint8_t)~_BV(PE4);
  PORTE |= _BV(PE4);

  // Configure INT4 to trigger on falling edge
  EICRB &= (uint8_t)~_BV(ISC40);
  EICRB |= _BV(ISC41);

  // Clear any pending INT4 flag and enable INT4
  EIFR |= _BV(INTF4);
  EIMSK |= _BV(INT4);
}

bool isPushButtonPressed() {
  bool pressed = false;

  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
    pressed = s_buttonPressed;
    s_buttonPressed = false;
  }

  return pressed;
}
