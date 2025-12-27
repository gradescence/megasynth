#include "ToneGenerator.hpp"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/atomic.h>

#ifndef F_CPU
#define F_CPU 16000000UL
#endif

namespace {

constexpr uint16_t kPwmTop = 511;  // 16MHz / (1*(511+1)) = 31250 Hz
constexpr uint32_t kSampleRate = (F_CPU / (uint32_t)(kPwmTop + 1));

constexpr uint8_t kMaxVoices = 4;  // polyphony level
constexpr int16_t kAmpPerVoice = (int16_t)((kPwmTop / 2) / kMaxVoices);

struct Voice {
  volatile uint8_t active;   // 0/1
  volatile uint32_t phase;   // DDS phase accumulator
  volatile uint32_t phaseInc;  // DDS increment
  char key;                  // which key owns this voice
  uint16_t baseFreq;         // un-multiplied frequency
  uint32_t age;              // for voice stealing (bigger -> newer)
};

volatile uint16_t s_pwmMid = kPwmTop / 2;
volatile uint32_t s_ageCounter = 0;
volatile uint16_t s_freqMultiplier = 1;

Voice s_voices[kMaxVoices];

inline uint32_t calcPhaseInc(uint32_t freqHz) {
  // phaseInc = freq * 2^32 / sampleRate (use 64-bit to preserve precision)
  const uint64_t num = ((uint64_t)freqHz << 32);
  return (uint32_t)(num / (uint64_t)kSampleRate);
}

int8_t findVoiceByKey(char key) {
  for (uint8_t i = 0; i < kMaxVoices; i++) {
    if (s_voices[i].active && s_voices[i].key == key) return (int8_t)i;
  }
  return -1;
}

uint8_t findFreeOrStealVoice() {
  // Prefer inactive
  for (uint8_t i = 0; i < kMaxVoices; i++) {
    if (!s_voices[i].active) return i;
  }
  // Steal oldest (smallest age)
  uint8_t victim = 0;
  uint32_t bestAge = s_voices[0].age;
  for (uint8_t i = 1; i < kMaxVoices; i++) {
    if (s_voices[i].age < bestAge) {
      bestAge = s_voices[i].age;
      victim = i;
    }
  }
  return victim;
}

}  // namespace

void setupToneGenerator() {
  // OC1C output: PB7 (Arduino Mega D13)
  DDRB |= _BV(PB7);

  // Clear voices
  for (uint8_t i = 0; i < kMaxVoices; i++) {
    s_voices[i].active = 0;
    s_voices[i].phase = 0;
    s_voices[i].phaseInc = 0;
    s_voices[i].key = 0;
    s_voices[i].baseFreq = 0;
    s_voices[i].age = 0;
  }

  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
    // Timer1 Fast PWM, TOP = ICR1 (Mode 14: WGM13:0 = 1110)
    TCCR1A = 0;
    TCCR1B = 0;

    // Non-inverting PWM on OC1C (COM1C1=1)
    TCCR1A |= _BV(COM1C1);

    // WGM bits
    TCCR1A |= _BV(WGM11);
    TCCR1B |= _BV(WGM13) | _BV(WGM12);

    // TOP and initial duty (silence)
    ICR1 = kPwmTop;
    OCR1C = s_pwmMid;

    // Enable overflow interrupt (fires at PWM rate)
    TIMSK1 |= _BV(TOIE1);

    // Prescaler = 1 (CS10=1)
    TCCR1B |= _BV(CS10);
  }
}

void toneSetFreqMultiplier(uint16_t mult) {
  if (mult == 0) mult = 1;
  if (mult > 16) mult = 16;  // safety

  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
    s_freqMultiplier = mult;
    for (uint8_t i = 0; i < kMaxVoices; i++) {
      if (s_voices[i].active) {
        const uint32_t f = (uint32_t)s_voices[i].baseFreq * (uint32_t)s_freqMultiplier;
        s_voices[i].phaseInc = calcPhaseInc(f);
      }
    }
  }
}

void toneNoteOn(char key, uint16_t baseFrequency) {
  if (baseFrequency == 0) return;

  const int8_t existing = findVoiceByKey(key);
  const uint8_t idx = (existing >= 0) ? (uint8_t)existing : findFreeOrStealVoice();

  const uint32_t f = (uint32_t)baseFrequency * (uint32_t)s_freqMultiplier;
  const uint32_t inc = calcPhaseInc(f);

  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
    // Prevent ISR from reading half-written values
    s_voices[idx].active = 0;
    s_voices[idx].key = key;
    s_voices[idx].baseFreq = baseFrequency;
    s_voices[idx].phase = 0;
    s_voices[idx].phaseInc = inc;
    s_voices[idx].age = s_ageCounter++;
    s_voices[idx].active = 1;
  }
}

void toneNoteOff(char key) {
  const int8_t idx = findVoiceByKey(key);
  if (idx < 0) return;

  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) { s_voices[(uint8_t)idx].active = 0; }
}

void toneAllOff() {
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
    for (uint8_t i = 0; i < kMaxVoices; i++) s_voices[i].active = 0;
    OCR1C = s_pwmMid;
  }
}

// Audio ISR runs at kSampleRate (about 31.25 kHz)
ISR(TIMER1_OVF_vect) {
  int16_t mix = 0;

  for (uint8_t i = 0; i < kMaxVoices; i++) {
    if (s_voices[i].active) {
      const uint32_t p = s_voices[i].phase + s_voices[i].phaseInc;
      s_voices[i].phase = p;

      // Square wave from MSB of phase
      mix += (p & 0x80000000UL) ? kAmpPerVoice : -kAmpPerVoice;
    }
  }

  int32_t duty = (int32_t)s_pwmMid + (int32_t)mix;
  if (duty < 0) duty = 0;
  if (duty > kPwmTop) duty = kPwmTop;

  OCR1C = (uint16_t)duty;
}
