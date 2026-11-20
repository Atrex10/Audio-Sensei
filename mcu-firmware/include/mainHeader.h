#ifndef MAIN_HEADER_H
#define MAIN_HEADER_H

// #define DEBUG
#define AUTO_READ_STATE_AFTER_BUTTON
#define USE_SIG_LEDS

#include "state-buffer.h"

extern unsigned long idCounter;

extern const int lcdSda;
extern const int lcdScl;

extern const int ledPins[3];

extern StateBuffer stateBuffer;

#endif
