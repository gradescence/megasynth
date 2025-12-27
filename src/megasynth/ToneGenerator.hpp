#pragma once
#include <stdint.h>

void setupToneGenerator();

// Start/stop a note by key ID (so we can release the right voice)
void toneNoteOn(char key, uint16_t baseFrequency);
void toneNoteOff(char key);

// Change multiplier (updates all active voices)
void toneSetFreqMultiplier(uint16_t mult);

void toneAllOff();
