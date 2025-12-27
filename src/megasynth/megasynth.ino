/*
Polyphonic Synthesizer Project for Introduction to Embedded Systems (Comp. Eng. 301)
mmacera(gradescent)[1][2], cadee[2], mattiag[2], jestorga[2]
[1]: Carnegie Mellon University
[2]: University of Nevada-Reno
*/
#include "KeyMatrixScanner.hpp"
#include "ToneGenerator.hpp"
#include "Time.hpp"
#include "Button.hpp"
#include "SerialComm.hpp"

// Index mapping corresponds to KeyMatrixScanner's kKeyMap:
//   A B C D E
//   F G H I J
//   K L M N O
//   P Q R S T
//   U V W X Y
static constexpr uint8_t kNumKeys = KEYMATRIX_NUM_KEYS;
static constexpr uint16_t kFreqByIndex[kNumKeys] = {
    262, 277, 293, 311, 329,  // A..E
    349, 369, 392, 415, 440,  // F..J
    587, 554, 523, 494, 466,  // K..O 
    784, 740, 698, 659, 622,  // P..T 
    1046, 988, 932, 880, 831  // U..Y 
};

// Debounce / state
static constexpr unsigned long kKeyDebounceMs = 50;      
static constexpr unsigned long kButtonDebounceMs = 150;  

static uint32_t g_prevRawMask = 0;
static uint32_t g_stableMask = 0;
static unsigned long g_lastChangeMs[kNumKeys] = {0};

static unsigned long g_lastButtonMs = 0;

// Multiplier (push button cycles) 
static constexpr uint8_t kFreqMultMax = 6;
static uint8_t g_freqMultiplier = 1;

void setup() {
  setupSerialComm();
  uartWriteString("Initializing time resources...");
  setupTime();

  uartWriteString("Initializing key matrix scanner...");
  setupKeyMatrixScanner();

  uartWriteString("Initializing tone generator...");
  setupToneGenerator();
  toneSetFreqMultiplier(g_freqMultiplier);

  setupButton();

  sleepMillis(250);
  uartWriteString("Initialization completed!");
}

static void handleKeys(uint32_t rawMask, unsigned long nowMs) {
  // Update per-key "last change" timestamps for bits that changed since last scan.
  const uint32_t changes = rawMask ^ g_prevRawMask;
  if (changes) {
    for (uint8_t idx = 0; idx < kNumKeys; idx++) {
      const uint32_t bit = (1UL << idx);
      if (changes & bit) g_lastChangeMs[idx] = nowMs;
    }
    g_prevRawMask = rawMask;
  }

  // commits the raw to stable when it has stayed different long enough
  for (uint8_t idx = 0; idx < kNumKeys; idx++) {
    const uint32_t bit = (1UL << idx);

    const bool rawPressed = (rawMask & bit) != 0;
    const bool stablePressed = (g_stableMask & bit) != 0;

    if (rawPressed == stablePressed) continue;
    if ((unsigned long)(nowMs - g_lastChangeMs[idx]) < kKeyDebounceMs) continue;

    const char key = keyFromIndex(idx);
    if (!key) continue;

    if (rawPressed) {
      g_stableMask |= bit;
      toneNoteOn(key, kFreqByIndex[idx]);
    } else {
      g_stableMask &= ~bit;
      toneNoteOff(key);
    }
  }
}

static void handleButton(unsigned long nowMs) {
  if (!isPushButtonPressed()) return;

  if ((unsigned long)(nowMs - g_lastButtonMs) < kButtonDebounceMs) return;
  g_lastButtonMs = nowMs;

  g_freqMultiplier = (g_freqMultiplier < kFreqMultMax) ? (g_freqMultiplier + 1) : 1;
  toneSetFreqMultiplier(g_freqMultiplier);

  uartWriteString("Push button pressed (multiplier updated).");
}

void loop() {
  const unsigned long nowMs = getElapsedMillis();

  const uint32_t rawMask = scanKeysMask();
  handleKeys(rawMask, nowMs);

  handleButton(nowMs);
}
