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
#define CC_ENVELOPE_FREQ_MSB 28
#define CC_ENVELOPE_FREQ_LSB 60
#define CC_ENVELOPE_SHAPE 29
#define CC_ENVELOPE_VALUE_MSB 30
#define CC_ENVELOPE_VALUE_LSB 62
#define CC_CONTROL_POWER 31

MIDI_CREATE_DEFAULT_INSTANCE();

// define an array of LEDs so we can do patterns (left to right)
const int LEDS[LED_COUNT] = { RED_LED, GREEN_LED, PINK_LED, WHITE_LED };

volatile unsigned long redDecay = 0;

void midiActivity() {
	digitalWrite(RED_LED, HIGH);
	redDecay = millis() + 50;
}

void handleNoteOn(byte channel, byte pitch, byte velocity) {
	midiActivity();

	YMZ.setNote(0, pitch);
	YMZ.setNote(1, pitch + 4);
	YMZ.setNote(2, pitch + 7);
	YMZ.setNote(3, pitch);
	YMZ.setNote(4, pitch + 4);
	YMZ.setNote(5, pitch + 7);
	uint16_t period = YMZ.getTonePeriod(0);
}

void handleNoteOff(byte channel, byte pitch, byte velocity) {
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
}

bool inline isRawMode(byte channel) {
	return (channel == CHANNEL_RAW_STEREO || channel == CHANNEL_RAW_LEFT
			|| channel == CHANNEL_RAW_RIGHT);
}

void setEnvelopeValueMsb(byte channel, byte reg, byte value) {
	// get current value
	uint8_t prevFine;
	uint8_t prevCoarse;
	switch (channel) {
	case CHANNEL_RAW_STEREO:
		prevCoarse = YMZ.getRegisterPsg(reg + 1);
		prevFine = YMZ.getRegisterPsg(reg);
		break;
	case CHANNEL_RAW_LEFT:
		prevCoarse = YMZ.getRegisterPsg1(reg + 1);
		prevFine = YMZ.getRegisterPsg1(reg);
		break;
	case CHANNEL_RAW_RIGHT:
		prevCoarse = YMZ.getRegisterPsg0(reg + 1);
		prevFine = YMZ.getRegisterPsg0(reg);
		break;
	default:
		return;
	}
	uint16_t prev = ((uint16_t) prevCoarse & 0xff) << 8 + ((uint16_t) prevFine);
	uint16_t curr = ((((uint16_t) value) & 0x7f) << 7);

}

void setChannelFreqMsb(byte channel, byte reg, byte value) {
	// get current value
	uint8_t prevFine;
	uint8_t prevCoarse;
	switch (channel) {
	case CHANNEL_RAW_STEREO:
		prevCoarse = YMZ.getRegisterPsg(reg + 1);
		prevFine = YMZ.getRegisterPsg(reg);
		break;
	case CHANNEL_RAW_LEFT:
		prevCoarse = YMZ.getRegisterPsg1(reg + 1);
		prevFine = YMZ.getRegisterPsg1(reg);
		break;
	case CHANNEL_RAW_RIGHT:
		prevCoarse = YMZ.getRegisterPsg0(reg + 1);
		prevFine = YMZ.getRegisterPsg0(reg);
		break;
	default:
		return;
	}
	uint16_t prev = ((uint16_t) prevCoarse & 0xff) << 8 + ((uint16_t) prevFine);
	uint16_t curr = ((((uint16_t) value) & 0x7f) << 7);

	// mask off bits we aren't changing
	prev &= 0x001f;

	// add bits we are
	curr |= prev;

	// split to new MSB/LSB
	uint8_t fine = curr & 0xff;
	uint8_t coarse = (curr & 0x0f00) >> 8;

	// update
	switch (channel) {
	case CHANNEL_RAW_STEREO:
		YMZ.setRegisterPsg(reg, fine);
		YMZ.setRegisterPsg(reg + 1, coarse);
		break;
	case CHANNEL_RAW_LEFT:
		YMZ.setRegisterPsg1(reg, fine);
		YMZ.setRegisterPsg1(reg + 1, coarse);
		break;
	case CHANNEL_RAW_RIGHT:
		YMZ.setRegisterPsg0(reg, fine);
		YMZ.setRegisterPsg0(reg + 1, coarse);
		break;
	default:
		return;
	}
}

void setChannelFreqLsb(byte channel, byte reg, byte value) {
	// get current value
	uint8_t prevFine;
	uint8_t prevCoarse;
	switch (channel) {
	case CHANNEL_RAW_STEREO:
		prevCoarse = YMZ.getRegisterPsg(reg + 1);
		prevFine = YMZ.getRegisterPsg(reg);
		break;
	case CHANNEL_RAW_LEFT:
		prevCoarse = YMZ.getRegisterPsg1(reg + 1);
		prevFine = YMZ.getRegisterPsg1(reg);
		break;
	case CHANNEL_RAW_RIGHT:
		prevCoarse = YMZ.getRegisterPsg0(reg + 1);
		prevFine = YMZ.getRegisterPsg0(reg);
		break;
	default:
		return;
	}
	uint16_t prev = ((uint16_t) prevCoarse & 0xff) << 8 + ((uint16_t) prevFine);
	uint16_t curr = ((((uint16_t) value) & 0x7f) << 7);

	// mask off bits we aren't changing
	prev &= 0x0f80;

	// add bits we are
	curr |= prev;

	// split to new MSB/LSB
	uint8_t fine = curr & 0xff;
	uint8_t coarse = (curr & 0x0f00) >> 8;

	// update
	switch (channel) {
	case CHANNEL_RAW_STEREO:
		YMZ.setRegisterPsg(reg, fine);
		YMZ.setRegisterPsg(reg + 1, coarse);
		break;
	case CHANNEL_RAW_LEFT:
		YMZ.setRegisterPsg1(reg, fine);
		YMZ.setRegisterPsg1(reg + 1, coarse);
		break;
	case CHANNEL_RAW_RIGHT:
		YMZ.setRegisterPsg0(reg, fine);
		YMZ.setRegisterPsg0(reg + 1, coarse);
		break;
	default:
		return;
	}
}

void setRegister(byte channel, byte reg, byte value) {
	switch (channel) {
	case CHANNEL_RAW_STEREO:
		YMZ.setRegisterPsg(reg, value);
		break;
	case CHANNEL_RAW_LEFT:
		YMZ.setRegisterPsg1(reg, value);
		break;
	case CHANNEL_RAW_RIGHT:
		YMZ.setRegisterPsg0(reg, value);
		break;
	default:
		return;
	}
}

void handleControlChange(byte channel, byte number, byte value) {
	if (isRawMode(channel)) {
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
		case CC_NOISE_FREQ:
			setRegister(channel, 0x06, value & 0x1f);
			break;
		case CC_MIXER:
			setRegister(channel, 0x07, value & 0x3f);
			break;
		case CC_CHANNEL_A_LEVEL:
			setRegister(channel, 0x08, value & 0x1f);
			break;
		case CC_CHANNEL_B_LEVEL:
			setRegister(channel, 0x09, value & 0x1f);
			break;
		case CC_CHANNEL_C_LEVEL:
			setRegister(channel, 0x0a, value & 0x1f);
			break;
		case CC_ENVELOPE_FREQ_MSB:
			// TODO
			break;
		case CC_ENVELOPE_FREQ_LSB:
			// TODO
			break;
		case CC_ENVELOPE_SHAPE:
			setRegister(channel, 0x0d, value & 0x0f);
			break;
		case CC_ENVELOPE_VALUE_MSB:
			// TODO
			break;
		case CC_ENVELOPE_VALUE_LSB:
			// TODO
			break;
		case CC_CONTROL_POWER:
			setRegister(channel, 0x0f, value & 0x0f);
			break;
		}
	}
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
	MIDI.setHandleControlChange(handleControlChange);

	MIDI.begin(MIDI_CHANNEL_OMNI); // TODO make input channel configurable
	MIDI.turnThruOff();

	YMZ.setVolume(10);

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

