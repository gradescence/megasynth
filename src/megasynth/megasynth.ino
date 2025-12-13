//Mikeal Macera --- mmacera@unr.edu
//Mattia Granata --- mattiag@unr.edu
//Julian Estorga --- jestorga@unr.edu
//Cade Evans --- cadee@unr.edu
#include "KeyMatrixScanner.hpp"
#include "ToneGenerator.hpp"
#include "Time.hpp"
#include "Melodies.hpp" 
#include "Button.hpp"
#include "SerialComm.hpp"

char mmid[13] ={0x4d,0x69,0x6b,0x65,0x61,0x6c,0x20,0x4d,0x61,0x63,0x65,0x72,0x61};

struct Note {
  char key;
  int  freq;
  int  durationMilliseconds;
} Notes;
const int NOTES = 25;       
Note notes[NOTES]{
  {'A', 262, 100},
  {'B', 277, 100}, 
  {'C', 293, 100}, 
  {'D', 311, 100},   
  {'E', 329, 100},
  {'F', 349, 100},
  {'G', 369, 100},
  {'H', 392, 100},
  {'I', 415, 100},
  {'J', 440, 100},
  {'O', 466, 100},
  {'N', 494, 100},
  {'M', 523, 100},
  {'L', 554, 100},
  {'K', 587, 100},
  {'T', 622, 100},
  {'S', 659, 100},
  {'R', 698, 100},
  {'Q', 740, 100},
  {'P', 784, 100},
  {'Y', 831, 100},
  {'X', 880, 100},
  {'W', 932, 100},
  {'V', 988, 100},
  {'U', 1046, 100}
};

int freqForKey(char key) {
  for (int i = 0; i < NOTES; i++) {
    if (notes[i].key == key) return notes[i].freq;
  }
  return 0;
}


static const uint8_t NUM_KEYS = 25;

uint32_t lastRawMask = 0;
uint32_t stableMask  = 0;
unsigned long lastChangeMs[NUM_KEYS] = {0};


int demo_done = false;
char prevKey;
long prevKeyMilliseconds = 0;
long debounceKeyMilliseconds = 100;

long prevPushButtonMilliseconds = 0;
long debouncePushButtonMilliseconds = 300;

/*
* The push button will increse by one freqMultiplier. When max is raeched freqMultiplers is set to 1.
*/
const int FREQ_MULT_MAX = 6;
int freqMultiplier = 1;

void setup() {
  setupSerialComm();
  uartWriteString("Initializing time resources...");
  setupTime();
  uartWriteString("Initializing key matrix scanner...");
  setupKeyMatrixScanner();
  uartWriteString("Initializing tone generator...");
  setupToneGenerator();
  toneSetFreqMultiplier(freqMultiplier);
  setupButton();

  sleepMillis(1000);
  uartWriteString("Initialization completed!");
  for(int i=0; i<13; i++){
    //uartWriteChar(mmid[i]);
  }

}

void loop() {
  unsigned long now = getElapsedMillis();

  //Read ALL keys 
  uint32_t rawMask = scanKeysMask();

  // Per-key debounce + generate note on/off events
  for (uint8_t idx = 0; idx < NUM_KEYS; idx++) {
    uint32_t bit = (1UL << idx);

    bool rawPressed  = (rawMask & bit) != 0;
    bool prevRaw     = (lastRawMask & bit) != 0;
    bool stablePress = (stableMask & bit) != 0;

    // track when raw changed
    if (rawPressed != prevRaw) {
      lastChangeMs[idx] = now;
      if (rawPressed) lastRawMask |= bit;
      else            lastRawMask &= ~bit;
    }

    // if raw differs from stable long enough, change
    if (rawPressed != stablePress) {
      if ((now - lastChangeMs[idx]) >= (unsigned long)debounceKeyMilliseconds) {
        char key = keyFromIndex(idx);

        if (rawPressed) {
          stableMask |= bit;
          int f = freqForKey(key);
          if (f > 0) toneNoteOn(key, (unsigned int)f);
        } else {
          stableMask &= ~bit;
          toneNoteOff(key);
        }
      }
    }
  }

  // Push button (multiplier)
  if (isPushButtonPressed()) {
    unsigned long curPushButtonMilliseconds = getElapsedMillis();
    if (curPushButtonMilliseconds - prevPushButtonMilliseconds > debouncePushButtonMilliseconds) {
      prevPushButtonMilliseconds = curPushButtonMilliseconds;
      uartWriteString("Push button pressed! ");
      freqMultiplier = (freqMultiplier < FREQ_MULT_MAX) ? (freqMultiplier + 1) : 1;

      // update all active voices
      toneSetFreqMultiplier(freqMultiplier);
    }
  }
}

