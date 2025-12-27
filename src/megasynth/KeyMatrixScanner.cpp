#include "KeyMatrixScanner.hpp"

#include <avr/io.h>

namespace {

struct OutPin {
  volatile uint8_t* ddr;
  volatile uint8_t* port;
  uint8_t mask;
};

struct InPin {
  volatile uint8_t* ddr;
  volatile uint8_t* port;
  volatile uint8_t* pin;
  uint8_t mask;
};

// Cols: PA0, PA2, PA4, PA6, PC7 (active LOW while scanning)
const OutPin kCols[KEYMATRIX_COLS] = {
    {&DDRA, &PORTA, _BV(PA0)},
    {&DDRA, &PORTA, _BV(PA2)},
    {&DDRA, &PORTA, _BV(PA4)},
    {&DDRA, &PORTA, _BV(PA6)},
    {&DDRC, &PORTC, _BV(PC7)},
};

// Rows: PG1, PL7, PL5, PL3, PL1 (inputs with pull-ups)
const InPin kRows[KEYMATRIX_ROWS] = {
    {&DDRG, &PORTG, &PING, _BV(PG1)},
    {&DDRL, &PORTL, &PINL, _BV(PL7)},
    {&DDRL, &PORTL, &PINL, _BV(PL5)},
    {&DDRL, &PORTL, &PINL, _BV(PL3)},
    {&DDRL, &PORTL, &PINL, _BV(PL1)},
};

const char kKeyMap[KEYMATRIX_ROWS][KEYMATRIX_COLS] = {
    {'A', 'B', 'C', 'D', 'E'},
    {'F', 'G', 'H', 'I', 'J'},
    {'K', 'L', 'M', 'N', 'O'},
    {'P', 'Q', 'R', 'S', 'T'},
    {'U', 'V', 'W', 'X', 'Y'},
};

inline void colHigh(uint8_t col) { *kCols[col].port |= kCols[col].mask; }
inline void colLow(uint8_t col) { *kCols[col].port &= (uint8_t)~kCols[col].mask; }

inline bool rowPressed(uint8_t row) {
  // With pull-ups enabled, pressed == LOW (0)
  return ((*kRows[row].pin & kRows[row].mask) == 0);
}

}  // namspace

void setupKeyMatrixScanner() {
  // Cols as outputs, idle HIGH
  for (uint8_t c = 0; c < KEYMATRIX_COLS; c++) {
    *kCols[c].ddr |= kCols[c].mask;
    colHigh(c);
  }

  // Rows as inputs with internal pull-ups
  for (uint8_t r = 0; r < KEYMATRIX_ROWS; r++) {
    *kRows[r].ddr &= (uint8_t)~kRows[r].mask;
    *kRows[r].port |= kRows[r].mask;
  }
}

uint32_t scanKeysMask() {
  uint32_t mask = 0;

  for (uint8_t c = 0; c < KEYMATRIX_COLS; c++) {
    colLow(c);

    // Small settling time (a few cycles)
    asm volatile("nop\n\t" "nop\n\t" "nop\n\t");

    for (uint8_t r = 0; r < KEYMATRIX_ROWS; r++) {
      if (rowPressed(r)) {
        const uint8_t idx = (uint8_t)(r * KEYMATRIX_COLS + c);  // 0..24
        mask |= (1UL << idx);
      }
    }

    colHigh(c);
  }

  return mask;
}

char keyFromIndex(uint8_t idx) {
  if (idx >= KEYMATRIX_NUM_KEYS) return 0;
  return kKeyMap[idx / KEYMATRIX_COLS][idx % KEYMATRIX_COLS];
}

char scanKey() {
  uint32_t mask = scanKeysMask();
  if (!mask) return 0;

  // Return the first key in proper order
  for (uint8_t idx = 0; idx < KEYMATRIX_NUM_KEYS; idx++) {
    if (mask & (1UL << idx)) return keyFromIndex(idx);
  }
  return 0;
}
