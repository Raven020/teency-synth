// voices.cpp — voice allocation and note-trigger implementation.

#include <Audio.h>       // must come before audio_design.h
#include <Wire.h>
#include <math.h>

#include "audio_design.h"   // extern audio object declarations
#include "synth_params.h"   // OSC1/2 levels, detune, ADSR defaults
#include "voices.h"

// ── Global voice state ────────────────────────────────────────────────────────
Voice voices[NUM_VOICES];

// ── Frequency conversion ──────────────────────────────────────────────────────
float noteToFrequency(byte midiNote) {
    return 440.0f * powf(2.0f, (midiNote - 69) / 12.0f);
}

// Helper: apply cent offset to a base frequency.
static inline float centOffset(float baseHz, float cents) {
    return baseHz * powf(2.0f, cents / 1200.0f);
}

// ── Allocation helpers ────────────────────────────────────────────────────────
int findFreeVoice() {
    for (int i = 0; i < NUM_VOICES; i++) {
        if (!voices[i].active) return i;
    }
    return -1;
}

int findVoiceByNote(byte note) {
    for (int i = 0; i < NUM_VOICES; i++) {
        if (voices[i].active && voices[i].midiNote == note) return i;
    }
    return -1;
}

int stealOldestVoice() {
    int      oldest   = 0;
    uint32_t earliest = voices[0].noteOnTime;
    for (int i = 1; i < NUM_VOICES; i++) {
        if (voices[i].noteOnTime < earliest) {
            earliest = voices[i].noteOnTime;
            oldest   = i;
        }
    }
    return oldest;
}

// ── Note on ───────────────────────────────────────────────────────────────────
void voiceNoteOn(byte note, byte velocity) {
    // Allocate a voice — steal oldest if all are busy
    int vi = findFreeVoice();
    if (vi < 0) {
        vi = stealOldestVoice();
        // Silence the stolen voice immediately so it doesn't ring on
        ampEnv[vi].noteOff();
        filterEnv[vi].noteOff();
    }

    // Update voice state
    voices[vi].active     = true;
    voices[vi].midiNote   = note;
    voices[vi].velocity   = velocity;
    voices[vi].noteOnTime = millis();

    // Compute frequencies
    float baseFreq   = noteToFrequency(note);
    float detuneFreq = centOffset(baseFreq, gDetuneCents);

    // Configure oscillators
    osc1[vi].frequency(baseFreq);
    osc1[vi].amplitude(1.0f);
    osc2[vi].frequency(detuneFreq);
    osc2[vi].amplitude(1.0f);

    // Apply velocity scaling through the osc mixer gains.
    // Velocity 0–127 → 0.0–1.0 times the base OSC levels.
    float velScale = velocity / 127.0f;
    oscMix[vi].gain(0, OSC1_LEVEL * velScale);
    oscMix[vi].gain(1, OSC2_LEVEL * velScale);

    // Trigger amplitude and filter envelopes
    ampEnv[vi].noteOn();
    filterEnv[vi].noteOn();
}

// ── Note off ──────────────────────────────────────────────────────────────────
void voiceNoteOff(byte note) {
    int vi = findVoiceByNote(note);
    if (vi < 0) return;   // note not found — ignore stray note-off

    voices[vi].active = false;
    ampEnv[vi].noteOff();
    filterEnv[vi].noteOff();
}
