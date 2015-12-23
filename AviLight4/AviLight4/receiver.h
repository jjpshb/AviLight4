#ifndef RECEIVER_DEFINED
#define RECEIVER_DEFINED

#include "integer.h"
#include "device.h"

// Size of the Channel structure for assembler
#define CHANNEL_SIZE				16

// Offsets in struct for assembler
#define OFF_INBIT			0
#define OFF_FLAGS			1
#define OFF_B0				2
#define OFF_B1				3
#define OFF_VALUE0		4
#define OFF_VALUE1		5

// Channel flags
#define CHF_NEW_PULSE				0
#define CHF_VALID						1

#define RECEIVER_FAILED			3
#define MIN_PULSE_LENGTH		0x0100 //1800
#define MAX_PULSE_LENGTH		0xf000 //0x5000

#ifndef FACTORY_SET_MINIMUM
#define FACTORY_SET_MINIMUM	0x2340
#define FACTORY_SET_MAXIMUM 0x3c80
#define FACTORY_SET_DIVISOR	0x19
#endif

#ifndef __ASSEMBLER__

// if the stick value is MAX_DERIVATION greater or less than the last value, skip it
#define MAX_DERIVATION			0x100
// skip it for max. MAX_SKIP_COUNTE, then accept it
#define MAX_SKIP_COUNTER		8

typedef struct
{
	BYTE inbit;         // the in port bit
	BYTE flags;         // see CHF_* consts
	WORD lastChange;    // timer value of last change
	WORD value;         // actual pulse value
	WORD minValue;      // min limit
	WORD maxValue;      // max limit
	WORD lastValue;     // last value
	BYTE skipCounter;
	BYTE divisor;
	BYTE bValue;
	BYTE failedCounter;
} CHANNEL;

class Receiver
{
public:

	static void init();

	static void cyclic( );

	static BYTE isAnyInputValid( );
	static BYTE isInputValid( BYTE channel );

	// turn the Stick learn mode on or off for Channel 0 (Bit 0) to Channel 3 (Bit 3)
	static void learnStickMinMax( BYTE bitMask );
  static BYTE getLearnStickMode();

	// Channel Mode, 0= Normal, 1= Reverse, 2= disabled
	static void setChannelMode( BYTE inChannel, BYTE mode );
	static BYTE getChannelMode( BYTE inChannel );

	static void setChannelModeByte( BYTE m )		{ channelReverse= m; }
	static BYTE getChannelModeByte()						{ return channelReverse; }

	static BYTE numberOfInputChannels()			{ return RECEIVER_CHANNELS; }
	//returns the Input Channel Value for Channel 0 ... 3. If the Channel is invalid return 0.
	// If the Channel 1 is invalid return the value of channel 0
	static BYTE getValue( BYTE channel );

	// get/set Min/Max for Storing to/Reading from EEPROM
	static WORD getChannelMin( BYTE channel );
	static void setChannelMin( BYTE channel, WORD value );
	static WORD getChannelMax( BYTE channel );
	static void setChannelMax( BYTE channel, WORD value );

  // only for dump
  static CHANNEL *getChannel( BYTE index );
  
  static volatile WORD *getTickerAddr();
  static WORD getTicker();
    
private:

	// returns every 8,192 ms a 1
	static BYTE getTickFlag();

	static void checkInput();

	// called every 8MHz / 65536 = 8,192 ms
	static void checkChannelValid();

	// Bit 0..1: Channel 1
	// Bit 2..3: Channel 2
	// Bit 4..5: Channel 3
	// Bit 6..7: Channel 4
	// Value: 0: Normal, 1: Channel Reverse, 2: Channel disabled
	static BYTE channelReverse;

	static volatile BYTE lastPortValue;
	static volatile WORD counter;

	static volatile CHANNEL ch[];

  static WORD lastCounter;
	static BYTE learnStickMinMaxMode;
};

#endif

#endif
