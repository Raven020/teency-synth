#ifndef AUDIO_DESIGN_H
#define AUDIO_DESIGN_H

// audio_design.h — extern declarations for all Teensy Audio objects.
//
// The actual object DEFINITIONS (and all AudioConnection patch cords) live in
// teency_synth.ino.  Including this header in voices.cpp lets that translation
// unit reference the objects without causing multiple-definition errors.

#include <Audio.h>
#include <Wire.h>

// ── Per-voice audio objects (6 voices) ───────────────────────────────────────
extern AudioSynthWaveformModulated osc1[6];   // primary oscillator
extern AudioSynthWaveformModulated osc2[6];   // detuned secondary oscillator
extern AudioMixer4                 oscMix[6]; // blend osc1 + osc2
extern AudioFilterStateVariable    vcf[6];          // lowpass / bandpass / highpass
extern AudioEffectEnvelope         ampEnv[6];        // VCA envelope (ADSR)
extern AudioSynthWaveformDc        filterEnvDC[6];   // DC source for filter envelope
extern AudioEffectEnvelope         filterEnv[6];     // ADSR shaper → VCF freq mod input

// ── Mixer tree ────────────────────────────────────────────────────────────────
extern AudioMixer4          subMixerA;    // voices 0–3
extern AudioMixer4          subMixerB;    // voices 4–5
extern AudioMixer4          masterMixer;  // subMixerA + subMixerB → output

// ── Output ────────────────────────────────────────────────────────────────────
extern AudioOutputI2S        i2s1;
extern AudioControlSGTL5000  audioShield;

#endif // AUDIO_DESIGN_H
