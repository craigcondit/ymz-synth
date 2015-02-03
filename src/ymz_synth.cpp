// Do not remove the include below
#include "ymz_synth.h"

/*
  YMZ284 Midi

  Turns a pair of Yamaha YMZ284 sound chips into a MIDI-controlled
  instrument (or at least it will).

  Created 31 Jan 2015
  by Craig Condit
 */

#define LED 13

void setup() {
	pinMode(13, OUTPUT);
}

void loop() {
	digitalWrite(13, HIGH);
	delay(200);
	digitalWrite(13, LOW);
	delay(200);
}
