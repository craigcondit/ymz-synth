#ifndef _ymz_synth_h_
#define _ymz_synth_h_
#include "Arduino.h"

#include "MIDI.h"
#include "MIDI.hpp"
#include "hcYmzShield.h"

typedef void (*regSet)(byte, byte);
typedef byte (*regGet)(byte);

#ifdef __cplusplus
extern "C" {
#endif
void loop();
void setup();
#ifdef __cplusplus
} // extern "C"
#endif

#endif /* _ymz_synth_h_ */
