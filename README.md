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
- 5-position waveform selector (SP8T rotary switch)

---

## Wiring Diagram

### Audio Board

The PJRC Audio Board Rev D stacks directly on top of the Teensy 4.1 via the header pins.
No additional wiring is required for audio. The 3.5mm output jack is on the Audio Board.

```
Teensy 4.1
┌─────────────────────┐
│  [Audio Board Rev D]│  ← stacks directly on top
│                     │
│  3.5mm jack ────────┼──► Headphones / Line Out
└─────────────────────┘
```

---

### USB Connections

```
PC ──── USB (Micro-USB) ────► Teensy 4.1 (programming + power)

Arturia KeyStep
  Micro-USB port ──── USB cable ────► Teensy 4.1 USB Host port
```

> The Teensy 4.1 USB Host port is a 5-pin header on the board.
> A USB-A socket must be wired to these pins to accept the KeyStep's cable.

---

### Potentiometers

All potentiometers are wired the same way:

```
3.3V ──── Left leg (power)
GND  ──── Right leg (ground)
          Middle leg (wiper) ──── Teensy ADC pin
```

| Pot | Teensy Pin | Parameter | Resistance |
|-----|-----------|-----------|------------|
| 1 | A0 (pin 14) | Filter cutoff | 10k |
| 2 | A1 (pin 15) | Filter resonance | 100k |
| 3 | A2 (pin 16) | Amp attack | 100k |
| 4 | A3 (pin 17) | Amp decay | 100k |
| 5 | A8 (pin 22) | Amp sustain | 100k |
| 6 | A10 (pin 24) | Amp release | 100k |
| 7 | A11 (pin 25) | Filter env amount | 100k |
| 8 | A12 (pin 26) | Filter env attack | 100k |
| 9 | A13 (pin 27) | Filter env decay | 100k |

> **Important:** Use the Teensy's **3.3V pin** for pot power — do not use 5V.
> The ADC pins are rated for 3.3V maximum.

---

### Waveform Selector Switch (ADA2925 SP8T)

Wire the common pin to GND. Each position pin connects to a Teensy digital input.
The internal pull-up resistors are enabled in software — no external resistors needed.

```
GND ──── Common pin

Position 1 ──── Teensy pin 0   (Sine)
Position 2 ──── Teensy pin 1   (Sawtooth)
Position 3 ──── Teensy pin 2   (Square)
Position 4 ──── Teensy pin 3   (Triangle)
Position 5 ──── Teensy pin 4   (Band-limited sawtooth)
Position 6 ──── Teensy pin 5   (unassigned — no change)
Position 7 ──── Teensy pin 6   (unassigned — no change)
Position 8 ──── Teensy pin 9   (unassigned — no change)
```

---

### Pins to Avoid

These pins are used by the Audio Board Rev D and must not be connected to anything else:

| Pin | Function |
|-----|----------|
| 7  | I2S TX |
| 8  | I2S RX |
| 10 | SD card CS |
| 11 | SD card MOSI |
| 12 | SD card MISO |
| 13 | SD card SCK |
| 18 | I2C SDA (codec control) |
| 19 | I2C SCL (codec control) |
| 20 | I2S LRCLK |
| 21 | I2S BCLK |
| 23 | I2S MCLK |

---

## MIDI CC Mappings

| CC | Parameter |
|----|-----------|
| CC 1  | Mod wheel → vibrato depth |
| CC 71 | Filter resonance |
| CC 74 | Filter cutoff |

Pitch wheel controls osc2 detune widening (centre = +7 cents, full up = +57 cents).

---

## Dependencies

- [Teensyduino](https://www.pjrc.com/teensy/teensyduino.html)
- Teensy Audio Library (included with Teensyduino)
- USBHost_t36 (included with Teensyduino)

## Building

1. Install Arduino IDE and Teensyduino
2. Open `teency_synth/teency_synth.ino`
3. Select **Tools → Board → Teensy 4.1**
4. Upload
