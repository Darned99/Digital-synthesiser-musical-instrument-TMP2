# Digital-synthesiser-musical-instrument-TMP2

# Music Instrument - Synthesizer

## Project for Microprocessor Technology 2

### Author
Bartłomiej Kisielewski

### Date
January 27, 2025

---

## Project Goal
The goal of this project was to create an easy-to-use musical synthesizer that:
- Generates various sounds (sine, triangular, sawtooth)
- Offers a wide octave range (from 2 to 6)
- Provides simple volume control
- Displays information on an LCD screen

## Components Used
- FRDM-KL05Z Development Board
- 4x4 Matrix Keypad
- 16x2 LCD Display
- I2C Converter LCNM 1602
- Speaker WSR-04489

![image](https://github.com/user-attachments/assets/85408c4c-5803-4d76-a8b0-392a733199b1)

## Functionality
- Selection of generated waveform (sine, triangular, sawtooth) via a single button
- Wide octave range (5 octaves, from 2 to 6)
- Display of sound information (note, octave, frequency, volume, waveform type)
- Keyboard support (13 sound buttons, octave change buttons, waveform selection button)
- Volume control via a touch panel

## System Overview
### Circuit Diagram
The system uses the following connections:

![image](https://github.com/user-attachments/assets/b917ed53-6855-4dbf-9f1b-50a65b9f8353)

- **LCD Display:**
  - PTB3: SCL
  - PTB4: SDA
  - +5V: VCC
  - GND: GND

- **Keyboard:**
  - PTA12: C4
  - PTA11: C3
  - PTA10: C2
  - PTA9: C1
  - PTA5: R1
  - PTA6: R2
  - PTA7: R3
  - PTA8: R4

- **Speaker:**
  - PTB1: VCC (3.3V)
  - +5V: +5V
  - GND: GND

### How It Works
The program implements a musical instrument on the MKL05Z4 board using a 4x4 keypad, an LCD display, and a speaker. Main features include:
- Sequential selection of 3 waveforms (sine, triangular, sawtooth)
- Wide octave range (5 octaves from 2 to 6)
- Accurate note frequencies (61 notes, 5 octaves)
- Real-time display of note, octave, frequency, volume, and waveform type on the LCD

### User Interface
The synthesizer uses 13 buttons (S1 to S13) as sound keys, where:
- S1: C
- S2: C# / D♭
- S3: D
- ...
- S12: B
- S13: C of the next octave

Octave changes are controlled by buttons S14 (down) and S15 (up), while waveform change is managed by button S16.
Volume is controlled via a touch panel on the MKL05Z board.

### Software Implementation
- **Keyboard Handling:**
  - Uses row and column scanning to detect key presses.
  - Reads key input and calculates corresponding modulation value.
  - Supports octave change and debounce handling.

- **Sound Generation:**
  - Uses DDS (Direct Digital Synthesis) for waveform generation.
  - Computes frequencies based on musical scales and outputs them via DAC.

- **Volume Control:**
  - Touch panel readings control the amplitude of the sound.

- **LCD Display:**
  - Continuously updated with current sound settings and status.

- **Main Function:**
  - Initializes the system and handles main program logic.
  - Uses interrupt-driven sound generation for real-time audio output.

## Running the Project
1. Connect the FRDM-KL05Z board to your computer.
2. Upload the code to the board.
3. Use the keypad to control sound settings and play notes.

