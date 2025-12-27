#pragma once
#include <Arduino.h>
#include <stdint.h>

// 5x5 key matrix -> 25 keys total
constexpr uint8_t KEYMATRIX_ROWS = 5;
constexpr uint8_t KEYMATRIX_COLS = 5;
constexpr uint8_t KEYMATRIX_NUM_KEYS = KEYMATRIX_ROWS * KEYMATRIX_COLS;

/**
 * Initializes the GPIO used by 5x5 key matrix:
 * Columns: outputs driven HIGH (inactive), driven LOW when scanning
 * Rows: inputs with internal pull-ups
 */
void setupKeyMatrixScanner();

/**
 * Scans the full matrix and returns a bitmask of pressed keys.
 * Bit i corresponds to row-major index: i = row * KEYMATRIX_COLS + col.
 * Pressed = 1.
 */
uint32_t scanKeysMask();

/**
 * Converts a row-major index (0..24) to the logical key ID ('A'..'Y').
 * Returns 0 if idx is out of range.
 */
char keyFromIndex(uint8_t idx);

/**
 * Returns the first detected pressed key ID, or 0 if none.
 * (use preferred scanKeysMask()).
 */
char scanKey();
