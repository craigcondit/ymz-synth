/**
 * Hardchord YMZ Shield 1.0 (hcYmzShield.cpp)
 * Derrick Sobodash <derrick@sobodash.com>
 * Version 0.4.3
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer. 
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution. 
 */

#include "hcYmzShield.h"


// Create the handle to the shield.
hcYmzShield YMZ;


// Base tone periods for the first 12 MIDI notes at 4MHz
#ifdef __FAVOR_PRECISION
static const uint16_t tpMidi[128] = {
      0,     0,     0,     0,     0,     0, // Octave -1
      0,     0,     0,     0,     0,     0,
      0,     0,     0,     0,     0,     0, // Octave 0
      0,     0,     0,     0,     0,     0,
   3822,  3608,  3405,  3214,  3034,  2863, // Octave 1
   2703,  2551,  2408,  2273,  2145,  2025,
   1911,  1804,  1703,  1607,  1517,  1432, // Octave 2
   1351,  1276,  1204,  1136,  1073,  1012,
    956,   902,   851,   804,   758,   716, // Octave 3
    676,   638,   602,   568,   536,   506,
    478,   451,   426,   402,   379,   358, // Octave 4
    338,   319,   301,   284,   268,   253,
    239,   225,   213,   201,   190,   179, // Octave 5
    169,   159,   150,   142,   134,   127,
    119,   113,   106,   100,    95,    89, // Octave 6
     84,    80,    75,    71,    67,    63,
     60,    56,    53,    50,    47,    45, // Octave 7
     42,    40,    38,    36,    34,    32,
     30,     0,     0,     0,     0,     0, // Octave 8
      0,     0,     0,     0,     0,     0,
      0,     0,     0,     0,     0,     0, // Octave 9
      0,     0
};
#else
static const uint16_t tpMidi[12] = {
  15289, 14431, 13621, 12856, 12135, 11454,
  10811, 10204,  9631,  9091,  8581,  8099
};
#endif


/**
 * Helper Methods
 * 
 * These static methods control data exchange with the YMZ284 chips and the
 * board's 75HC595 serial shifter.
 */
#if defined(__SPI_HACK)
void hcYmzShield::_shiftOut(uint8_t value) {
  PORTB &= ~B00000010;
  SPDR = value;
  while (!(SPSR & B10000000));
  PORTB |=  B00000010;
  uint8_t c = SPDR;
}
void hcYmzShield::_busAddress() {
  PORTB &= ~B00000001;
}
void hcYmzShield::_busData() {
  PORTB |=  B00000001;
}
void hcYmzShield::_psgWrite() {
  PORTD &= ~B00001100; // MASK_CS1 | MASK_CS2
  PORTD |=  B00001100; // MASK_CS1 | MASK_CS2
}
void hcYmzShield::_psg0Write() {
  PORTD &= ~B00000100;
  PORTD |=  B00000100;
}
void hcYmzShield::_psg1Write() {
  PORTD &= ~B00001000;
  PORTD |=  B00001000;
}
void hcYmzShield::_debugLightOn() {
}
void hcYmzShield::_debugLightOff() {
}
#elif defined(__AVR_ATmega328P__) || defined(__AVR_ATmega328__) || defined(__AVR_ATmega168__) || defined(__AVR_ATmega168P__) || defined(__AVR_ATmega168A__) || defined(__AVR_ATmega168PA__)
void hcYmzShield::_shiftOut(uint8_t value) {
  PORTD &= ~B00001000;     // Latch
  for(uint8_t i = 8; i; i--) {
    PORTD &= ~B00010000;   // Clock
    PORTD &= ~B00000100;   // Data
    if(value & 0x80)
      PORTD |=  B00000100; // Data
    value <<= 1;
    PORTD |=  B00010000;   // Clock
  }
  PORTD |=  B00001000;     // Latch
}
void hcYmzShield::_busAddress() {
  PORTB &= ~B00001000;
}
void hcYmzShield::_busData() {
  PORTB |=  B00001000;
}
void hcYmzShield::_psgWrite() {
  PORTB &= ~B00010100;
  PORTB |=  B00010100;
}
void hcYmzShield::_psg0Write() {
  PORTB &= ~B00010000;
  PORTB |=  B00010000;
}
void hcYmzShield::_psg1Write() {
  PORTB &= ~B00000100;
  PORTB |=  B00000100;
}
void hcYmzShield::_debugLightOn() {
  PORTB |=  B0010000;
}
void hcYmzShield::_debugLightOff() {
  PORTB &= ~B0010000;
}
#elif defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
void hcYmzShield::_shiftOut(uint8_t value) {
  PORTE &= ~B00010000;     // Latch
  for(uint8_t i = 8; i; i--) {
    PORTG &= ~B00100000;   // Clock
    PORTE &= ~B00010000;   // Data
    if(value & 0x80)
      PORTE |=  B00010000; // Data
    value <<= 1;
    PORTG |=  B00100000;   // Clock
  }
  PORTE |=  B00010000;     // Latch
}
void hcYmzShield::_busAddress() {
  PORTB &= ~B00100000;
}
void hcYmzShield::_busData() {
  PORTB |=  B00100000;
}
void hcYmzShield::_psgWrite() {
  PORTB &= ~B01010000; // MASK_CS1 | MASK_CS2
  PORTB |=  B01010000; // MASK_CS1 | MASK_CS2
}
void hcYmzShield::_psg0Write() {
  PORTB &= ~B00010000;
  PORTB |=  B00010000;
}
void hcYmzShield::_psg1Write() {
  PORTB &= ~B01000000;
  PORTB |=  B01000000;
}
void hcYmzShield::_debugLightOn() {
  PORTB |=  B1000000;
}
void hcYmzShield::_debugLightOff() {
  PORTB &= ~B1000000;
}
#else
void hcYmzShield::_shiftOut(uint8_t value) {
  digitalWrite(PIN_RCK, LOW);
  shiftOut(PIN_SER, PIN_SRCK, MSBFIRST, value);
  digitalWrite(PIN_RCK, HIGH);
}
void hcYmzShield::_busAddress() {
  digitalWrite(PIN_SEL, LOW);
}
void hcYmzShield::_busData() {
  digitalWrite(PIN_SEL, HIGH);
}
void hcYmzShield::_psgWrite() {
  digitalWrite(PIN_CS2, LOW);
  digitalWrite(PIN_CS1, LOW);
  digitalWrite(PIN_CS2, HIGH);
  digitalWrite(PIN_CS1, HIGH);
}
void hcYmzShield::_psg0Write() {
  digitalWrite(PIN_CS2, LOW);
  digitalWrite(PIN_CS2, HIGH);
}
void hcYmzShield::_psg1Write() {
  digitalWrite(PIN_CS1, LOW);
  digitalWrite(PIN_CS1, HIGH);
}
void hcYmzShield::_debugLightOn() {
  digitalWrite(13, HIGH);
}
void hcYmzShield::_debugLightOff() {
  digitalWrite(13, LOW);
}
#endif


/**
 * public hcYmzShield()
 *
 * Initializes the shield as an object.
 */
hcYmzShield::hcYmzShield() {
  #if defined(__SPI_HACK)
  DDRB  |= B00101111;
  DDRD  |= B00001100;
  PORTB &= ~B00101000;
  PORTB |= B00000110;
  PORTD |= B00001100;
  SPCR |= B01010000;
  SPSR |= B00000001;
  #elif defined(__AVR_ATmega328P__) || defined(__AVR_ATmega328__) || defined(__AVR_ATmega168__) || defined(__AVR_ATmega168P__) || defined(__AVR_ATmega168A__) || defined(__AVR_ATmega168PA__)
  DDRB  |= B00111100; // LED | MASK_CS1 | MASK_SEL | MASK_CS2
  DDRD  |= B00011100; // MASK_SER | MASK_RCK | MASK_SRCK
  PORTB |= B00010100; // MASK_CS1 | MASK_CS2
  #elif defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
  DDRB  |= B11110000; // LED | CS1 | SEL | CS2
  DDRE  |= B00110000; // SER | RCK
  DDRG  |= B00100000; // SRCK
  PORTB |= B01010000; // CS1 | CS2
  #else
  pinMode(PIN_SER,  OUTPUT);
  pinMode(PIN_RCK,  OUTPUT);
  pinMode(PIN_SRCK, OUTPUT);
  pinMode(PIN_CS1,  OUTPUT);
  pinMode(PIN_SEL,  OUTPUT);
  pinMode(PIN_CS2,  OUTPUT);
  pinMode(13,       OUTPUT); // LED
  digitalWrite(PIN_CS1, HIGH);
  digitalWrite(PIN_CS2, HIGH);
  #endif
  
  // Initialize register backup to 0
  memset(_psg0Registers, 0, 0x0d);
  memset(_psg1Registers, 0, 0x0d);
  
  // Set default tempo
  _bpm = MODERATO;

  // Set default articulation
  _articulation = 8;
  
  // Make sure the speakers don't fart
  mute();
  setVolume(0);
}

/**
 * accessor for _getRegisterPsg
 */
uint8_t hcYmzShield::getRegisterPsg(uint8_t reg) {
	return _psg0Registers[reg];
}

/**
 * accessor for _getRegisterPsg0
 */
uint8_t hcYmzShield::getRegisterPsg0(uint8_t reg) {
	return _psg0Registers[reg];
}

/**
 * accessor for _getRegisterPsg0
 */
uint8_t hcYmzShield::getRegisterPsg1(uint8_t reg) {
	return _psg1Registers[reg];
}

/**
 * accessor for _setRegisterPsg
 */
void hcYmzShield::setRegisterPsg(uint8_t reg, uint8_t data) {
  _setRegisterPsg(reg, data);
}

/**
 * accessor for _setRegisterPsg0
 */
void hcYmzShield::setRegisterPsg0(uint8_t reg, uint8_t data) {
  _setRegisterPsg0(reg, data);
}

/**
 * accessor for _setRegisterPsg1
 */
void hcYmzShield::setRegisterPsg1(uint8_t reg, uint8_t data) {
  _setRegisterPsg1(reg, data);
}

/**
 * private hcYmzShield::_setRegisterPsg()
 * 
 * Set a byte in both YMZ284s' internal registers.
 */
void hcYmzShield::_setRegisterPsg(uint8_t reg, uint8_t data) {
  _debugLightOn();

  // Switch the bus to recieve a register address and shift it out
  _busAddress();
  _shiftOut(reg);
  _psgWrite();
  
  // Switch the bus to recieve data and shift it out
  _busData();
  _shiftOut(data);
  _psgWrite();
  
  // Copy the byte to the internal register map
  _psg0Registers[reg] = data;
  _psg1Registers[reg] = data;

  _debugLightOff();
}


/**
 * private hcYmzShield::_setRegisterPsg0()
 * 
 * Set a byte in PSG0's internal registers.
 */
void hcYmzShield::_setRegisterPsg0(uint8_t reg, uint8_t data) {
  _debugLightOn();

  // Switch the bus to recieve a register address and shift it out
  _busAddress();
  _shiftOut(reg);
  _psg1Write();
  
  // Switch the bus to recieve data and shift it out
  _busData();
  _shiftOut(data);
  _psg1Write();
  
  // Copy the byte to the internal register map
  _psg0Registers[reg] = data;

  _debugLightOff();
}


/**
 * private hcYmzShield::_setRegisterPsg1()
 * 
 * Set a byte in PSG1's internal registers.
 */
void hcYmzShield::_setRegisterPsg1(uint8_t reg, uint8_t data) {
  _debugLightOn();

  // Switch the bus to recieve a register address and shift it out
  _busAddress();
  _shiftOut(reg);  
  _psg0Write();
  
  // Switch the bus to recieve data and shift it out
  _busData();
  _shiftOut(data);
  _psg0Write();
  
  // Copy the byte to the internal register map
  _psg1Registers[reg] = data;

  _debugLightOff();
}


/**
 * public hcYmzShield::setTonePeriod()
 * 
 * Sets the tone period (TP) of a channel. Frequency is generated as
 * Hz = (4 * 10^6) / (32 * TP)
 */
void hcYmzShield::setTonePeriod(uint8_t channel, uint16_t tp) {
  tp &= 0x0fff; // Sanitize
  
  if(channel > 2) {
    _setRegisterPsg1(((channel -= 3) *= 2), tp & 0xff);
    _setRegisterPsg1((++channel), tp >> 8);
  }
  else {
    _setRegisterPsg0((channel *= 2), tp & 0xff);
    _setRegisterPsg0((++channel), tp >> 8);
  }
}


/**
 * public hcYmzShield::getTonePeriod()
 * 
 * Returns the current tone period of a channel.
 */
uint16_t hcYmzShield::getTonePeriod(uint8_t channel) {
  uint16_t tp;
  
  if(channel > 2) {
    tp =  (_psg1Registers[((channel -= 3) *=2)]);
    tp += (_psg1Registers[(++channel)] << 8);
  }
  else {
    tp =  (_psg0Registers[(channel *=2)]);
    tp += (_psg0Registers[(++channel)] << 8);
  }
  
  return(tp);
}


/**
 * public hcYmzShield::setToneFrequency()
 * 
 * Sets the tone period of a channel to produce a given frequency in Hz.
 */
void hcYmzShield::setToneFrequency(uint8_t channel, float hz) {
  uint16_t tp = 125000 / hz;
  
  if(channel > 2) {
    _setRegisterPsg1(((channel -= 3) *= 2), tp & 0xff);
    _setRegisterPsg1((++channel), tp >> 8);
  }
  else {
    _setRegisterPsg0((channel *= 2), tp & 0xff);
    _setRegisterPsg0((++channel), tp >> 8);
  }
}


/**
 * public hcYmzShield::setToneMidi()
 * 
 * Sets the tone period of a channel to produce the given MIDI note.
 */
void hcYmzShield::setToneMidi(uint8_t channel, uint16_t note) {
  #ifdef __FAVOR_PRECISION
  uint16_t tp = tpMidi[note];
  #else
  uint16_t tp = ((note > 12) ? (tpMidi[note%12] >> (note/12)) : tpMidi[note]);
  #endif
  
  if(channel > 2) {
    _setRegisterPsg1(((channel -= 3) *= 2), tp & 0xff);
    _setRegisterPsg1((++channel), tp >> 8);
  }
  else {
    _setRegisterPsg0((channel *= 2), tp & 0xff);
    _setRegisterPsg0((++channel), tp >> 8);
  }
}


/**
 * public hcYmzShield::setNoisePeriod()
 * 
 * Sets the noise period (NP) for the noise generator of the YMZ284. The
 * formula for the frequency of output is:
 * 
 * Hz = (4 * 10^6) / (32 * NP)
 */
void hcYmzShield::setNoisePeriod(uint8_t np) {
  _setRegisterPsg(0x06, np & B00011111); // Sanitize and write
}


/**
 * public hcYmzShield::getNoisePeriod()
 * 
 * Returns the current noise period of the shield.
 */
uint8_t hcYmzShield::getNoisePeriod() {
  return(_psg0Registers[0x06]);
}


/**
 * public hcYmzShield::setNoiseFrequency()
 * 
 * Sets the noise period to a given frequency in Hz.
 */
void hcYmzShield::setNoiseFrequency(float hz) {
  uint16_t np = 125000 / hz;
  
  _setRegisterPsg(0x06, np & B00011111); // Sanitize and write
}


/**
 * public hcYmzShield::setEnvelopePeriod()
 * 
 * Adjust the envelope period (EP) to adjust how frequently the noise
 * envelope repeats.
 * 
 * Hz = (4 * 10^6) / (256 * EP)
 */
void hcYmzShield::setEnvelopePeriod(uint16_t ep) {
  _setRegisterPsg(0x0b, ep & 0xff);
  _setRegisterPsg(0x0c, ep >> 8);
}


/**
 * public hcYmzShield::getEnvelopePeriod()
 * 
 * Returns the current envelope period of the shield.
 */
uint16_t hcYmzShield::getEnvelopePeriod() {
  return(_psg0Registers[0x0b] + (_psg0Registers[0x0c] << 8));
}


/**
 * public hcYmzShield::setEnvelopeFrequency()
 * 
 * Sets the envelope period to a given frequency in Hz.
 */
void hcYmzShield::setEnvelopeFrequency(float hz) {
  uint16_t ep = 7812.5 / hz;
  
  _setRegisterPsg(0x0b, ep & 0xff);
  _setRegisterPsg(0x0c, ep >> 8);
}


/**
 * public hcYmzShield::startEnvelope()
 * 
 * Sets the shape of the noise envelope. Only the lower 4 bits of the
 * register are used. The bits toggle CONTINUE, ATTACK, ALTERNATE and HOLD.
 */
void hcYmzShield::startEnvelope(uint8_t shape) {
  _setRegisterPsg(0x0d, shape & 0xf); // Sanitize
}


/**
 * public hcYmzShield::restartEnvelope()
 * 
 * Resets the current envelope using the existing shape.
 */
void hcYmzShield::restartEnvelope() {
  startEnvelope(_psg0Registers[0x0d]);
}


/**
 * public hcYmzShield::setTone()
 * 
 * Enables or disables tone output on a channel.
 */
void hcYmzShield::setTone(uint8_t channel, bool isEnabled) {
  if(channel > 2)
    _setRegisterPsg1(0x07, (isEnabled ? _psg1Registers[0x07] & ~(1 << (channel - 3)) : _psg1Registers[0x07] | (1 << (channel - 3))) & 0x3f);
  else
    _setRegisterPsg0(0x07, (isEnabled ? _psg0Registers[0x07] & ~(1 << channel) : _psg0Registers[0x07] | (1 << channel)) & 0x3f);
}


/**
 * public hcYmzShield::isTone()
 * 
 * Returns bool true if the current channel is being used for tone output.
 */
bool hcYmzShield::isTone(uint8_t channel) {
  if(channel > 2)
    return(((_psg1Registers[0x07] & (1 << (channel - 3))) == 0) ? true : false);
  else
    return(((_psg0Registers[0x07] & (1 << channel)) == 0) ? true : false);
}


/**
 * public hcYmzShield::setNoise()
 * 
 * Enables or disables noise output on a channel.
 */
void hcYmzShield::setNoise(uint8_t channel, bool isEnabled) {
  if(channel > 2)
    _setRegisterPsg1(0x07, (isEnabled ? _psg1Registers[0x07] & ~(1 << channel) : _psg1Registers[0x07] | (1 << channel)) & 0x3f);
  else
    _setRegisterPsg0(0x07, (isEnabled ? _psg0Registers[0x07] & ~(1 << (channel + 3)) : _psg0Registers[0x07] | (1 << (channel + 3))) & 0x3f);
}


/**
 * public hcYmzShield::isNoise()
 * 
 * Returns bool true if the current channel is being used for noise output.
 */
bool hcYmzShield::isNoise(uint8_t channel) {
  if(channel > 2)
    return(((_psg1Registers[0x07] & (1 << ((channel-3) * 3))) == 0) ? true : false);
  else
    return(((_psg0Registers[0x07] & (1 << (channel * 3))) == 0) ? true : false);  
}


/**
 * public hcYmzShield::mute()
 * 
 * Toggle sound channels off.
 */
void hcYmzShield::mute() {
  _tone = B00111111;
  _setRegisterPsg(0x07, B00111111);
}

void hcYmzShield::setVolumeByEnvelope(uint8_t channel) {
    
    if(channel > 2)
        _setRegisterPsg1(0x08 + (channel - 3), 16);
    else
        _setRegisterPsg0(0x08 + channel, 16);
    
}

/**
 * public hcYmzShield::setVolume()
 * 
 * Adjust the value of each channel.
 */
void hcYmzShield::setVolume(uint8_t channel, uint8_t volume, bool fakeMute) {
  volume &= 0xf; // Sanitize
  if(!fakeMute)
    _volume[channel] = volume;
  
  if(channel > 2)
    _setRegisterPsg1(0x08 + (channel - 3), volume + (_psg1Registers[0x08] & 0x10));
  else
    _setRegisterPsg0(0x08 + channel, volume + (_psg0Registers[0x08] & 0x10));
}


/**
 * public hcYmzShield::getVolume()
 * 
 * Returns the volume of a channel.
 */
byte hcYmzShield::getVolume(uint8_t channel) {
  if(channel > 2)
    return(_psg1Registers[0x08 + (channel - 3)]);
  else
    return(_psg0Registers[0x08 + channel]);
}


/**
 * public hcYmzShield::setVolume()
 * 
 * Adjust the value of all channels.
 */
void hcYmzShield::setVolume(uint8_t volume) {
  for(uint8_t i = 0; i < 6; i++)
    setVolume(i, volume);
}


/**
 * public hcYmzShield::setEnvelope()
 * 
 * Enables or disables mixing a channel through the envelope generator.
 */
void hcYmzShield::setEnvelope(uint8_t channel, bool isEnabled) {
  uint8_t data;
  if(channel > 2) {
    data = (isEnabled ? _psg1Registers[0x08] | 0x10 : _psg1Registers[0x08] & ~0x10);
    _setRegisterPsg1(0x08, data & 0x1f);
  }
  else {
    data = (isEnabled ? _psg0Registers[0x08] | 0x10 : _psg0Registers[0x08] & ~0x10);
    _setRegisterPsg0(0x08, data & 0x1f);  
  }
}


/**
 * public hcYmzShield::isEnvelope()
 * 
 * Returns bool true if the current channel is being used for noise output.
 */
bool hcYmzShield::isEnvelope(uint8_t channel) {  
  if(channel > 2)
    return(((_psg1Registers[0x08] & 0x10) == 1) ? true : false);
  else
    return(((_psg0Registers[0x08] & 0x10) == 1) ? true : false);
}


/**
 * public hcYmzShield::setChannels()
 * 
 * Sets all six channels of the chip in one shot using MIDI notes. 255 is
 * understood as OFF; 128 is understood as SKIP.
 * 
 * We will *NOT* waste clock cycles here to sanitize input.
 */
void hcYmzShield::setChannels(uint8_t c0, uint8_t c1, uint8_t c2, uint8_t c3, uint8_t c4, uint8_t c5) {
  uint8_t channel[6] = {c0, c1, c2, c3, c4, c5};
  uint8_t i, state = 0;

  // First turn off any notes we will change
  for(i = 0; i < 6; i++) {
    if(channel[i] != SKIP)
      state |= (1 << i);
    if(channel[i] == OFF) {
      _tone |= (1 << i);
      setVolume(i, 0, true);
    }
    else if(channel[i] < 128)
      _tone &= ~(1 << i);
  }
  
  _setRegisterPsg0(0x07, (_psg0Registers[0x07] & ~B00000111) | (state & B00000111));
  _setRegisterPsg1(0x07, (_psg1Registers[0x07] & ~B00000111) | (state >> 3));
  
  // Pause for articulation
  delay(_articulation);
  
  // Now set the new notes
  for(i = 0; i < 6; i++) {
    if(channel[i] != OFF && channel[i] != SKIP) {
      setToneMidi(i, channel[i]);
      setVolume(i, _volume[i]);
    }
  }
  
  _setRegisterPsg0(0x07, (_psg0Registers[0x07] & ~B00000111) | (_tone & B00000111));
  _setRegisterPsg1(0x07, (_psg1Registers[0x07] & ~B00000111) | (_tone >> 3));

}


/**
 * public hcYmzShield::setNote()
 * 
 * Sets a single channel to the given MIDI note. 255 is understood as OFF.
 * 
 * We will *NOT* waste clock cycles here to sanitize input.
 */
void hcYmzShield::setNote(uint8_t channel, uint8_t note) {
  // First turn off the channel
  setTone(channel, false);
    
  // Set the note and turn the channel on
  if(note != OFF) {
    setToneMidi(channel, note);
    
    // Pause for articulation
    delay(_articulation);
    
    setTone(channel);
  }
}


/**
 * public hcYmzShield::setTempo()
 * 
 * Sets the current tempo in beats per minute (BPM).
 */
void hcYmzShield::setTempo(uint8_t bpm) {
  _bpm = bpm;
}


/**
 * public hcYmzShield::getTempo()
 * 
 * Returns the current tempo in beats per minute (BPM).
 */
uint8_t hcYmzShield::getTempo() {
  return(_bpm);
}


/**
 * public hcYmzShield::setArticulation()
 * 
 * Set the articulation between notes to STACCATO or LEGATO. Call it with no
 * parameters to return to default spacing.
 */
void hcYmzShield::setArticulation(uint8_t tonguing) {
  _articulation = tonguing;
}


/**
 * public hcYmzShield::beat()
 * 
 * Delays the number of milliseconds relative to 1/beat at the current tempo.
 * For example, at the default tempo of MODERATO, beat(4) would delay for
 * 666 milliseconds, or one quarter note.
 * 
 * For dotted notes, you can pass DOT, DOUBLEDOT or TRIPLEDOT as the second
 * parameter to extend the length.
 *
 * There are 60,000 milliseconds per minute.
 */
void hcYmzShield::beat(uint8_t beat, uint8_t dot) {
  // Subtract articulation to keep beat count
  delay((((60000/_bpm) * 4)/beat) * (float(dot)/8) - _articulation);
}



/**
 * public hcYmzShield::playBlock()
 * 
 * Plays a Hardchord Music block that has been encoded and stored as a byte
 * array. This can be an efficient way to store very large amounts of music
 * data in a single project.
 * 
 * You should be aware that shorter songs will not benefit from this function.
 * A longer song that compiled to 24KB may fit within 10KB when using converted
 * for playBlock(). However, something like the short Duck Hunt demo becomes
 * 1KB larger due to this function's overhead.
 * 
 * You should carefully weigh the benefits of using this function in any
 * programs that have severe space constraints.
 */
void hcYmzShield::playBlock(const uint8_t *song) {
  uint32_t i;
  uint8_t command;
  if(pgm_read_byte(song) == 0x48 && pgm_read_byte(song + 1) == 0x43) {
    if(song[2] < 2) { // Support <= Revision 1
      command = pgm_read_byte(song + 3);
      i = 4;
      while(command != 0) {
        switch(command) {
          // Set volume on all channels
          case 0x50:
            setVolume(pgm_read_byte(song + i));
            i++;
            break;
          // Set volume on one channel
          case 0x51:
            setVolume(pgm_read_byte(song + i), pgm_read_byte(song + i + 1));
            i += 2;
            break;
          // Set tempo
          case 0x52:
            setTempo(pgm_read_byte(song + i));
            i++;
            break;
          // Set articulation
          case 0x53:
            setArticulation(pgm_read_byte(song + i));
            i++;
            break;
          
          // Mute all channels
          case 0x60:
            mute();
            break;
          // Toggle tone on channel
          case 0x61:
            setTone(pgm_read_byte(song + i), !!pgm_read_byte(song + i + 1));
            i += 2;
            break;
          // Toggle noise on channel
          case 0x62:
            setNoise(pgm_read_byte(song + i), !!pgm_read_byte(song + i + 1));
            i += 2;
            break;
          // Toggle envelope on channel
          case 0x63:
            setEnvelope(pgm_read_byte(song + i), !!pgm_read_byte(song + i + 1));
            i += 2;
            break;
          
          // Start envelope generator with ADSR envelope
          case 0x70:
            startEnvelope(pgm_read_byte(song + i));
            i++;
            break;
          // Restart current ADSR envelope
          case 0x71:
            restartEnvelope();
            break;
          // Set envelope period
          case 0x73:
            setEnvelopePeriod((pgm_read_byte(song + i) << 8) + pgm_read_byte(song + i + 1));
            i += 2;
            break;
          
          // Set tone period
          case 0x80:
            setTonePeriod(pgm_read_byte(song + i), (pgm_read_byte(song + i + 1) << 8) + pgm_read_byte(song + i + 2));
            i += 3;
            break;
          // Set tone MIDI
          case 0x81:
            setToneMidi(pgm_read_byte(song + i), pgm_read_byte(song + i + 1));
            i += 2;
            break;
          // Set note
          case 0x82:
            setNote(pgm_read_byte(song + i), pgm_read_byte(song + i + 1));
            i += 2;
            break;
          // Set channels
          case 0x83:
            setChannels(pgm_read_byte(song + i), pgm_read_byte(song + i + 1), pgm_read_byte(song + i + 2), pgm_read_byte(song + i + 3), pgm_read_byte(song + i + 4), pgm_read_byte(song + i + 5));
            i += 6;
            break;
          
          // Set noise period
          case 0x90:
            setNoisePeriod(pgm_read_byte(song + i));
            i++;
            break;
          
          // Pause for a beat
          case 0xa0:
            beat(pgm_read_byte(song + i), pgm_read_byte(song + i + 1));
            i += 2;
            break;
          // Delay
          case 0xa1:
            delay((pgm_read_byte(song + i) << 8) + pgm_read_byte(song + i + 1));
            i += 2;
            break;
        }
        command = pgm_read_byte(song + i);
        i++;
      }
    }
  }
}
