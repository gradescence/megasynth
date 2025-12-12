void setupToneGenerator();
// Start/stop a note by key ID (so we can release the right voice)
void toneNoteOn(char key, unsigned int baseFrequency);
void toneNoteOff(char key);
// Change multiplier (updates all active voices)
void toneSetFreqMultiplier(unsigned int mult);
void toneAllOff();
