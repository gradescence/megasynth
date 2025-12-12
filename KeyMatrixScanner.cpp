#ifndef KEYMATRIXSCANNER_HPP
#define KEYMATRIXSCANNER_HPP

#include <avr/io.h>
#include "Arduino.h"

const int ROWS = 5;
const int COLS = 5;

int numColumns(){
  return COLS;
}

int numRows(){
  return ROWS;
}

char keysMap[ROWS][COLS] = {
  {'A', 'B', 'C', 'D', 'E'},
  {'F', 'G', 'H', 'I', 'J'},
  {'K', 'L', 'M', 'N', 'O'},
  {'P', 'Q', 'R', 'S', 'T'},
  {'U', 'V', 'W', 'X', 'Y'}
};

void setupKeyMatrixScanner() {
  //Set columns as outputs
  DDRA |= (1 << PA0); // Set PA0 as output
  PORTA |= (1 << PA0); // Set PA0 HIGH
  DDRA |= (1 << PA2); // Set PA2 as output
  PORTA |= (1 << PA2); // Set PA2 HIGH
  DDRA |= (1 << PA4); // Set PA4 as output
  PORTA |= (1 << PA4); // Set PA4 HIGH
  DDRA |= (1 << PA6); // Set PA6 as output
  PORTA |= (1 << PA6); // Set PA6 HIGH
  DDRC |= (1 << PC7); // Set PC7 as output
  PORTC |= (1 << PC7); // Set PC7 HIGH

  //Set rows as inputs (with internal pull up resistor)
  DDRG &= ~(1 << PG1); //Set PG1 is input
  PORTG |= (1 << PG1); //Enable internal pull up resistor for PG1
  DDRL &= ~(1 << PL7); //Set PL7 is input
  PORTL |= (1 << PL7); //Enable internal pull up resistor for PL7
  DDRL &= ~(1 << PL5); //Set PL5 is input
  PORTL |= (1 << PL5); //Enable internal pull up resistor for PL5
  DDRL &= ~(1 << PL3); //Set PL3 is input
  PORTL |= (1 << PL3); //Enable internal pull up resistor for PL3
  DDRL &= ~(1 << PL1); //Set PL1 is input
  PORTL |= (1 << PL1); //Enable internal pull up resistor for PL1
}

/*Activate column (write LOW)*/
void activateColumn(int index){
  if(index == 0)
    PORTA &= ~(1 << PA0);
  else if (index == 1)
   PORTA &= ~(1 << PA2);
  else if (index == 2)
    PORTA &= ~(1 << PA4);
  else if (index == 3)
    PORTA &= ~(1 << PA6);
  else if(index == 4)
    PORTC &= ~(1 << PC7);
}

/* Dectivate column (write HIGH)*/
void deactivateColumn(int index){
  if(index == 0)
    PORTA |= (1 << PA0);
  else if (index == 1)
   PORTA |= (1 << PA2);
  else if (index == 2)
    PORTA |= (1 << PA4);
  else if (index == 3)
    PORTA |= (1 << PA6);
  else if(index == 4)
     PORTC |= (1 << PC7);
}

/* Read a row */
int readRow(int index){
  if(index == 0)
    return PING & (1 << PG1);
  else if (index == 1)
   return PINL & (1 << PL7);
  else if (index == 2)
   return  PINL & (1 << PL5);
  else if (index == 3)
   return  PINL & (1 << PL3);
  else if(index == 4)
    return PINL & (1 << PL1);
  return -1;
}

char scanKey(){
  int rKey = -1;
  int cKey = -1;  
  int c = 0;
  int r = 0;
  for(c = 0; c < numColumns(); c++){
    cKey = c;
    activateColumn(c);    
    for(r = 0; r < numRows() ; r++){
       if(readRow(r) == LOW){
         rKey = r;
       }
    }
    deactivateColumn(c);
    if(rKey != -1){
      break;
    }
  }

  if(rKey != -1 && cKey != -1){
     //Serial.print(" Row: ");
     //Serial.print(rKey);
     //Serial.print(" Col: ");
     //Serial.println(cKey);
     return keysMap[rKey][cKey];
  }
  return 0;
}


uint32_t scanKeysMask() {
  uint32_t mask = 0;

  for (int c = 0; c < numColumns(); c++) {
    activateColumn(c);

    for (int r = 0; r < numRows(); r++) {
      if (readRow(r) == LOW) {
        uint8_t idx = (uint8_t)(r * COLS + c); // row-major 0..24
        mask |= (1UL << idx);
      }
    }

    deactivateColumn(c);
  }
  return mask;
}

char keyFromIndex(uint8_t idx) {
  if (idx >= (ROWS * COLS)) return 0;
  return keysMap[idx / COLS][idx % COLS];
}


#endif