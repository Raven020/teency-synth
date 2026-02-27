# teency-synth

A 6-voice polyphonic subtractive synthesizer built on the Teensy 4.1 and Audio Board Rev D.

## Hardware

| Component | Details |
|-----------|---------|
| Microcontroller | Teensy 4.1 |
| Audio Board | PJRC Audio Board Rev D (SGTL5000 codec) |
| MIDI Input | Arturia KeyStep via USB Host MIDI |
| Audio Output | 3.5mm jack (headphone / line) |

## Features

- 6-voice polyphony with oldest-note voice stealing
- Dual sawtooth oscillators per voice with adjustable detune
- State-variable low-pass filter with ADSR envelope modulation
- Amplitude ADSR envelope
- Pitch wheel → osc2 detune widening
- Mod wheel → vibrato (5.5 Hz LFO)
- 9 potentiometer controls

## Potentiometer Assignments

| Pin | Parameter |
|-----|-----------|
| A0 (14) | Filter cutoff |
| A1 (15) | Filter resonance |
| A2 (16) | Amp attack |
| A3 (17) | Amp decay |
| A8 (22) | Amp sustain |
| A10 (24) | Amp release |
| A11 (25) | Filter env amount |
| A12 (26) | Filter env attack |
| A13 (27) | Filter env decay |

## MIDI CC Mappings

| CC | Parameter |
|----|-----------|
| CC 1  | Mod wheel → vibrato depth |
| CC 71 | Filter resonance |
| CC 74 | Filter cutoff |

## Dependencies

- [Teensyduino](https://www.pjrc.com/teensy/teensyduino.html)
- Teensy Audio Library (included with Teensyduino)
- USBHost_t36 (included with Teensyduino)

## Building

1. Install Arduino IDE and Teensyduino
2. Open `teency_synth/teency_synth.ino`
3. Select **Tools → Board → Teensy 4.1**
4. Upload
