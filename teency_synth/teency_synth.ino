// teency_synth.ino — 6-Voice Polyphonic Subtractive Synthesizer
//
// Hardware:  Teensy 4.1 + Audio Board Rev D + Arturia KeyStep (USB Host MIDI)
// Output:    SGTL5000 → 3.5 mm headphone / line jack
// Board:     Teensy 4.1
// USB Type:  Serial + MIDI  (or MIDI)
// CPU Speed: 600 MHz

#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>
#include <USBHost_t36.h>

#include "audio_design.h"   // extern declarations (types required for objects below)
#include "synth_params.h"
#include "voices.h"

// =============================================================================
// AUDIO OBJECT DEFINITIONS
// All AudioConnection patch cords must be global (static lifetime).
// =============================================================================

// ── Per-voice audio objects ───────────────────────────────────────────────────
AudioSynthWaveformModulated osc1[6];
AudioSynthWaveformModulated osc2[6];
AudioMixer4                 oscMix[6];
AudioFilterStateVariable    vcf[6];
AudioEffectEnvelope         ampEnv[6];
AudioSynthWaveformDc        filterEnvDC[6];  // constant signal shaped by filterEnv
AudioEffectEnvelope         filterEnv[6];    // sweeps VCF cutoff on each note

// ── Mixer tree ────────────────────────────────────────────────────────────────
AudioMixer4          subMixerA;    // voices 0–3
AudioMixer4          subMixerB;    // voices 4–5
AudioMixer4          masterMixer;  // subMixerA + subMixerB → I2S

// ── Output ────────────────────────────────────────────────────────────────────
AudioOutputI2S       i2s1;
AudioControlSGTL5000 audioShield;

// ── Patch cords: osc1 + osc2 → osc mixer ─────────────────────────────────────
AudioConnection pc_osc1_0(osc1[0], 0, oscMix[0], 0);
AudioConnection pc_osc2_0(osc2[0], 0, oscMix[0], 1);
AudioConnection pc_osc1_1(osc1[1], 0, oscMix[1], 0);
AudioConnection pc_osc2_1(osc2[1], 0, oscMix[1], 1);
AudioConnection pc_osc1_2(osc1[2], 0, oscMix[2], 0);
AudioConnection pc_osc2_2(osc2[2], 0, oscMix[2], 1);
AudioConnection pc_osc1_3(osc1[3], 0, oscMix[3], 0);
AudioConnection pc_osc2_3(osc2[3], 0, oscMix[3], 1);
AudioConnection pc_osc1_4(osc1[4], 0, oscMix[4], 0);
AudioConnection pc_osc2_4(osc2[4], 0, oscMix[4], 1);
AudioConnection pc_osc1_5(osc1[5], 0, oscMix[5], 0);
AudioConnection pc_osc2_5(osc2[5], 0, oscMix[5], 1);

// ── Patch cords: osc mixer → VCF ─────────────────────────────────────────────
AudioConnection pc_mix_vcf0(oscMix[0], 0, vcf[0], 0);
AudioConnection pc_mix_vcf1(oscMix[1], 0, vcf[1], 0);
AudioConnection pc_mix_vcf2(oscMix[2], 0, vcf[2], 0);
AudioConnection pc_mix_vcf3(oscMix[3], 0, vcf[3], 0);
AudioConnection pc_mix_vcf4(oscMix[4], 0, vcf[4], 0);
AudioConnection pc_mix_vcf5(oscMix[5], 0, vcf[5], 0);

// ── Patch cords: VCF lowpass output (0) → amp envelope ───────────────────────
AudioConnection pc_vcf_env0(vcf[0], 0, ampEnv[0], 0);
AudioConnection pc_vcf_env1(vcf[1], 0, ampEnv[1], 0);
AudioConnection pc_vcf_env2(vcf[2], 0, ampEnv[2], 0);
AudioConnection pc_vcf_env3(vcf[3], 0, ampEnv[3], 0);
AudioConnection pc_vcf_env4(vcf[4], 0, ampEnv[4], 0);
AudioConnection pc_vcf_env5(vcf[5], 0, ampEnv[5], 0);

// ── Patch cords: filter env DC → filter envelope → VCF frequency mod input ────
// VCF input 1 is the frequency modulation input; octaveControl() sets the range.
AudioConnection pc_fenvdc0(filterEnvDC[0], 0, filterEnv[0], 0);
AudioConnection pc_fenvdc1(filterEnvDC[1], 0, filterEnv[1], 0);
AudioConnection pc_fenvdc2(filterEnvDC[2], 0, filterEnv[2], 0);
AudioConnection pc_fenvdc3(filterEnvDC[3], 0, filterEnv[3], 0);
AudioConnection pc_fenvdc4(filterEnvDC[4], 0, filterEnv[4], 0);
AudioConnection pc_fenvdc5(filterEnvDC[5], 0, filterEnv[5], 0);
AudioConnection pc_fenv_vcf0(filterEnv[0], 0, vcf[0], 1);
AudioConnection pc_fenv_vcf1(filterEnv[1], 0, vcf[1], 1);
AudioConnection pc_fenv_vcf2(filterEnv[2], 0, vcf[2], 1);
AudioConnection pc_fenv_vcf3(filterEnv[3], 0, vcf[3], 1);
AudioConnection pc_fenv_vcf4(filterEnv[4], 0, vcf[4], 1);
AudioConnection pc_fenv_vcf5(filterEnv[5], 0, vcf[5], 1);

// ── Patch cords: amp envelopes → sub-mixers ───────────────────────────────────
AudioConnection pc_env_subA0(ampEnv[0], 0, subMixerA, 0);
AudioConnection pc_env_subA1(ampEnv[1], 0, subMixerA, 1);
AudioConnection pc_env_subA2(ampEnv[2], 0, subMixerA, 2);
AudioConnection pc_env_subA3(ampEnv[3], 0, subMixerA, 3);
AudioConnection pc_env_subB0(ampEnv[4], 0, subMixerB, 0);
AudioConnection pc_env_subB1(ampEnv[5], 0, subMixerB, 1);

// ── Patch cords: sub-mixers → master mixer ────────────────────────────────────
AudioConnection pc_subA_master(subMixerA, 0, masterMixer, 0);
AudioConnection pc_subB_master(subMixerB, 0, masterMixer, 1);

// ── Patch cords: master mixer → I2S (mono signal to both channels) ────────────
AudioConnection pc_master_L(masterMixer, 0, i2s1, 0);  // left
AudioConnection pc_master_R(masterMixer, 0, i2s1, 1);  // right

// =============================================================================
// USB HOST MIDI
// =============================================================================
USBHost    myusb;
USBHub     hub1(myusb);
MIDIDevice midi1(myusb);

// =============================================================================
// GLOBAL SYNTH STATE
// =============================================================================
float gFilterCutoff    = FILTER_CUTOFF_HZ;
float gFilterResonance = FILTER_RESONANCE;
float gModWheel        = 0.0f;   // 0.0 – 1.0  (vibrato depth)
float gDetuneCents     = OSC2_DETUNE_CENTS;  // controlled by pitch wheel
float gLfoPhase        = 0.0f;   // vibrato LFO phase (radians)
int   gWaveform        = OSC1_WAVEFORM;      // current oscillator waveform

// Live ADSR values — updated by pots, applied to all voices
float gAttackMs  = AMP_ATTACK_MS;
float gDecayMs   = AMP_DECAY_MS;
float gSustain   = AMP_SUSTAIN;
float gReleaseMs = AMP_RELEASE_MS;

// Filter envelope — updated by pots
float gFilterEnvAmount   = FILTER_ENV_AMOUNT;
float gFilterEnvAttackMs = FILTER_ENV_ATTACK_MS;
float gFilterEnvDecayMs  = FILTER_ENV_DECAY_MS;

// =============================================================================
// FORWARD DECLARATIONS
// =============================================================================
void onNoteOn(byte channel, byte note, byte velocity);
void onNoteOff(byte channel, byte note, byte velocity);
void onCC(byte channel, byte control, byte value);
void onPitchChange(byte channel, int value);
void applyFilterToAllVoices();
void applyADSRToAllVoices();
void applyFilterEnvToAllVoices();
void readPotentiometers();
void readWaveformSwitch();

// =============================================================================
// SETUP
// =============================================================================
void setup() {
    Serial.begin(115200);

    // Allocate audio memory blocks. 80 is comfortable for 6 voices.
    AudioMemory(100);  // increased for 6 × filterEnvDC + filterEnv objects

    // ── Audio shield initialisation ───────────────────────────────────────────
    audioShield.enable();
    audioShield.volume(MASTER_VOLUME);      // headphone output volume
    audioShield.lineOutLevel(13);           // 3.16 V p-p line output

    // ── Per-voice initialisation ──────────────────────────────────────────────
    for (int i = 0; i < NUM_VOICES; i++) {
        // Oscillators
        osc1[i].begin(OSC1_WAVEFORM);
        osc1[i].amplitude(1.0f);
        osc1[i].frequency(440.0f);

        osc2[i].begin(OSC2_WAVEFORM);
        osc2[i].amplitude(1.0f);
        osc2[i].frequency(440.0f);

        // Osc mixer — equal blend; channels 2 & 3 unused
        oscMix[i].gain(0, OSC1_LEVEL);
        oscMix[i].gain(1, OSC2_LEVEL);
        oscMix[i].gain(2, 0.0f);
        oscMix[i].gain(3, 0.0f);

        // Filter: lowpass, frequency and resonance from defaults
        vcf[i].frequency(FILTER_CUTOFF_HZ);
        vcf[i].resonance(FILTER_RESONANCE);
        vcf[i].octaveControl(1.0f);   // allow 1-octave frequency modulation range

        // Amp ADSR
        ampEnv[i].attack(AMP_ATTACK_MS);
        ampEnv[i].decay(AMP_DECAY_MS);
        ampEnv[i].sustain(AMP_SUSTAIN);
        ampEnv[i].release(AMP_RELEASE_MS);

        // Filter envelope — DC source sets the peak amplitude (= env amount)
        filterEnvDC[i].amplitude(FILTER_ENV_AMOUNT);
        filterEnv[i].attack(FILTER_ENV_ATTACK_MS);
        filterEnv[i].decay(FILTER_ENV_DECAY_MS);
        filterEnv[i].sustain(FILTER_ENV_SUSTAIN);  // 0.0 — decay/contour style
        filterEnv[i].release(AMP_RELEASE_MS);      // tracks amp release

        // Voice struct
        voices[i].active     = false;
        voices[i].midiNote   = 0;
        voices[i].velocity   = 0;
        voices[i].noteOnTime = 0;
    }

    // ── Sub-mixer levels: each voice reduced to avoid clipping ────────────────
    for (int ch = 0; ch < 4; ch++) subMixerA.gain(ch, VOICE_LEVEL);
    for (int ch = 0; ch < 4; ch++) subMixerB.gain(ch, VOICE_LEVEL);

    // ── Master mixer: two sub-mixers into a single mono bus ───────────────────
    masterMixer.gain(0, 1.0f);
    masterMixer.gain(1, 1.0f);
    masterMixer.gain(2, 0.0f);
    masterMixer.gain(3, 0.0f);

    // ── Waveform selector switch ──────────────────────────────────────────────
    for (int i = 0; i < 8; i++) pinMode(WAVE_SW_PINS[i], INPUT_PULLUP);

    // ── USB Host MIDI ─────────────────────────────────────────────────────────
    myusb.begin();
    midi1.setHandleNoteOn(onNoteOn);
    midi1.setHandleNoteOff(onNoteOff);
    midi1.setHandleControlChange(onCC);
    midi1.setHandlePitchChange(onPitchChange);

    Serial.println("teency-synth ready");
}

// =============================================================================
// LOOP
// =============================================================================
void loop() {
    // Drive USB Host — must be called every loop iteration
    myusb.Task();
    midi1.read();

    // Read potentiometers every 20 ms (50 Hz) — fast enough to feel immediate
    static uint32_t lastPoll = 0;
    if (millis() - lastPoll >= 20) {
        lastPoll = millis();
        readPotentiometers();
        readWaveformSwitch();
    }

    // ── Vibrato LFO — updated every 5 ms ─────────────────────────────────────
    static uint32_t lastLfo = 0;
    uint32_t lfoNow = millis();
    if (lfoNow - lastLfo >= 5) {
        float dt = (lfoNow - lastLfo) * 0.001f;
        lastLfo = lfoNow;
        gLfoPhase += 2.0f * 3.14159265f * VIB_RATE_HZ * dt;
        if (gLfoPhase > 2.0f * 3.14159265f) gLfoPhase -= 2.0f * 3.14159265f;
        float vibCents = sinf(gLfoPhase) * gModWheel * VIB_DEPTH_CENTS;
        for (int i = 0; i < NUM_VOICES; i++) {
            if (voices[i].active) {
                float baseFreq  = noteToFrequency(voices[i].midiNote);
                float vibFactor = powf(2.0f, vibCents / 1200.0f);
                osc1[i].frequency(baseFreq * vibFactor);
                osc2[i].frequency(baseFreq * vibFactor * powf(2.0f, gDetuneCents / 1200.0f));
            }
        }
    }

    // Periodic debug report (audio CPU & memory high-water marks)
    static uint32_t lastReport = 0;
    if (millis() - lastReport > 5000) {
        lastReport = millis();
        Serial.print("CPU max: ");
        Serial.print(AudioProcessorUsageMax(), 1);
        Serial.print("%  Mem max: ");
        Serial.println(AudioMemoryUsageMax());
        AudioProcessorUsageMaxReset();
        AudioMemoryUsageMaxReset();
    }
}

// =============================================================================
// MIDI CALLBACKS
// =============================================================================

void onNoteOn(byte channel, byte note, byte velocity) {
    // Running-status: NoteOn with velocity 0 is treated as NoteOff
    if (velocity == 0) {
        onNoteOff(channel, note, 0);
        return;
    }
    Serial.print("NoteOn  ch="); Serial.print(channel);
    Serial.print(" note="); Serial.print(note);
    Serial.print(" vel="); Serial.println(velocity);

    voiceNoteOn(note, velocity);
}

void onNoteOff(byte channel, byte note, byte velocity) {
    Serial.print("NoteOff ch="); Serial.print(channel);
    Serial.print(" note="); Serial.println(note);

    voiceNoteOff(note);
}

void onCC(byte channel, byte control, byte value) {
    Serial.print("CC ch="); Serial.print(channel);
    Serial.print(" cc="); Serial.print(control);
    Serial.print(" val="); Serial.println(value);

    switch (control) {

        case CC_FILTER_CUTOFF: {
            // Exponential mapping: 0 → 80 Hz, 127 → 8 000 Hz
            // Capped at 8kHz so filter env (1 oct) can't push above ~16kHz
            float t = value / 127.0f;
            gFilterCutoff = 80.0f * powf(8000.0f / 80.0f, t);
            applyFilterToAllVoices();
            break;
        }

        case CC_FILTER_RESONANCE: {
            // Linear mapping: 0 → 0.7 (flat), 127 → 5.0 (self-oscillation)
            gFilterResonance = 0.7f + (value / 127.0f) * (5.0f - 0.7f);
            applyFilterToAllVoices();
            break;
        }

        case CC_MOD_WHEEL: {
            // Mod wheel controls vibrato depth — LFO in loop() applies it
            gModWheel = value / 127.0f;
            break;
        }

        default:
            break;
    }
}

void onPitchChange(byte channel, int value) {
    // value: -8192 (full down) to +8191 (full up), 0 = centre
    float t = (value + 8192) / 16383.0f;   // 0.0 – 1.0
    gDetuneCents = OSC2_DETUNE_CENTS + t * MOD_DETUNE_MAX;
}

// =============================================================================
// HELPERS
// =============================================================================

void applyFilterToAllVoices() {
    for (int i = 0; i < NUM_VOICES; i++) {
        vcf[i].frequency(gFilterCutoff);
        vcf[i].resonance(gFilterResonance);
    }
}

void applyADSRToAllVoices() {
    for (int i = 0; i < NUM_VOICES; i++) {
        ampEnv[i].attack(gAttackMs);
        ampEnv[i].decay(gDecayMs);
        ampEnv[i].sustain(gSustain);
        ampEnv[i].release(gReleaseMs);
        filterEnv[i].release(gReleaseMs);  // filter env release tracks amp release
    }
}

void applyFilterEnvToAllVoices() {
    for (int i = 0; i < NUM_VOICES; i++) {
        filterEnvDC[i].amplitude(gFilterEnvAmount);
        filterEnv[i].attack(gFilterEnvAttackMs);
        filterEnv[i].decay(gFilterEnvDecayMs);
        filterEnv[i].sustain(FILTER_ENV_SUSTAIN);
        filterEnv[i].release(gReleaseMs);
    }
}

// =============================================================================
// POTENTIOMETER READER
// Called every 20 ms from loop(). Updates global synth state and applies
// changes only when a pot has moved more than POT_DEADBAND counts, preventing
// constant re-application from ADC noise.
// =============================================================================
void readPotentiometers() {
    // Last raw readings — initialised to an out-of-range value so the first
    // read always triggers an update regardless of pot position.
    static int lastCutoff      = -100;
    static int lastResonance   = -100;
    static int lastAttack      = -100;
    static int lastDecay       = -100;
    static int lastSustain     = -100;
    static int lastRelease     = -100;
    static int lastFEnvAmount  = -100;
    static int lastFEnvAttack  = -100;
    static int lastFEnvDecay   = -100;

    int raw;

    // ── Filter cutoff ─────────────────────────────────────────────────────────
    raw = analogRead(POT_CUTOFF);
    if (abs(raw - lastCutoff) > POT_DEADBAND) {
        lastCutoff = raw;
        // Exponential taper: full CCW = 80 Hz, full CW = 8 000 Hz
        // Capped at 8kHz so filter env (1 oct) can't push above ~16kHz
        float t = raw / 1023.0f;
        gFilterCutoff = 80.0f * powf(8000.0f / 80.0f, t);
        applyFilterToAllVoices();
    }

    // ── Filter resonance ──────────────────────────────────────────────────────
    raw = analogRead(POT_RESONANCE);
    if (abs(raw - lastResonance) > POT_DEADBAND) {
        lastResonance = raw;
        // Linear: full CCW = 0.7 (flat), full CW = 5.0 (self-oscillation)
        gFilterResonance = 0.7f + (raw / 1023.0f) * (5.0f - 0.7f);
        applyFilterToAllVoices();
    }

    // ── Amp attack ────────────────────────────────────────────────────────────
    raw = analogRead(POT_ATTACK);
    if (abs(raw - lastAttack) > POT_DEADBAND) {
        lastAttack = raw;
        gAttackMs = POT_ATTACK_MIN_MS +
                    (raw / 1023.0f) * (POT_ATTACK_MAX_MS - POT_ATTACK_MIN_MS);
        applyADSRToAllVoices();
    }

    // ── Amp decay ─────────────────────────────────────────────────────────────
    raw = analogRead(POT_DECAY);
    if (abs(raw - lastDecay) > POT_DEADBAND) {
        lastDecay = raw;
        gDecayMs = POT_DECAY_MIN_MS +
                   (raw / 1023.0f) * (POT_DECAY_MAX_MS - POT_DECAY_MIN_MS);
        applyADSRToAllVoices();
    }

    // ── Amp sustain ───────────────────────────────────────────────────────────
    raw = analogRead(POT_SUSTAIN);
    if (abs(raw - lastSustain) > POT_DEADBAND) {
        lastSustain = raw;
        gSustain = raw / 1023.0f;   // 0.0 – 1.0 linear
        applyADSRToAllVoices();
    }

    // ── Amp release ───────────────────────────────────────────────────────────
    raw = analogRead(POT_RELEASE);
    if (abs(raw - lastRelease) > POT_DEADBAND) {
        lastRelease = raw;
        gReleaseMs = POT_RELEASE_MIN_MS +
                     (raw / 1023.0f) * (POT_RELEASE_MAX_MS - POT_RELEASE_MIN_MS);
        applyADSRToAllVoices();        // also updates filterEnv release inside
    }

    // ── Filter env amount ─────────────────────────────────────────────────────
    raw = analogRead(POT_FENV_AMOUNT);
    if (abs(raw - lastFEnvAmount) > POT_DEADBAND) {
        lastFEnvAmount = raw;
        gFilterEnvAmount = raw / 1023.0f;   // 0.0 (off) – 1.0 (full sweep)
        for (int i = 0; i < NUM_VOICES; i++) filterEnvDC[i].amplitude(gFilterEnvAmount);
    }

    // ── Filter env attack ─────────────────────────────────────────────────────
    raw = analogRead(POT_FENV_ATTACK);
    if (abs(raw - lastFEnvAttack) > POT_DEADBAND) {
        lastFEnvAttack = raw;
        gFilterEnvAttackMs = POT_FENV_ATTACK_MIN_MS +
                             (raw / 1023.0f) * (POT_FENV_ATTACK_MAX_MS - POT_FENV_ATTACK_MIN_MS);
        for (int i = 0; i < NUM_VOICES; i++) filterEnv[i].attack(gFilterEnvAttackMs);
    }

    // ── Filter env decay ──────────────────────────────────────────────────────
    raw = analogRead(POT_FENV_DECAY);
    if (abs(raw - lastFEnvDecay) > POT_DEADBAND) {
        lastFEnvDecay = raw;
        gFilterEnvDecayMs = POT_FENV_DECAY_MIN_MS +
                            (raw / 1023.0f) * (POT_FENV_DECAY_MAX_MS - POT_FENV_DECAY_MIN_MS);
        for (int i = 0; i < NUM_VOICES; i++) filterEnv[i].decay(gFilterEnvDecayMs);
    }
}

// =============================================================================
// WAVEFORM SELECTOR SWITCH
// Reads SP8T rotary switch (common → GND, positions → INPUT_PULLUP pins).
// Positions 0–4 select a waveform; positions 5–7 are unassigned (do nothing).
// =============================================================================
void readWaveformSwitch() {
    static const int waveforms[5] = {
        WAVEFORM_SINE,
        WAVEFORM_SAWTOOTH,
        WAVEFORM_SQUARE,
        WAVEFORM_TRIANGLE,
        WAVEFORM_BANDLIMIT_SAWTOOTH
    };
    static int lastPos = -1;

    // Find which position pin is pulled LOW
    int pos = -1;
    for (int i = 0; i < 8; i++) {
        if (digitalRead(WAVE_SW_PINS[i]) == LOW) {
            pos = i;
            break;
        }
    }

    // Positions 5–7 are unassigned — do nothing
    if (pos > 4) return;

    // No change since last read — do nothing
    if (pos == lastPos) return;
    lastPos = pos;

    // pos == -1 means switch is between positions — do nothing
    if (pos < 0) return;

    gWaveform = waveforms[pos];
    for (int i = 0; i < NUM_VOICES; i++) {
        osc1[i].begin(gWaveform);
        osc2[i].begin(gWaveform);
    }
    Serial.print("Waveform changed to position "); Serial.println(pos);
}
