#ifndef TONEGENERATOR_HPP
#define TONEGENERATOR_HPP

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/atomic.h>

#ifndef F_CPU
#define F_CPU 16000000UL
#endif


static const uint16_t PWM_TOP = 511; // 16MHz/(1*(511+1)) = 31250 Hz
static const uint32_t SAMPLE_RATE = (F_CPU / (PWM_TOP + 1));

static const uint8_t MAX_VOICES = 4; // polyphony level
static const int16_t AMP_PER_VOICE = (int16_t)((PWM_TOP / 2) / MAX_VOICES);

struct Voice {
  volatile uint8_t active; // 0/1
  volatile uint32_t phase; // DDS phase accumulator
  volatile uint32_t phaseInc; // DDS increment
  char key; // which key owns this voice
  uint16_t baseFreq; // un-multiplied frequency (for multiplier changes)
  uint32_t age;  // for voice stealing (bigger = newer)
};

static volatile uint16_t pwmMid = PWM_TOP / 2;
static volatile uint32_t gAgeCounter = 0;
static volatile uint16_t gFreqMultiplier = 1;
static Voice voices[MAX_VOICES];

static inline uint32_t calcPhaseInc(uint32_t freqHz) {
  // phaseInc = freq * 2^32 / sampleRate
  // use 64-bit to preserve precision
  uint64_t num = ((uint64_t)freqHz << 32);
  return (uint32_t)(num / (uint64_t)SAMPLE_RATE);
}

static int8_t findVoiceByKey(char key) {
  for (uint8_t i = 0; i < MAX_VOICES; i++) {
    if (voices[i].active && voices[i].key == key) return (int8_t)i;
  }
  return -1;
}

static uint8_t findFreeOrStealVoice() {
  // Prefer inactive
  for (uint8_t i = 0; i < MAX_VOICES; i++) {
    if (!voices[i].active) return i;
  }
  // Steal oldest (smallest age)
  uint8_t victim = 0;
  uint32_t bestAge = voices[0].age;
  for (uint8_t i = 1; i < MAX_VOICES; i++) {
    if (voices[i].age < bestAge) {
      bestAge = voices[i].age;
      victim = i;
    }
  }
  return victim;
}

void setupToneGenerator() {
  // OC1C output: PB7 (Arduino D13)
  DDRB |= (1 << PB7);

  // Clear voices
  for (uint8_t i = 0; i < MAX_VOICES; i++) {
    voices[i].active = 0;
    voices[i].phase = 0;
    voices[i].phaseInc = 0;
    voices[i].key = 0;
    voices[i].baseFreq = 0;
    voices[i].age = 0;
  }

  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
    // Timer1 Fast PWM, TOP=ICR1 (Mode 14: WGM13:0 = 1110)
    TCCR1A = 0;
    TCCR1B = 0;

    // Non-inverting PWM on OC1C (COM1C1=1)
    TCCR1A |= (1 << COM1C1);

    // WGM bits
    TCCR1A |= (1 << WGM11);
    TCCR1B |= (1 << WGM13) | (1 << WGM12);

    // TOP
    ICR1 = PWM_TOP;

    // Start at mid (silence)
    OCR1C = pwmMid;

    // Enable overflow interrupt (fires at PWM rate)
    TIMSK1 |= (1 << TOIE1);

    // Prescaler = 1 (CS10=1)
    TCCR1B |= (1 << CS10);
  }
}

void toneSetFreqMultiplier(unsigned int mult) {
  if (mult == 0) mult = 1;
  if (mult > 16) mult = 16; // safety clamp

  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
    gFreqMultiplier = (uint16_t)mult;
    // Update all active voices
    for (uint8_t i = 0; i < MAX_VOICES; i++) {
      if (voices[i].active) {
        uint32_t f = (uint32_t)voices[i].baseFreq * (uint32_t)gFreqMultiplier;
        voices[i].phaseInc = calcPhaseInc(f);
      }
    }
  }
}

void toneNoteOn(char key, unsigned int baseFrequency) {
  if (baseFrequency == 0) return;

  // If already active, just refresh (recompute inc/age)
  int8_t existing = findVoiceByKey(key);
  uint8_t idx = (existing >= 0) ? (uint8_t)existing : findFreeOrStealVoice();

  uint32_t f = (uint32_t)baseFrequency * (uint32_t)gFreqMultiplier;
  uint32_t inc = calcPhaseInc(f);

  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
    voices[idx].active = 0;      // prevent ISR reading half-written values
    voices[idx].key = key;
    voices[idx].baseFreq = (uint16_t)baseFrequency;
    voices[idx].phase = 0;
    voices[idx].phaseInc = inc;
    voices[idx].age = ++gAgeCounter;
    voices[idx].active = 1;
  }
}

void toneNoteOff(char key) {
  int8_t idx = findVoiceByKey(key);
  if (idx < 0) return;

  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
    voices[(uint8_t)idx].active = 0;
  }
}

void toneAllOff() {
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
    for (uint8_t i = 0; i < MAX_VOICES; i++) voices[i].active = 0;
    OCR1C = pwmMid;
  }
}

// Audio ISR: runs at SAMPLE_RATE (~31.25kHz)
ISR(TIMER1_OVF_vect) {
  int16_t mix = 0;

  for (uint8_t i = 0; i < MAX_VOICES; i++) {
    if (voices[i].active) {
      uint32_t p = voices[i].phase + voices[i].phaseInc;
      voices[i].phase = p;

      // square wave from MSB of phase
      mix += (p & 0x80000000UL) ? AMP_PER_VOICE : -AMP_PER_VOICE;
    }
  }

  int32_t duty = (int32_t)pwmMid + (int32_t)mix;
  if (duty < 0) duty = 0;
  if (duty > PWM_TOP) duty = PWM_TOP;

  OCR1C = (uint16_t)duty;
}

#endif
