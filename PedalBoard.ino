/*
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
 */

#include <frequencyToNote.h>
#include <MIDIUSB.h>
#include <pitchToNote.h>

#include <Keyboard.h>

#define MIDI_MODE 1
#define KEYB_MODE 2

int currentMode = KEYB_MODE;

/*
 * selectPin - an input used at setup time to select MIDI or Keyboard mode
 *
 * The Keyboard and MIDIUSB libraries for output both use the USB connection
 * so we want to select one mode or the other at setup time. To select MIDI
 * mode, hold down the selectPin input button and reset the Arduino board.
 * By default, we start in Keyboard mode.Hello, world!
 */
int selectPin = 2;

int ledPin = 13;

void setup() {
  pinMode(ledPin, OUTPUT);
  pinMode(selectPin, INPUT_PULLUP);
  delay(50); // 50 ms to allow the button signal to stabilize
  int selectValue = digitalRead(selectPin);
  if (selectValue == 0) {
    // button is pressed
    currentMode = MIDI_MODE;
  } else {
    // button is up
    currentMode = KEYB_MODE;
  }

  if (currentMode == KEYB_MODE) {
    Keyboard.begin();
  } else {
    // Nothing to do for the MIDI mode.
  }
}

void loop() {
  delay(1000);
  if (currentMode == KEYB_MODE) {
    Keyboard.println("Hello, world!\n");
    Keyboard.println(currentMode);
    delay(10000); // 10 sec
  } else {
    // Check the input pins for the pedals and send appropriate MIDI events for each.
    delay(10000); // 10 sec
  }
}
