#ifndef INC_ALGORITHM_REPOSITORY
#define INC_ALGORITHM_REPOSITORY

#include "BlinkAlgorithm.h"
#include "device.h"

extern "C" BYTE value2program( BYTE x );
extern "C" BYTE program2value( BYTE x );

class AlgorithmRepository
{
public:

	static void init();


	static void initAlgorithms();

	static BYTE findProgram( BYTE outputChannel, BYTE value );

	static void program( BYTE tickerFlag, BYTE setupMode );

	static PROGRAM *getCurrentProgram( BYTE outputChannel );
	static PROGRAM *getProgram( BYTE outputChannel, BYTE segment );

	static void incrementCurrentAlgo( BYTE outputChannel );

	static void learnPeriod( BYTE outputChannel );
	static void flashcount( BYTE channel );

	static void factoryDefault();
	static void readFromEEPROM();
	static void writeToEEPROM();

	static BYTE getInputChannel( BYTE outputChannel );
	static void setInputChannel( BYTE outputChannel, BYTE inputChannel );

  static void allOutputsOff();
#if HARDWARE_VERSION < 3
  static void setOutput( BYTE outputChannel, BYTE value );
#endif
  
  static BYTE getChannelSegment( BYTE outputChannel );
	static void initAlgorithm( BYTE outputChannel );

private:

	static void setCurrentIndex( BYTE outputChannel, BYTE index );

	//static BlinkAlgorithm *algorithms[ALGORITHMS_PWM];

	// volatile Data (not persisted in EEPROM)
public:  
	static ChannelData channelData[OUTPUT_CHANNELS];
	static PROGRAM programArray[OUTPUT_CHANNELS][2 + MAX_SEGMENTS];
private:
	static BYTE currentIndex[OUTPUT_CHANNELS];

	// stored in EEPROM
	static BYTE inputChannel[OUTPUT_CHANNELS];

	static PROGRAM *currentProgram[OUTPUT_CHANNELS];

public:
	// Bitvector of enabled outputs
	static WORD channelEnable;
	static BYTE segmentCounter;
	static BYTE segmentWaitCount;

	// Bitvector of enabled outputs by hardware
	static WORD availableOuputs;
};

#endif
