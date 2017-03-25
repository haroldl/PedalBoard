/*
 *
 * PedalBoard
 *
 * Connect an organ pedalboard to your computer for playing music via MIDI, or to perform
 * keyboard shortcuts with your feet.
 *
 * The Arduino (which must be a 32u4 in order to perform USB keyboard typing) is connected
 * to a organ pedalboard.
 *
 * The pedalboard can operate in 2 modes:
 *
 * 1. Keyboard mode (the default)
 *
 *    The pedals operate as a set of keyboard keys (e.g. Shift, Control) and the Arduino
 *    will be seen as a USB HID keyboard by the OS of the connected computer.
 *
 * 2. MIDI mode (activate by holding the button on pin 2 during reset)
 *
 *    The pedals operate as a MIDI keyboard and the Arduino will be seen as a USB MIDI
 *    device by the OS of the connected computer.
 *
 * Developer Setup:
 * 
 * You'll need the MIDIUSB library.
 * 1. Sketch -> Include Library -> Manage Libraries...
 * 2. Search for MIDIUSB, and click on that library, then click the Install button.
 *
 * Sample MIDI project for reference:
 * https://www.arduino.cc/en/Tutorial/MidiDevice
 *
 * N.B. Keyboard and Serial don't work together, so to support typing as a USB keyboard
 *      we don't get to use e.g. Serial.println to debug.
 */

#include <frequencyToNote.h>
#include <MIDIUSB.h>
#include <pitchToNote.h>

#include <Keyboard.h>

#define MIDI_MODE 1
#define KEYB_MODE 2

int currentMode = KEYB_MODE;

// One octave + the next C.
// We'll wire up pins 0-12 for the inputs.
#define NUM_PEDALS 13

// Probably seen as MIDI channel 2 on the synth/computer side
const byte midiChannel = 1;

// How loud - hard coded since these are on/off switches; [0, 127]
const byte midiVelocity = 64;

const uint8_t pedalInputs[NUM_PEDALS] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};

const byte notePitches[NUM_PEDALS] = {
  pitchC3,  pitchD3b, pitchD3, pitchE3b, pitchE3,
  pitchF3,  pitchG3b, pitchG3, pitchA3b, pitchA3,
  pitchB3b, pitchB3,  pitchC3
  };

const char pedalKeys[NUM_PEDALS] = {
  //  C              C#            D             D#
  KEY_LEFT_CTRL, KEY_LEFT_ALT, KEY_LEFT_SHIFT, KEY_TAB,
  // E F F# G
  'e', 'f', 'g', 'h',
  //  G#      A       A#      B       C
  KEY_F3, KEY_F4, KEY_F5, KEY_F6, KEY_F7};

// Using INPUT_PULLUP for these, so 0 means the pedal is down, 1 means the pedal is up.
int pedalValues[NUM_PEDALS] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};

/*
 * selectPin - an input used at setup time to select MIDI or Keyboard mode
 *
 * The Keyboard and MIDIUSB libraries for output both use the USB connection
 * so we want to select one mode or the other at setup time. To select MIDI
 * mode, hold down the selectPin input button and reset the Arduino board.
 * By default, we start in Keyboard mode.
 */
int selectPin = A5;

const int greenLED = A0;
const int redLED = A1;

/*
 * Function pointers that are invoked when a button changes state.
 */
void (*buttonDownHandler)(int buttonNum) = NULL;
void (*buttonUpHandler)(int buttonNum) = NULL;

void setup() {
  for (int i = 0; i < NUM_PEDALS; i++) {
    pinMode(pedalInputs[i], INPUT_PULLUP);
  }

  pinMode(greenLED, OUTPUT);
  pinMode(redLED, OUTPUT);

  digitalWrite(greenLED, HIGH);
  digitalWrite(redLED, HIGH);
  delay(2000);

  // Figure out which mode we're in based on the switch (default: MIDI)
  pinMode(selectPin, INPUT_PULLUP);
  delay(50); // 50 ms to allow the button signal to stabilize
  int selectValue = digitalRead(selectPin);
  if (selectValue != LOW) {
    // Switch is open
    currentMode = MIDI_MODE;
    digitalWrite(greenLED, HIGH);
    digitalWrite(redLED, LOW);
  } else {
    // Switch is closed
    currentMode = KEYB_MODE;
    digitalWrite(greenLED, LOW);
    digitalWrite(redLED, HIGH);
  }

  if (currentMode == KEYB_MODE) {
    // USB Keyboard mode
    buttonDownHandler = handleKeyboardDown;
    buttonUpHandler = handleKeyboardUp;
    Keyboard.begin();
  } else {
    // MIDI mode
    buttonDownHandler = handleMidiDown;
    buttonUpHandler = handleMidiUp;
  }
}

void loop() {
  // Check the input pins for the pedals and call
  // the appropriate event handler for each change.
  for (int i = 0; i < NUM_PEDALS; i++) {
    int pedalValue = digitalRead(pedalInputs[i]);
    if (pedalValues[i] != pedalValue) {
      if (pedalValue == 0) {
        // The pedal just went down.
        if (buttonDownHandler != NULL) {
          buttonDownHandler(i);
        }
      } else {
        // The pedal just came up.
        if (buttonUpHandler != NULL) {
          buttonUpHandler(i);
        }
      }
    }
    pedalValues[i] = pedalValue;
  }
  delay(10);
}

void handleKeyboardDown(int buttonNum) {
  Keyboard.press(pedalKeys[buttonNum]);
}

void handleKeyboardUp(int buttonNum) {
  Keyboard.release(pedalKeys[buttonNum]);
}

void handleMidiDown(int buttonNum) {
  startNote(notePitches[buttonNum]);
}

void handleMidiUp(int buttonNum) {
  endNote(notePitches[buttonNum]);
}

// Send a MIDI event to start playing the note with the given pitch.
void startNote(byte pitch) {
  byte velocity = midiVelocity;
  midiEventPacket_t noteOn = {0x09, 0x90 | midiChannel, pitch, velocity};
  digitalWrite(redLED, HIGH);
  MidiUSB.sendMIDI(noteOn);
  MidiUSB.flush();
  digitalWrite(redLED, LOW);
}

// Send a MIDI event to stop playing the note with the given pitch.
void endNote(byte pitch) {
  byte velocity = 0;
  midiEventPacket_t noteOff = {0x08, 0x80 | midiChannel, pitch, velocity};
  digitalWrite(redLED, HIGH);
  MidiUSB.sendMIDI(noteOff);
  MidiUSB.flush();
  digitalWrite(redLED, LOW);
}

