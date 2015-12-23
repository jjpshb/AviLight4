#ifndef COMM_INCLUDED
#define COMM_INCLUDED

#include "device.h"
#include "integer.h"
#include <stdarg.h>
#include <avr/pgmspace.h>
#include "ByteReceiver.h"

#define COMM_MODE_DISCONNECTED			0
#define COMM_RECEIVING							1
#define COMM_ESCAPE									2

#define BUFFER_SIZE									(OUTPUT_CHANNELS+1)

#define	STX													2
#define ETX													3
#define ESCAPE											27

// Ping the device, the answer is a ping, too
#define CMD_PING												0
// Ping Answer
// sendBuffer[0]: 00: Firmware mode, 01: Bootloader mode, 02: Bootloader mode, no firmware installed.

// Answers the firmware version as a string
#define CMD_INFO												1

// Print string to terminal
#define CMD_TERMINAL										2

// Send Channel Info
// sendBuffer[0]: No of PWM outputs
// sendBuffer[1]: No of switch outputs
// sendBuffer[2]: No of receiver channels
// sendBuffer[3]: No of segments
#define CMD_CHANNEL_INFO								3

// Get Receiver Channel values:
// sendBuffer[0]: No of receiver channels
// sendBuffer[1]: Segment Ch #0, 0xff, if no signal
// sendBuffer[2]: Input value Ch #0 (0.255)
// repeat 1,2 for all channels
#define CMD_RECEIVER										4

// Get the receiver channel modes
// sendBuffer[0]: number of input channels
// sendBuffer[1]: Mode of Input Channel 0
// repeat for the number of channels
// Mode 0= normal
// Mode 1= reverse
// Mode 2= no input signal
#define CMD_GET_RECEIVER_CHANNEL_MODE		5

// Get the receiver channel modes
// buffer[0]: Input Channel No
// buffer[1]: Mode of Input Channel, if mode is 2, it can not be changed
// Answer like CMD_GET_RECEIVER_CHANNEL_MODE
#define CMD_SET_RECEIVER_CHANNEL_MODE		6

// get the controlling input channel for the output channel
// sendBuffer[0]: the number of output channels
// sendBuffer[1]: the controlling input channel for output channel #0
// repeat for all output channels
#define CMD_GET_CONTROLLING_CHANNEL			7

// set the controlling input channel for the output channel
// buffer[0]: output Channel no
// buffer[1]: receiver input channel
// answer like CMD_GET_CONTROLLING_CHANNEL
#define CMD_SET_CONTROLLING_CHANNEL			8

// Get programm info
// buffer[0]: output Channel, first PWM Channels, then Switch Channels, max OUTPUT_CHANNELS-1
// buffer[1]: Segment No
// Answer:
// sendBuffer[0]: output Channel
// sendBuffer[1]: Segment No
// sendBuffer[2]: algorithm No
// sendBuffer[3]: Period Low
// sendBuffer[4]: Period High
// sendBuffer[5]: Flash Byte
#define CMD_GET_PROGRAMM								9

// Set Program info
// buffer[0]: output Channel, first PWM Channels, then Switch Channels, max OUTPUT_CHANNELS-1
// buffer[1]: Segment No
// buffer[2]: algorithm No
// buffer[3]: Period Low
// buffer[4]: Period High
// buffer[5]: Flash Byte
// Answer:
// sendBuffer[0]: output Channel
// sendBuffer[1]: Segment No
#define CMD_SET_PROGRAMM								10

// read adc value
// answer:
// sendBuffer[0]: adc low
// sendBuffer[1]: adc high
// sendBuffer[2]: limit low
// sendBuffer[3]: limit high
#define CMD_GET_VOLTAGE                 11

// Enter bootloader
#define CMD_ENTER_BOOTLOADER            12

// Config changed on local device (AviLight - PC)
#define CMD_CONFIG_CHANGED              13

#define CMD_WRITE_TO_EEPROM             14
#define CMD_READ_FROM_EEPROM            15
#define CMD_TEST_CHANNEL                16

#define CMD_GET_LEARN_STICKMODE         17
#define CMD_SET_LEARN_STICKMODE         18

#define CMD_SET_BATTERY_LIMIT           19
// buffer[0]: voltage limit low
// buffer[1]: voltage limit high
// Answer like Voltage


#define CMD_DUMP_PROGRAM_ARRAY          0x7b
#define CMD_DUMP_EEPROM                 0x7c
#define CMD_DUMP_RECEIVER               0x7d
#define CMD_DUMP_GLOBALS                0x7e
#define CMD_DUMP_CHANNELS               0x7f


// Answer: Command unknown
#define CMD_UNKNONWN										0xff

// Communication Module
// Received Telegram: STX BYTE0 ... BYTEn ETX
// BYTE0 is the Command

class Comm : public ByteReceiver
{
public:
	
  Comm( ByteReceiver *byteReceiver );

	void cyclic( BYTE ticker );
	virtual void received( BYTE data );

	void writeAnswer( BYTE command );
	void writeAnswer( BYTE command, const unsigned char *answer, BYTE size );
	void writePSTR( BYTE command, const char *answer );

  // connected to the Host
  BYTE isHostConnected();
  
private:
	void sendSequence( BYTE c );

public:

#ifdef ENABLE_DEBUG_UART
	// %bx: Byte, %bd= Byte (dez)
	// %wx: Word, %wd= Word (dez)
	// %lx: Long
	void printf(const char *fmt, ...);
	void printf_P(const char *fmt, ...);
	void v_printf(const char *fmt, bool rom, va_list p);
#else
	void printf(const char *fmt, ...)										{ }
	void printf_P(const char *fmt, ...)									{ }
	void v_printf(const char *fmt, bool rom, va_list p)	{ }
#endif

private:

  ByteReceiver *byteReceiver;

  // COMM_* constants
	BYTE mode;
	BYTE buffer[ BUFFER_SIZE ];
	BYTE sendBuffer[ BUFFER_SIZE ];
	BYTE recptr;

  // counts ticks, if no data received
  BYTE offlineCounter;
  
	void processCommand();
	void processReceiver();
	void getReceiverChannelMode();
	void setReceiverChannelMode();
	void getControllingInputChannels();
	void setControllingInputChannels();
	void getProgram();
	void setProgram();
  void getVoltage();
  void dumpProgramArray();
  void dumpEeprom();
  void dumpReceiver();
	void dumpChannels();
  void dumpChannel( BYTE ch );
  void dumpGlobals();

  void setVoltageLimit();

#ifdef ENABLE_DEBUG_UART
	void putch( BYTE c );
	void print_nibble(BYTE c);

	void print_hex(BYTE c);
	void print_hex(WORD w);
	void print_hex(DWORD w);

	BYTE print_dez(BYTE b);
	BYTE print_dez(WORD w);
#else
	void putch( BYTE c )					{ }
	void print_nibble(BYTE c)		{ }

	void print_hex(BYTE c)				{ }
	void print_hex(WORD w)				{ }
	void print_hex(DWORD w)			{ }

	BYTE print_dez(BYTE b)				{ return 0; }
	BYTE print_dez(WORD w)				{ return 0; }
#endif
};

#endif
