#pragma once
#include <Arduino.h>

/**
 * Initializes Timer2 to generate a 1ms tick for getElapsedMillis().
 */
void setupTime();

/**
 * Milliseconds since setupTime() was called.
 */
unsigned long getElapsedMillis();

/**
 * Blocking delay implemented on top of getElapsedMillis().
 * Interrupts remain enabled so audio continues to run.
 */
void sleepMillis(unsigned long milliseconds);
