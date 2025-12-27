#pragma once
#include <Arduino.h>
#include <stdbool.h>

/**
 * Configures the push button on PE4 (Arduino Mega D2 / INT4).
 * The button is expected to pull the pin LOW when pressed (internal pull-up enabled).
 */
void setupButton();

/**
 * Returns true exactly once per physical press (edge), otherwise false.
 * This function is ISR-safe.
 */
bool isPushButtonPressed();
