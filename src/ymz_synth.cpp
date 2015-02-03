// Do not remove the include below
#include "ymz_synth.h"

/*
  YMZ284 Midi
 
 Turns a pair of Yamaha YMZ284 sound chips into a MIDI-controlled
 instrument (or at least it will).
 
 Created 31 Jan 2015
 by Craig Condit
 */

/*
Ideas for features:

PC 1 - Normal mode. MIDI + mapping of any effects that make sense.

How to map noise to MIDI? 

PC 2 - Register mode. CC20-34? control YMZ284 chips directly.

We could use different MIDI channels for mono / left / right. Configurable via EEPROM?

Channel 1 - stereo
Channel 2 - left
Channel 3 - right
Channel 4 - stereo noise
Channel 5 - left noise
Channel 6 - right noise

Implementation notes:

Music frequency is a 12-bit value (TP). Frequency is given by fT = fMaster (4MHz) / (32 * TP). Or simplified:

  125,000 / TP.

A value of 0 is off.

Noise frequency is a 5-bit value (NP). Frequency is given by fN = fMaster (4MHz) / (32 * NP). Or simplified:

  125,000 / NP.
  
A value of 0 is off.


Volume is available per-channel, with range 0..15. Setting bit 5 (16) output level is determined by envelope generator value (i.e. LFO).
Envelope frequency is 


 */
#include <Arduino.h>
#include <hcYmzShield.h>
#include <MIDI.h>

#define RED_LED 7
#define GREEN_LED 6
#define PINK_LED 5
#define WHITE_LED 4

#define LED_COUNT 4


MIDI_CREATE_DEFAULT_INSTANCE();

// define an array of LEDs so we can do patterns (left to right)
const int LEDS[LED_COUNT] = { 
  RED_LED, GREEN_LED, PINK_LED, WHITE_LED };

volatile unsigned long redDecay = 0;

int PC = 1;

void midiActivity() {
  digitalWrite(RED_LED, HIGH);
  redDecay = millis() + 50;  
}

void handleNoteOn(byte channel, byte pitch, byte velocity)
{
  midiActivity();
  
  if (velocity < 10) { 
    velocity = 10; 
  }

  YMZ.setVolumeByEnvelope(0);
  YMZ.setVolumeByEnvelope(1);
  YMZ.setVolumeByEnvelope(2);
  YMZ.setVolumeByEnvelope(3);
  YMZ.setVolumeByEnvelope(4);
  YMZ.setVolumeByEnvelope(5);
  
  YMZ.setNote(0, pitch);
  YMZ.setNote(1, pitch + 4);
  YMZ.setNote(2, pitch + 7);
  YMZ.setNote(3, pitch);
  YMZ.setNote(4, pitch + 4);
  YMZ.setNote(5, pitch + 7);
  uint16_t period = YMZ.getTonePeriod(0);
  
  YMZ.setEnvelopePeriod(period / 2); // generate overtone (note, this doesn't work well at higher frequencies)
  YMZ.startEnvelope(12);
  
  MIDI.sendNoteOn(pitch , velocity,2);
  MIDI.sendNoteOn(pitch + 4, velocity-10,2);
  MIDI.sendNoteOn(pitch + 7, velocity-10,2);
  MIDI.sendNoteOn(pitch + 12, velocity-10,2);
}

void handleNoteOff(byte channel, byte pitch, byte velocity)
{
  midiActivity();
  
  if (velocity < 10) { 
    velocity = 10; 
  }
  
  YMZ.setNote(0, OFF);
  YMZ.setNote(1, OFF);
  YMZ.setNote(2, OFF);
  YMZ.setNote(3, OFF);
  YMZ.setNote(4, OFF);
  YMZ.setNote(5, OFF);
  YMZ.setVolume(0, 0);
  YMZ.setVolume(1, 0);
  YMZ.setVolume(2, 0);
  YMZ.setVolume(3, 0);
  YMZ.setVolume(4, 0);
  YMZ.setVolume(5, 0);
  YMZ.setEnvelopeFrequency(4.0);
  YMZ.startEnvelope(0);  
  
  MIDI.sendNoteOff(pitch, velocity, 2);
  MIDI.sendNoteOff(pitch + 4, velocity-10, 2);
  MIDI.sendNoteOff(pitch + 7, velocity-10, 2);
  MIDI.sendNoteOff(pitch + 12, velocity-10, 2);
}

void setup() {
  for (int i = 0; i < LED_COUNT; i++) {
    pinMode(LEDS[i], OUTPUT);
    digitalWrite(LEDS[i], LOW);
  }

  YMZ.mute();
  YMZ.setTempo(120);
  YMZ.setArticulation(LEGATO);

  MIDI.setHandleNoteOn(handleNoteOn);
  MIDI.setHandleNoteOff(handleNoteOff);
  MIDI.begin(1); // TODO make input channel configurable
  MIDI.turnThruOff();
  
  // let the user know we're ready to go by flashing all the lights
  for (int i = 0; i < LED_COUNT; i++) {
    digitalWrite(LEDS[i], HIGH);
    delay(250);
    digitalWrite(LEDS[i], LOW);
  }

}

void loop() {
  unsigned long time = millis();
  if (redDecay < time || redDecay > (time + 30000L)) {
    redDecay = 0;
    digitalWrite(RED_LED, LOW);
  }
  MIDI.read();  
}

