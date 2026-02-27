#ifndef SYNTH_PARAMS_H
#define SYNTH_PARAMS_H

// synth_params.h — global synthesizer parameter defaults
// Include Audio.h before this file so WAVEFORM_* constants are available.

// ── Oscillator ────────────────────────────────────────────────────────────────
#define OSC1_WAVEFORM      WAVEFORM_SAWTOOTH
#define OSC2_WAVEFORM      WAVEFORM_SAWTOOTH
constexpr float OSC2_DETUNE_CENTS = 7.0f;   // osc2 pitched +7 cents sharp

// Osc mixer blend (each osc 50%); velocity is applied on top of these
constexpr float OSC1_LEVEL = 0.5f;
constexpr float OSC2_LEVEL = 0.5f;

// ── Filter ────────────────────────────────────────────────────────────────────
constexpr float FILTER_CUTOFF_HZ  = 2000.0f;
constexpr float FILTER_RESONANCE  = 0.7f;   // 0.7 (flat) – 5.0 (self-osc)

// ── Amp ADSR ──────────────────────────────────────────────────────────────────
constexpr float AMP_ATTACK_MS  =   5.0f;
constexpr float AMP_DECAY_MS   = 100.0f;
constexpr float AMP_SUSTAIN    =   0.8f;   // 0.0 – 1.0
constexpr float AMP_RELEASE_MS = 200.0f;

// ── Levels ────────────────────────────────────────────────────────────────────
constexpr float MASTER_VOLUME = 0.8f;    // SGTL5000 headphone / line volume
constexpr float VOICE_LEVEL   = 0.25f;  // per-voice gain into sub-mixers

// ── CC mappings (Arturia KeyStep defaults) ────────────────────────────────────
constexpr int CC_FILTER_CUTOFF    = 74;  // Brightness → filter cutoff
constexpr int CC_FILTER_RESONANCE = 71;  // Resonance  → filter Q
constexpr int CC_MOD_WHEEL        =  1;  // Mod wheel  → vibrato depth

// ── Pitch-wheel detune range ──────────────────────────────────────────────────
// Pitch wheel controls osc2 detune widening.
// At pitch min: osc2 is OSC2_DETUNE_CENTS sharp.
// At pitch max: osc2 is (OSC2_DETUNE_CENTS + MOD_DETUNE_MAX) sharp.
constexpr float MOD_DETUNE_MAX = 50.0f;

// ── Vibrato LFO (mod wheel) ───────────────────────────────────────────────────
constexpr float VIB_RATE_HZ     =  5.5f;  // LFO speed in Hz
constexpr float VIB_DEPTH_CENTS = 25.0f;  // max pitch swing at full mod wheel

// ── Filter envelope ───────────────────────────────────────────────────────────
// A separate ADSR that sweeps the filter cutoff on every note.
// Sustain is hardcoded to 0 (decay/contour style — filter opens then closes).
// Release tracks the amp release pot so both envelopes tail off together.
constexpr float FILTER_ENV_AMOUNT    = 0.3f;   // 0.0 (off) – 1.0 (4-octave sweep)
constexpr float FILTER_ENV_ATTACK_MS =   5.0f;
constexpr float FILTER_ENV_DECAY_MS  = 500.0f;
constexpr float FILTER_ENV_SUSTAIN   =   0.0f; // hardcoded — do not expose as pot

// ── Potentiometer pin assignments ─────────────────────────────────────────────
// These pins are safe when Audio Board Rev D is stacked.
// Avoid: A4/pin18 (I2C SDA), A5/pin19 (I2C SCL),
//        A6/pin20 (I2S LRCLK), A7/pin21 (I2S BCLK), A9/pin23 (I2S MCLK).
constexpr int POT_CUTOFF    = A0;   // pin 14
constexpr int POT_RESONANCE = A1;   // pin 15
constexpr int POT_ATTACK    = A2;   // pin 16
constexpr int POT_DECAY     = A3;   // pin 17
constexpr int POT_SUSTAIN   = A8;   // pin 22
constexpr int POT_RELEASE   = A10;  // pin 24

// Filter envelope pots — inner-row pins, safe with Audio Board stacked
constexpr int POT_FENV_AMOUNT = A11;  // pin 25 — how far the filter sweeps
constexpr int POT_FENV_ATTACK = A12;  // pin 26 — filter env attack time
constexpr int POT_FENV_DECAY  = A13;  // pin 27 — filter env decay time

// Ignore ADC changes smaller than this to suppress noise jitter (0–1023 scale)
constexpr int POT_DEADBAND = 4;

// ── Filter env pot output ranges ──────────────────────────────────────────────
constexpr float POT_FENV_ATTACK_MIN_MS =    1.0f;
constexpr float POT_FENV_ATTACK_MAX_MS = 2000.0f;
constexpr float POT_FENV_DECAY_MIN_MS  =    5.0f;
constexpr float POT_FENV_DECAY_MAX_MS  = 4000.0f;

// ── ADSR pot output ranges ────────────────────────────────────────────────────
constexpr float POT_ATTACK_MIN_MS  =    1.0f;
constexpr float POT_ATTACK_MAX_MS  = 2000.0f;
constexpr float POT_DECAY_MIN_MS   =    5.0f;
constexpr float POT_DECAY_MAX_MS   = 2000.0f;
constexpr float POT_RELEASE_MIN_MS =   10.0f;
constexpr float POT_RELEASE_MAX_MS = 4000.0f;

// ── Sample rate ───────────────────────────────────────────────────────────────
constexpr int SAMPLE_RATE = 44100;

#endif // SYNTH_PARAMS_H
