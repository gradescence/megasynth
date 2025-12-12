#ifndef BUTTON_HPP
#define BUTTON_HPP
#include "Arduino.h"

// Variable for the button state pressed (true), not pressed (false).
// It will a shared variable used by ISR and the check state function.
volatile bool buttonPressed = false;

void setupButton() {
  // Configure the button pin 2 (PE4) as an input with internal pull-up
  DDRE &= ~(1 << PE4); //Set PE4 is input
  PORTE |= (1 << PE4); //Enable internal pull up resistor for PE4

  // Configure the interrupt trigger mode (Falling Edge)
  // Pin 2 is INT4, controlled by EICRB register (bits ISC41 and ISC40)
  // 1 0 = Falling Edge
  EICRB |= (1 << ISC41);  // Set ISC41 (bit 3) to 1
  EICRB &= ~(1 << ISC40); // Set ISC40 (bit 2) to 0

  // Enable external interrupt INT4
  EIMSK |= (1 << INT4); // Set INT4 (bit 4) in EIMSK
}

// Interrupt Service Routine (ISR)
// The vector name for INT5 is INT5_vect
ISR(INT4_vect) {
  // Button press detected.
  buttonPressed = true;
}


// Return the current state of push button true (pressed), false otherwise.
bool isPushButtonPressed(){
  if (buttonPressed) {
    buttonPressed = false; // Reset
    return true;
  }
  return false;
}

#endif