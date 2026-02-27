#ifndef VOICES_H
#define VOICES_H

// voices.h — voice allocation engine for 6-voice polyphony.

#include <Arduino.h>

constexpr int NUM_VOICES = 6;

// ── Voice state ───────────────────────────────────────────────────────────────
struct Voice {
    bool     active;
    byte     midiNote;
    byte     velocity;
    uint32_t noteOnTime;  // millis() timestamp — used for oldest-note stealing
};

extern Voice voices[NUM_VOICES];
extern float gDetuneCents;  // current osc2 detune — controlled by pitch wheel

// ── Allocation helpers ────────────────────────────────────────────────────────
// Returns index of first inactive voice, or -1 if all voices are active.
int findFreeVoice();

// Returns index of active voice playing `note`, or -1 if not found.
int findVoiceByNote(byte note);

// Returns index of the voice with the earliest noteOnTime (steal candidate).
int stealOldestVoice();

// ── Frequency conversion ──────────────────────────────────────────────────────
// Standard equal-temperament MIDI note → Hz  (A4 = 69 = 440 Hz).
float noteToFrequency(byte midiNote);

// ── High-level note triggers ──────────────────────────────────────────────────
// Called from MIDI callbacks in teency_synth.ino.
void voiceNoteOn(byte note, byte velocity);
void voiceNoteOff(byte note);

#endif // VOICES_H
