#include "ymz_synth.h"

#define RED_LED 7
#define GREEN_LED 6
#define PINK_LED 5
#define WHITE_LED 4

#define LED_COUNT 4

// MIDI channels - standard
#define CHANNEL_STEREO 1
#define CHANNEL_LEFT 2
#define CHANNEL_RIGHT 3

// MIDI channels - noise
#define CHANNEL_NOISE_STEREO 4
#define CHANNEL_NOISE_LEFT 5
#define CHANNEL_NOISE_RIGHT 6

// MIDI channels - raw
#define CHANNEL_RAW_STEREO 7
#define CHANNEL_RAW_LEFT 8
#define CHANNEL_RAW_RIGHT 9

// CC #s
#define CC_CHANNEL_A_FREQ_MSB 20
#define CC_CHANNEL_A_FREQ_LSB 52
#define CC_CHANNEL_B_FREQ_MSB 21
#define CC_CHANNEL_B_FREQ_LSB 53
#define CC_CHANNEL_C_FREQ_MSB 22
#define CC_CHANNEL_C_FREQ_LSB 54
#define CC_NOISE_FREQ 23
#define CC_MIXER 24
#define CC_CHANNEL_A_LEVEL 25
#define CC_CHANNEL_B_LEVEL 26
#define CC_CHANNEL_C_LEVEL 27
#define CC_ENVELOPE_FREQ_HIGH 28 // high 7 bits
#define CC_ENVELOPE_FREQ_MED 29  // middle 7 bits
#define CC_ENVELOPE_FREQ_LOW 30  // low 2 bits
#define CC_ENVELOPE_SHAPE 31
#define CC_LATCH 80
#define CC_DEBUG 119

const byte hex[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B',
		'C', 'D', 'E', 'F' };

MIDI_CREATE_DEFAULT_INSTANCE();

// define an array of LEDs so we can do patterns (left to right)
const int LEDS[LED_COUNT] = { RED_LED, GREEN_LED, PINK_LED, WHITE_LED };
volatile unsigned long decay[LED_COUNT] = { 0, 0, 0, 0 };
volatile uint8_t rawRegisters0[0xd];
volatile uint8_t rawRegisters1[0xd];
volatile bool latched = false;

// wrapper functions to allow pointer to functions

void setRegisterPsg(byte reg, byte value) {
	YMZ.setRegisterPsg(reg, value);
}

void setRegisterPsg0(byte reg, byte value) {
	YMZ.setRegisterPsg0(reg, value);
}

void setRegisterPsg1(byte reg, byte value) {
	YMZ.setRegisterPsg1(reg, value);
}

byte getRegisterPsg(byte reg) {
	return YMZ.getRegisterPsg(reg);
}

byte getRegisterPsg0(byte reg) {
	return YMZ.getRegisterPsg0(reg);
}

byte getRegisterPsg1(byte reg) {
	return YMZ.getRegisterPsg1(reg);
}

// array of getter/setter pairs for the PSG registers -- used to allow per-MIDI channel mapping

const regSet setters[3] =
		{ &setRegisterPsg, &setRegisterPsg1, &setRegisterPsg0 };
const regGet getters[3] =
		{ &getRegisterPsg, &getRegisterPsg1, &getRegisterPsg0 };

/**
 * Is the given MIDI channel used for normal music output?
 */
bool inline isMusicMode(byte channel) {
	return (channel == CHANNEL_STEREO || channel == CHANNEL_LEFT
			|| channel == CHANNEL_RIGHT);
}

/**
 * Is the given MIDI channel used for noise output?
 */
bool inline isNoiseMode(byte channel) {
	return (channel == CHANNEL_NOISE_STEREO || channel == CHANNEL_NOISE_LEFT
			|| channel == CHANNEL_NOISE_RIGHT);
}

/**
 * Is the given MIDI channel used for raw register output?
 */
bool inline isRawMode(byte channel) {
	return (channel == CHANNEL_RAW_STEREO || channel == CHANNEL_RAW_LEFT
			|| channel == CHANNEL_RAW_RIGHT);
}

/**
 * Pulse the red LED when MIDI activity is generated.
 */
void midiActivity(int led) {
	digitalWrite(led, HIGH);
	for (int i = 0; i < LED_COUNT; i++) {
		if (LEDS[i] == led) {
			decay[i] = millis() + 10L;
		}
	}
}

/**
 * Decay the LEDs
 */
void decayLeds() {
	unsigned long time = millis();
	for (int i = 0; i < LED_COUNT; i++) {
		if (decay[i] < time || decay[i] > (time + 30000L)) {
			decay[i] = 0L;
			digitalWrite(LEDS[i], LOW);
		}
	}
}

void debugMidi(byte value) {
	MIDI.sendControlChange(CC_DEBUG, value & 0x7f, 1);
}

void debugMidiBinary(byte value) {
	for (int i = 7; i >= 0; i--) {
		byte send = (value >> i) & 0x01;
		MIDI.sendControlChange(CC_DEBUG, (send == 0) ? '0' : '1', 1);
		if (i == 4) {
			MIDI.sendControlChange(CC_DEBUG, ' ', 1);
		}
	}
}

void debugMidiHex(byte value) {
	byte high = (value & B11110000) >> 4;
	byte low = (value & B00001111);
	MIDI.sendControlChange(CC_DEBUG, hex[high], 1);
	MIDI.sendControlChange(CC_DEBUG, hex[low], 1);
}

void debugMidiStr(char * value) {
	for (int i = 0; i < strlen(value); i++) {
		MIDI.sendControlChange(CC_DEBUG, value[i], 1);
	}
}

/**
 * Process MIDI NOTE ON messages.
 */
void handleNoteOn(byte channel, byte pitch, byte velocity) {
	if (!isMusicMode(channel)) {
		return;
	}
	switch (channel) {
	case CHANNEL_STEREO:
		midiActivity(RED_LED);
		midiActivity(GREEN_LED);
		break;
	case CHANNEL_LEFT:
		midiActivity(RED_LED);
		break;
	case CHANNEL_RIGHT:
		midiActivity(GREEN_LED);
		break;
	}

	// TODO handle multiple channels (stereo/left/right) as well as polyphony.
	YMZ.setNote(0, pitch);
	YMZ.setNote(1, pitch + 4);
	YMZ.setNote(2, pitch + 7);
	YMZ.setNote(3, pitch);
	YMZ.setNote(4, pitch + 4);
	YMZ.setNote(5, pitch + 7);
	uint16_t period = YMZ.getTonePeriod(0);
}

void handleNoteOff(byte channel, byte pitch, byte velocity) {
	if (!isMusicMode(channel)) {
		return;
	}
	switch (channel) {
	case CHANNEL_STEREO:
		midiActivity(RED_LED);
		midiActivity(GREEN_LED);
		break;
	case CHANNEL_LEFT:
		midiActivity(RED_LED);
		break;
	case CHANNEL_RIGHT:
		midiActivity(GREEN_LED);
		break;
	}

	// TODO handle multiple channels (stereo/left/right) as well as polyphony.

	YMZ.setNote(0, OFF);
	YMZ.setNote(1, OFF);
	YMZ.setNote(2, OFF);
	YMZ.setNote(3, OFF);
	YMZ.setNote(4, OFF);
	YMZ.setNote(5, OFF);
}

void writeAllRegisters(byte channel) {
	if (!isRawMode(channel)) {
		return;
	}
	for (int i = 0; i < 14; i++) {
		switch (channel) {
		case CHANNEL_RAW_STEREO:
			setRegisterPsg1(i, rawRegisters1[i]);
			setRegisterPsg0(i, rawRegisters0[i]);
			break;
		case CHANNEL_RAW_LEFT:
			setRegisterPsg1(i, rawRegisters1[i]);
			break;
		case CHANNEL_RAW_RIGHT:
			setRegisterPsg0(i, rawRegisters0[i]);
			break;
		}
	}
}

void setChannelFreqMsb(byte channel, byte reg, byte value) {
	if (!isRawMode(channel)) {
		return;
	}

	// get current value
	uint8_t oldFine = getters[channel - CHANNEL_RAW_STEREO](reg);
	uint8_t oldRough = getters[channel - CHANNEL_RAW_STEREO](reg + 1);

	// shift lower 4 bits of oldRough measurement up 8 bits and add oldFine; giving 12-bit number [0..4095]
	uint16_t buf = (uint16_t) (oldRough & B00001111);
	buf <<= 8;
	buf += oldFine;

	// mask off bits we aren't changing (lower 5 bits)
	buf &= 0x001f;

	// take new value, use as most significant 7 bits
	uint16_t newValue = (uint16_t) (value & B01111111) << 5;

	// add bits we are
	buf |= newValue;

	// split to new MSB/LSB
	uint8_t newFine = buf & 0xff; // lower 8 bits
	uint8_t newRough = ((uint8_t) (buf >> 8)) & B00001111; // upper 4 bits

	// update
	switch (channel) {
	case CHANNEL_RAW_STEREO:
		rawRegisters0[reg] = newFine;
		rawRegisters0[reg + 1] = newRough;
		rawRegisters1[reg] = newFine;
		rawRegisters1[reg + 1] = newRough;
		break;
	case CHANNEL_RAW_LEFT:
		rawRegisters1[reg] = newFine;
		rawRegisters1[reg + 1] = newRough;
		break;
	case CHANNEL_RAW_RIGHT:
		rawRegisters0[reg] = newFine;
		rawRegisters0[reg + 1] = newRough;
		break;
	}
	if (!latched) {
		setters[channel - CHANNEL_RAW_STEREO](reg, newFine);
		setters[channel - CHANNEL_RAW_STEREO](reg + 1, newRough);
	}
}

void setChannelFreqLsb(byte channel, byte reg, byte value) {
	if (!isRawMode(channel)) {
		return;
	}

	// get current value
	uint8_t oldFine = getters[channel - CHANNEL_RAW_STEREO](reg);
	uint8_t oldRough = getters[channel - CHANNEL_RAW_STEREO](reg + 1);

	// shift lower 4 bits of oldRough measurement up 8 bits and add oldFine; giving 12-bit number [0..4095]
	uint16_t buf = (uint16_t) (oldRough & B00001111);
	buf <<= 8;
	buf += oldFine;

	// mask off bits we aren't changing (upper 7 bits)
	buf &= 0x0fe0; // 0000 1111 1110 0000

	// take new value, use as least significant 5 bits
	uint16_t newValue = ((uint16_t) (value & B01111100)) >> 2;

	// add bits we are
	buf |= newValue;

	// split to new MSB/LSB
	uint8_t newFine = buf & 0xff; // lower 8 bits
	uint8_t newRough = ((uint8_t) (buf >> 8)) & B00001111; // upper 4 bits

	// update
	switch (channel) {
	case CHANNEL_RAW_STEREO:
		rawRegisters0[reg] = newFine;
		rawRegisters0[reg + 1] = newRough;
		rawRegisters1[reg] = newFine;
		rawRegisters1[reg + 1] = newRough;
		break;
	case CHANNEL_RAW_LEFT:
		rawRegisters1[reg] = newFine;
		rawRegisters1[reg + 1] = newRough;
		break;
	case CHANNEL_RAW_RIGHT:
		rawRegisters0[reg] = newFine;
		rawRegisters0[reg + 1] = newRough;
		break;
	}
	if (!latched) {
		setters[channel - CHANNEL_RAW_STEREO](reg, newFine);
		setters[channel - CHANNEL_RAW_STEREO](reg + 1, newRough);
	}
}

void setEnvelopeFreqHigh(byte channel, byte value) {
	if (!isRawMode(channel)) {
		return;
	}

	// get current value
	uint8_t oldFine = getters[channel - CHANNEL_RAW_STEREO](0x0b);
	uint8_t oldRough = getters[channel - CHANNEL_RAW_STEREO](0x0c);

	// shift lower 8 bits of oldRough measurement up 8 bits and add oldFine; giving 16-bit number
	uint16_t buf = oldRough;
	buf <<= 8;
	buf += oldFine;

	// mask off bits we aren't changing (bits 9-15)
	buf &= 0x01ff; // 0000 0001 1111 1111

	// take new value, use as bits 9-15
	uint16_t newValue = value & B01111111;
	newValue <<= 9;

	// add bits we are
	buf |= newValue;

	// split to new MSB/LSB
	uint8_t newFine = buf & 0xff; // lower 8 bits
	uint8_t newRough = ((uint8_t) (buf >> 8)); // upper 8 bits

	// update
	switch (channel) {
	case CHANNEL_RAW_STEREO:
		rawRegisters0[0xb] = newFine;
		rawRegisters0[0xc] = newRough;
		rawRegisters1[0xb] = newFine;
		rawRegisters1[0xc] = newRough;
		break;
	case CHANNEL_RAW_LEFT:
		rawRegisters1[0xb] = newFine;
		rawRegisters1[0x6] = newRough;
		break;
	case CHANNEL_RAW_RIGHT:
		rawRegisters0[0xb] = newFine;
		rawRegisters0[0x6] = newRough;
		break;
	}
	if (!latched) {
		setters[channel - CHANNEL_RAW_STEREO](0xb, newFine);
		setters[channel - CHANNEL_RAW_STEREO](0xc, newRough);
	}
}

void setEnvelopeFreqMed(byte channel, byte value) {
	if (!isRawMode(channel)) {
		return;
	}

	// get current value
	uint8_t oldFine = getters[channel - CHANNEL_RAW_STEREO](0x0b);
	uint8_t oldRough = getters[channel - CHANNEL_RAW_STEREO](0x0c);

	// shift lower 8 bits of oldRough measurement up 8 bits and add oldFine; giving 16-bit number
	uint16_t buf = oldRough;
	buf <<= 8;
	buf += oldFine;

	// mask off bits we aren't changing (bits 2-8)
	buf &= 0xfe03; // 1111 1110 0000 0011

	// take new value, use as bits 2-8
	uint16_t newValue = value & B01111111;
	newValue <<= 2;

	// add bits we are
	buf |= newValue;

	// split to new MSB/LSB
	uint8_t newFine = buf & 0xff; // lower 8 bits
	uint8_t newRough = ((uint8_t) (buf >> 8)); // upper 8 bits

	// update
	switch (channel) {
	case CHANNEL_RAW_STEREO:
		rawRegisters0[0xb] = newFine;
		rawRegisters0[0xc] = newRough;
		rawRegisters1[0xb] = newFine;
		rawRegisters1[0xc] = newRough;
		break;
	case CHANNEL_RAW_LEFT:
		rawRegisters1[0xb] = newFine;
		rawRegisters1[0x6] = newRough;
		break;
	case CHANNEL_RAW_RIGHT:
		rawRegisters0[0xb] = newFine;
		rawRegisters0[0x6] = newRough;
		break;
	}
	if (!latched) {
		setters[channel - CHANNEL_RAW_STEREO](0xb, newFine);
		setters[channel - CHANNEL_RAW_STEREO](0xc, newRough);
	}
}

void setEnvelopeFreqLow(byte channel, byte value) {
	if (!isRawMode(channel)) {
		return;
	}

	// get current value
	uint8_t oldFine = getters[channel - CHANNEL_RAW_STEREO](0x0b);
	uint8_t oldRough = getters[channel - CHANNEL_RAW_STEREO](0x0c);

	// shift lower 8 bits of oldRough measurement up 8 bits and add oldFine; giving 16-bit number
	uint16_t buf = oldRough;
	buf <<= 8;
	buf += oldFine;

	// mask off bits we aren't changing (bits 0-1)
	buf &= 0xfffc; // 0000 0000 0000 0011

	// take new value, use as bits 0-2
	uint16_t newValue = value & B01100000;
	newValue >>= 5;

	// add bits we are
	buf |= newValue;

	// split to new MSB/LSB
	uint8_t newFine = buf & 0xff; // lower 8 bits
	uint8_t newRough = ((uint8_t) (buf >> 8)); // upper 8 bits

	// update
	switch (channel) {
	case CHANNEL_RAW_STEREO:
		rawRegisters0[0xb] = newFine;
		rawRegisters0[0xc] = newRough;
		rawRegisters1[0xb] = newFine;
		rawRegisters1[0xc] = newRough;
		break;
	case CHANNEL_RAW_LEFT:
		rawRegisters1[0xb] = newFine;
		rawRegisters1[0x6] = newRough;
		break;
	case CHANNEL_RAW_RIGHT:
		rawRegisters0[0xb] = newFine;
		rawRegisters0[0x6] = newRough;
		break;
	}
	if (!latched) {
		setters[channel - CHANNEL_RAW_STEREO](0xb, newFine);
		setters[channel - CHANNEL_RAW_STEREO](0xc, newRough);
	}
}

void setRegister(byte channel, byte reg, byte value) {
	switch (channel) {
	case CHANNEL_RAW_STEREO:
		rawRegisters0[reg] = value;
		rawRegisters1[reg] = value;
		if (!latched) {
			YMZ.setRegisterPsg(reg, value);
		}
		break;
	case CHANNEL_RAW_LEFT:
		rawRegisters1[reg] = value;
		if (!latched) {
			YMZ.setRegisterPsg1(reg, value);
		}
		break;
	case CHANNEL_RAW_RIGHT:
		rawRegisters0[reg] = value;
		if (!latched) {
			YMZ.setRegisterPsg0(reg, value);
		}
		break;
	default:
		return;
	}
}

void handleControlChange(byte channel, byte number, byte value) {
	if (!isRawMode(channel)) {
		return;
	}
	switch (channel) {
	case CHANNEL_RAW_STEREO:
		midiActivity(PINK_LED);
		midiActivity(WHITE_LED);
		break;
	case CHANNEL_RAW_LEFT:
		midiActivity(PINK_LED);
		break;
	case CHANNEL_RAW_RIGHT:
		midiActivity(WHITE_LED);
		break;
	}

	value &= B01111111; // make 7-bit clean

	switch (number) {
	case CC_CHANNEL_A_FREQ_MSB:
		setChannelFreqMsb(channel, 0x00, value);
		break;
	case CC_CHANNEL_B_FREQ_MSB:
		setChannelFreqMsb(channel, 0x02, value);
		break;
	case CC_CHANNEL_C_FREQ_MSB:
		setChannelFreqMsb(channel, 0x04, value);
		break;
	case CC_CHANNEL_A_FREQ_LSB:
		setChannelFreqLsb(channel, 0x00, value);
		break;
	case CC_CHANNEL_B_FREQ_LSB:
		setChannelFreqLsb(channel, 0x02, value);
		break;
	case CC_CHANNEL_C_FREQ_LSB:
		setChannelFreqLsb(channel, 0x04, value);
		break;
	case CC_NOISE_FREQ: // 5 bits
		setRegister(channel, 0x06, (value >> 2) & B00011111); // 7 -> 5 bits
		break;
	case CC_MIXER:
		setRegister(channel, 0x07, (value >> 1) & B00111111); // 7 -> 6 bits
		break;
	case CC_CHANNEL_A_LEVEL:
		setRegister(channel, 0x08, (value >> 2) & B00011111); // 7 -> 5 bits
		break;
	case CC_CHANNEL_B_LEVEL:
		setRegister(channel, 0x09, (value >> 2) & B00011111); // 7 -> 5 bits
		break;
	case CC_CHANNEL_C_LEVEL:
		setRegister(channel, 0x0a, (value >> 2) & B00011111); // 7 -> 5 bits
		break;
	case CC_ENVELOPE_FREQ_HIGH:
		setEnvelopeFreqHigh(channel, value);
		break;
	case CC_ENVELOPE_FREQ_MED:
		setEnvelopeFreqMed(channel, value);
		break;
	case CC_ENVELOPE_FREQ_LOW:
		setEnvelopeFreqLow(channel, value);
		break;
	case CC_ENVELOPE_SHAPE:
		setRegister(channel, 0x0d, (value >> 3) & B00001111); // 7 -> 4 bits
		break;
	case CC_LATCH:
		latched = (value > 64);
		if (!latched) {
			writeAllRegisters(channel);
		}
	}
}

void setup() {
	for (int i = 0; i < LED_COUNT; i++) {
		pinMode(LEDS[i], OUTPUT);
		digitalWrite(LEDS[i], LOW);
	}

	// clear all YMZ registers
	for (byte i = 0x00; i < 0x0d; i++) {
		YMZ.setRegisterPsg(i, 0x00);
	}

	// turn off all sound
	YMZ.mute();

	// set default tempo and articulation
	YMZ.setTempo(120);
	YMZ.setArticulation(LEGATO);

	// set event handlers
	MIDI.setHandleNoteOn(handleNoteOn);
	MIDI.setHandleNoteOff(handleNoteOff);
	MIDI.setHandleControlChange(handleControlChange);

	// listen to all channels
	MIDI.begin(MIDI_CHANNEL_OMNI);
	MIDI.turnThruOff();

	// default volume for music
	YMZ.setVolume(10);

	// let the user know we're ready to go by flashing all the lights
	for (int i = 0; i < LED_COUNT; i++) {
		digitalWrite(LEDS[i], HIGH);
		delay(250);
		digitalWrite(LEDS[i], LOW);
	}

}

void loop() {
	decayLeds();
	MIDI.read();
}

