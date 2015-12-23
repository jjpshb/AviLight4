#ifndef INC_BLINK_ALGORITHM
#define INC_BLINK_ALGORITHM

#include <avr/io.h>

#include "integer.h"
#include "device.h"

struct PROGRAM
{
	// the individual algorithm
	BYTE algorithm;
	// the algorithm period
	WORD period;
	// algorithm dependend:
	// FLASH: Bit 0/1: flashcount 1..4, Bit 2..8: Flash duration
	// PWM ON: the brightness
	BYTE flashcount;
};

struct ChannelData
{
	// counted up every 8,192ms, may be resetted by individual algorithm
	WORD counter;
	// the Stick value 0 .. 255
	BYTE stickValue;
	// the old (and new) brightness value
	BYTE oldValue;
	// individual algorithm flags
	BYTE flags;
	// the number of flashes
	BYTE flashCounter;

	BYTE dummy[2];
};

// Channel Flags:
// The counter reached the period
#define CHF_COUNTER_REACHED		7	// the counter reached 0 (decrementing)
#define CHF_TICKER						6	// set every (8,921ms), the counter has been decremented
#define CHF_INIT							5
#define CHF_OUTPUT_ON					4
#define CHF_OUTPUT_OFF_FLASH	3	// The output is short off
#define CHF_REVERSE						2	// Rising Algo
#define CHF_LEARN_NOW					1	// learn Period


// The Blink/Flash algorithm
// flag bits see AFL_*
extern BYTE algorithm(ChannelData *cd, PROGRAM *program, BYTE flag );

#define AFL_TICKER						0	// the ticker flag is set every 8,192ms
#define AFL_SWITCH_ALGO				1 // set, if swich outputs, PWM else

// the Switch Output Channel algorithm
typedef void (* PF_BLINK_ALGORITHM)(ChannelData *cd, PROGRAM *program );

#endif
