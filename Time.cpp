#ifndef TIME_HPP
#define TIME_HPP

#include "Arduino.h"
#include <avr/io.h>
#include <avr/interrupt.h>

/*
* This library implements thw folowing time resources:
* Function to ger number milliseconds since initialization. Non-blocking call.
* Function to delay execition for a given number of milliseconds. Blocking call.
* It uses Timer 2 to support get elapsed milliseconds.
* It uses Timer 3 to support sleep a number of milliseconds.
*/

volatile unsigned long elapsetMilliseconds = 0; // Global variable to track milliseconds since setup.
volatile unsigned long delayMillisecondsCount = 0; // Global variable to track delay millisecodsn.

// Function to initialize Timers used by 
// It uses Timer 2 to support getElapsedMilliseconds()
// It uses Timer 3 to support sleepMilliseconds()
void setupTime() {
  /*
  * Configure Timer2 for 1ms interrupt
  * Timer 2 supports getElapsedMilliseconds()
  */
  noInterrupts(); // Disable interrupts during setup
  TCCR2A = 0;    // Clear TCCR1A register
  TCCR2B = 0;    // Clear TCCR1B register
  TCNT2 = 0;     // Initialize counter value to 0

  // Set CTC mode (Clear Timer on Compare Match)
  // WGM12 bit in TCCR1B for CTC mode
  TCCR2B |= (1 << WGM12);

  // Set prescaler to 64 (for 16MHz clock, 16,000,000 / 64 = 250,000 Hz)
  // CS11 and CS10 bits in TCCR1B for prescaler of 64
  TCCR2B |= (1 << CS11) | (1 << CS10);

  // Set compare match register for 1ms interrupt
  // (250,000 Hz / 1000 Hz = 250 counts per millisecond)
  OCR2A = 249; // Count from 0 to 249 (250 counts)

  // Enable Timer Compare Interrupt A
  TIMSK2 |= (1 << OCIE1A);

  /*
   * Configure Timer3 for a 1-millisecond interrupt.
   * This timer spports sleepMillisecods()
   */

  TCCR3A = 0;
  TCCR3B = 0;
  TCNT3 = 0; // Reset timer count
  // Set compare match register for 1ms interrupt
  // (250,000 Hz / 1000 Hz = 250 counts per millisecond)
  OCR3A = 249; // Count from 0 to 249 (250 counts)
  TCCR3B |= (1 << WGM12); // CTC mode
  TCCR3B |= (1 << CS11) | (1 << CS10); // Prescaler 64
  TIMSK3 |= (1 << OCIE1A); // Enable Timer1 Compare Match A Interrupt

  interrupts(); // enable interrupts
}

// Interrupt Service Routine for Timer2 Compare Match A
ISR(TIMER2_COMPA_vect) {
  // One milliseconds elapsed, increment the milliseconds counter.
  elapsetMilliseconds++;
}

// Interrupt Service Routine for Timer3 Compare Match A
ISR(TIMER3_COMPA_vect) {
  delayMillisecondsCount++; // Increment counter every milliecond.
}

// Function to get milliseconds since initialization.
// It supported by Timer 2.
unsigned long getElapsedMillis() {
  unsigned long currentMillis;
  // Disable interupts to prevent conflict with ISR when accesing elapsetMilliseconds.
  noInterrupts(); 
  currentMillis = elapsetMilliseconds;
  // Re-enable interupts.
  interrupts();
  return currentMillis;
}

// Function to delay current execution for the given millisecods.
// It is supported by Timer 3
void sleepMillis(unsigned long milliseconds) {
  noInterrupts(); 
  delayMillisecondsCount = 0;
  interrupts(); 
  while(true){
    if( delayMillisecondsCount >= milliseconds){
      break;
    }
  }
}

#endif
