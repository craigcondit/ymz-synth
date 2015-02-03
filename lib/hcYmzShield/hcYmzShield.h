/**
 * Hardchord YMZ Shield 1.0 (hcYmzShield.h)
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


#ifndef __HCYMZSHIELD_H
#define __HCYMZSHIELD_H

#if defined(ARDUINO) && ARDUINO >= 100
#include <Arduino.h>
#else
#include <WProgram.h>
#endif

// Uncomment this if you mod your board for SPI access. SPI Pinning is:
// * CS1  (YMZ284#1 PIN  1)  = 2
// * CS2  (YMZ284#2 PIN  2)  = 3
// * SEL  (YMZ284#2 PIN  3)  = 8
// * RCK  (74HC595  PIN 12)  = 9
// * SER  (74HC595  PIN 14) = 11
// * SRCK (74HC595  PIN 11) = 13
// Pin 10 must be kept free. Setting Pin 10 LOW will kill all SPI devices.
#define __SPI_HACK

// Envelope controls
#define CONT B00001000
#define ATT  B00000100
#define ALT  B00000010
#define HOLD B00000001

// YMZ Shield pinning masks for AVR
#define MASK_SER  B00000100
#define MASK_RCK  B00001000
#define MASK_SRCK B00010000
#define MASK_CS1  B00000100
#define MASK_SEL  B00001000
#define MASK_CS2  B00010000

// YMZ Shield pinning masks for digitalWrite
#define PIN_SER  2
#define PIN_RCK  3
#define PIN_SRCK 4
#define PIN_CS1  10
#define PIN_SEL  11
#define PIN_CS2  12

// These are used by all Hardchord shields
#ifndef __HCINTERNALS
#define __HCINTERNALS

// Special note states
#define SKIP 128
#define OFF  255

// Used for dotted notes
#define DOT 12
#define DOUBLEDOT 14
#define TRIPLEDOT 15

// Tempo
#define LARGHISSIMO 16
#define GRAVE 30
#define LENTO 42
#define LARGO 48
#define LARGHETTO 52
#define ADAGIO 60
#define ADAGIETTO 68
#define ANDANTE 75
#define ANDANTINO 80
#define MODERATO 90
#define ALEGRETTO 102
#define ALLEGRO 120
#define VIVACE 136
#define VIVACISSIMO 146
#define ALLEGRISSIMO 160
#define PRESTO 174
#define PRESTISSIMO 180

// Articulation
#define STACCATO 20
#define LEGATO 0

#endif __HCINTERNALS

class hcYmzShield {
  public:
    hcYmzShield();
    void setTonePeriod(uint8_t, uint16_t);
    uint16_t getTonePeriod(uint8_t);
    void setToneFrequency(uint8_t, float);
    void setToneMidi(uint8_t, uint16_t);
    void setNoisePeriod(uint8_t);
    uint8_t getNoisePeriod();
    void setNoiseFrequency(float);
    void mute();
    void setEnvelopePeriod(uint16_t);
    uint16_t getEnvelopePeriod();
    void setEnvelopeFrequency(float);
    void startEnvelope(uint8_t);
    void restartEnvelope();
    void setTone(uint8_t, bool = true);
    bool isTone(uint8_t);
    void setNoise(uint8_t, bool = true);
    bool isNoise(uint8_t);
    void setEnvelope(uint8_t, bool = true);
    bool isEnvelope(uint8_t);
    void setVolume(uint8_t, uint8_t, bool = false);
    void setVolume(uint8_t);
    void setVolumeByEnvelope(uint8_t);
    uint8_t getVolume(uint8_t);
    void setChannels(uint8_t, uint8_t = OFF, uint8_t = OFF, uint8_t = OFF, uint8_t = OFF, uint8_t = OFF);
    void setNote(uint8_t, uint8_t);
    void setTempo(uint8_t);
    void setArticulation(uint8_t = 8);
    uint8_t getTempo();
    void beat(uint8_t, uint8_t = 8);
    void playBlock(const uint8_t*);
    void setRegisterPsg(uint8_t, uint8_t);
    void setRegisterPsg0(uint8_t, uint8_t);
    void setRegisterPsg1(uint8_t, uint8_t);
  private:
    uint8_t _psg0Registers[0x0d];
    uint8_t _psg1Registers[0x0d];
    uint8_t _volume[6];
    uint8_t _tone;
    uint8_t _bpm;
    uint8_t _articulation;
    void _setRegisterPsg(uint8_t, uint8_t);
    void _setRegisterPsg0(uint8_t, uint8_t);
    void _setRegisterPsg1(uint8_t, uint8_t);
    inline static void _shiftOut(uint8_t);
    inline static void _busAddress();
    inline static void _debugLightOn();
    inline static void _debugLightOff();
    inline static void _busData();
    inline static void _psgWrite();
    inline static void _psg0Write();
    inline static void _psg1Write();
};

extern hcYmzShield YMZ;


#endif __HCYMZSHIELD_H
