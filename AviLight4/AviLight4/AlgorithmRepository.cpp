#include "AlgorithmRepository.h"

#include "Receiver.h"

#include "Comm.h"
#include "adc.h"

#ifdef DEBUG_ALGO
#include "PrintStream.h"
extern PrintStream out;
#endif

/*static LedOff ch0LedOff;
static Blink ch0Blink;
static BlinkLong ch0BlinkLong;
static Rising ch0Rising;
static RotatingBeacon ch0RotatingBeacon;
static LedOn ch0LedOn;*/

extern void setOutput( BYTE channel, BYTE value );

WORD AlgorithmRepository::availableOuputs;
WORD AlgorithmRepository::channelEnable;
BYTE AlgorithmRepository::currentIndex[OUTPUT_CHANNELS];
ChannelData AlgorithmRepository::channelData[OUTPUT_CHANNELS];

BYTE AlgorithmRepository::inputChannel[OUTPUT_CHANNELS];
PROGRAM AlgorithmRepository::programArray[OUTPUT_CHANNELS][2 + MAX_SEGMENTS];
PROGRAM *AlgorithmRepository::currentProgram[OUTPUT_CHANNELS];

/*BlinkAlgorithm *AlgorithmRepository::algorithms[ALGORITHMS_PWM] = 
{
	&ch0LedOff, &ch0Blink, &ch0BlinkLong, &ch0Rising, &ch0RotatingBeacon, &ch0LedOn
};*/

BYTE AlgorithmRepository::segmentCounter;
BYTE AlgorithmRepository::segmentWaitCount;

//static BYTE dc= 100;

#ifdef ENABLE_DEBUG_UART
static BYTE lastOutput[OUTPUT_CHANNELS];
#endif

void AlgorithmRepository::init()
{
#if HARDWARE_VERSION < 4
  HC_DDR |= _BV(HC_BIT0)|_BV(HC_BIT1);
#endif

	availableOuputs= ALL_CHANNELS;
#ifdef DEBUGGING
	availableOuputs= 0x40;
#endif 

	for( BYTE i=0;i < OUTPUT_CHANNELS;i++ )
	{
		currentIndex[i] = 0;
		inputChannel[i] = 0;
#ifdef ENABLE_DEBUG_UART
    lastOutput[i]= 0xfe;
#endif
	}

	readFromEEPROM();

	channelEnable= ALL_CHANNELS;

	initAlgorithms();
}


void AlgorithmRepository::initAlgorithms()
{
	BYTE i;

	for(i = 0;i < OUTPUT_CHANNELS;i++ )
	{
    setCurrentIndex( i, getChannelSegment(i) );
    
		initAlgorithm( i );
	}

	allOutputsOff();
}

PROGRAM *AlgorithmRepository::getCurrentProgram( BYTE outputChannel )
{
	if( outputChannel >= OUTPUT_CHANNELS )
	{
		outputChannel= 0;
	}

	return currentProgram[ outputChannel ]; 
}

PROGRAM *AlgorithmRepository::getProgram( BYTE outputChannel, BYTE segment )
{
	if( outputChannel < OUTPUT_CHANNELS && segment < (2 + MAX_SEGMENTS) )
	{
		return &programArray[outputChannel][segment];
	}
	else
	{
		return 0;
	}
}

void AlgorithmRepository::initAlgorithm( BYTE outputChannel )
{
	if( outputChannel < OUTPUT_CHANNELS )
	{
		ChannelData *cd= &channelData[outputChannel];
		
		cd->counter= currentProgram[outputChannel]->period - 1;
		cd->flags= _BV( CHF_INIT );
		cd->oldValue= 0;
		cd->flashCounter= 0;
	}
}

BYTE AlgorithmRepository::getChannelSegment( BYTE outputChannel )
{  
	BYTE inChannel= getInputChannel( outputChannel )-1;
	BYTE segment;

	if( inChannel == 0xff ||
	  Receiver::getChannelMode( inChannel ) == 2 ||
	  !Receiver::isInputValid( inChannel ) )
	{
  	segment= 5;
	}
	else if( Adc::isLimitReached())
  {
    segment= 6;
  }
  else
	{
  	BYTE value= Receiver::getValue( inChannel );
  	segment= findProgram( outputChannel, value );
	}
  
  return segment;
  
}
//static BYTE dummy;

void AlgorithmRepository::program( BYTE tickerFlag, BYTE setupMode )
{
/*
if( tickerFlag )
{
	dc--;
}
*/
	BYTE outValue = 0;	
	BYTE outputChannel;
	WORD mask= 1;

	for(outputChannel = 0;outputChannel < OUTPUT_CHANNELS;outputChannel++, mask <<= 1 )
	{
		if( ( mask & availableOuputs ) == 0 )
		{
			continue;
		}

		BYTE index = getChannelSegment(outputChannel);

		if( index != currentIndex[outputChannel] )
		{
			setCurrentIndex( outputChannel, index );

			initAlgorithm( outputChannel );

#ifdef DEBUG_ALGO
			out.printf_P( PSTR("C%bx A%bx\r\n"), outputChannel, getCurrentProgram( outputChannel )->algorithm ) ;
#endif

			if( setupMode )
			{
				segmentCounter= 2 * (1 + index);
				segmentWaitCount= 0;
			}
		}

		BYTE flag= tickerFlag ? _BV(AFL_TICKER) : 0;
		if( outputChannel >= PWM_CHANNELS )
		{
			flag |= _BV(AFL_SWITCH_ALGO);
		}
    
/*
if( dc == 0 && outputChannel == 6 )
{
flag |= 0x80;
	Comm::printf_P( PSTR("A:%bx F:%bx"), getCurrentProgram( outputChannel )->algorithm, flag );
}
*/
		outValue= algorithm( &channelData[ outputChannel ], getCurrentProgram( outputChannel ), flag );


		if( channelEnable & mask )
		{
			if( index != 5 )
			{
				if( setupMode && segmentCounter != 0 )
				{
					if( tickerFlag )
					{
						segmentWaitCount++;
						if( segmentWaitCount > 24 )
						{
							segmentWaitCount= 0;

							segmentCounter--;
						}
					}

					outValue= (segmentCounter & 1 ) ? 255 : 0;
				}
				else
				{
					segmentCounter= 0;
				}
			}
		}
		else
		{
			outValue = 0;
		}
/*
if( dc == 0 && outputChannel == 15 )
{
	Comm::printf_P( PSTR("AO: %bx O:%bx CE:%wx"), channelData[ outputChannel ].oldValue, outValue, channelEnable );
}
*/
		setOutput(outputChannel, outValue );
	}
/*
if( dc == 0 )
{
	dc= 200;
}*/
}


void AlgorithmRepository::allOutputsOff()
{
  for(BYTE i= 0; i < OUTPUT_CHANNELS;i++ )
  {
    setOutput( i, 0 );
  }  
}

#if HARDWARE_VERSION < 3
void AlgorithmRepository::setOutput( BYTE outputChannel, BYTE outValue )
{
#ifdef ENABLE_DEBUG_UART
  /*if( outValue != lastOutput[outputChannel] )
  {
    lastOutput[outputChannel]= outValue;
    Comm::printf_P(PSTR("O%bx = %bx"), outputChannel, outValue);
  }*/
#endif

  if( outputChannel < PWM_CHANNELS )
  {
    //PWM::setPwmChannel( outputChannel, outValue );
  }
  else if( outputChannel < (PWM_CHANNELS + SWITCH_CHANNELS))
  {
    setOutput( outputChannel - PWM_CHANNELS, outValue );
  }
  /*else
  {
    BYTE bit;
    if( outputChannel == (PWM_CHANNELS + SWITCH_CHANNELS))
    {
      bit= _BV( HC_BIT0 );
    }
    else
    {
      bit= _BV( HC_BIT1 );
    }
  
    if( outValue )
    {
      HC_PORT |= bit;
    }
    else
    {
      HC_PORT &= ~bit;
    }
  }  */
}
#endif

void AlgorithmRepository::setCurrentIndex( BYTE outputChannel, BYTE index )
{
	if( outputChannel >= OUTPUT_CHANNELS )
	{
		outputChannel= 0;
	}

  if( currentIndex[ outputChannel ] != index)
  {
#ifdef DEBUG_ALGO
    out.printf_P( PSTR("CIDX: %bx I:%bx OI:%bx A:%bx"),outputChannel, index, currentIndex[ outputChannel ], currentProgram[ outputChannel ]->algorithm);
#endif

	  currentIndex[ outputChannel ] = index;
	  currentProgram[ outputChannel ] = &programArray[outputChannel][index];
  }
}

void AlgorithmRepository::incrementCurrentAlgo( BYTE outputChannel )
{
	PROGRAM *program = getCurrentProgram( outputChannel );
	BYTE max;

	if( outputChannel < PWM_CHANNELS )
	{
		max= ALGORITHMS_PWM;
	}
	else
	{
		max= ALGORITHMS_SWITCH;
	}

	program->algorithm++;
	if( program->algorithm >= max )
	{
		program->algorithm= 0;
	}

	program->flashcount= 0;

	initAlgorithms();
}

BYTE AlgorithmRepository::getInputChannel( BYTE outputChannel )
{
	if( outputChannel >= OUTPUT_CHANNELS )
	{
		outputChannel= 0;
	}

	return inputChannel[ outputChannel ];
}

void AlgorithmRepository::setInputChannel( BYTE outputChannel, BYTE inChannel )
{
	if( outputChannel < OUTPUT_CHANNELS )
	{
		inputChannel[ outputChannel ]= inChannel;
	}
}

BYTE AlgorithmRepository::findProgram( BYTE outputChannel, BYTE value )
{
	if( outputChannel >= OUTPUT_CHANNELS )
	{
		outputChannel= 0;
	}
	BYTE inChannel= inputChannel[ outputChannel ];
	if( inChannel >= Receiver::numberOfInputChannels())
	{
		inChannel= 0;
	}

	BYTE index= value2program( value );
	
	if( index == currentIndex[ outputChannel ] )
	{
		return index;
	}
	else if( index > currentIndex[ outputChannel ])
	{
		BYTE level= program2value( 1 + currentIndex[ outputChannel ] );

		if( value - PROGRAM_HYSTERESIS > level )
		{
			return index;
		}
		else
		{
			return currentIndex[ outputChannel ];
		}
	}
	else
	{
		BYTE level= program2value( currentIndex[ outputChannel ] );

		if( value + PROGRAM_HYSTERESIS < level )
		{
			return index;
		}
		else
		{
			return currentIndex[ outputChannel ];
		}
	}
}

void AlgorithmRepository::learnPeriod( BYTE outputChannel )
{
	if( outputChannel < OUTPUT_CHANNELS )
	{
		ChannelData *cd= &channelData[ outputChannel ];
	  PROGRAM *p= getCurrentProgram( outputChannel );
    
		if( cd->flags & _BV(CHF_LEARN_NOW) )
		{
			p->period= -cd->counter;
			cd->flags &= ~_BV(CHF_LEARN_NOW);

#ifdef ENABLE_DEBUG_UART
		Comm::printf_P( PSTR("C %bx P %wx"), outputChannel, p->period );
#endif

		}
		else
		{
			initAlgorithm( outputChannel );
		
      p->period= 3600;
      if( (p->flashcount & 0xfc) == 0 )
      {
        p->flashcount= 0x20;
      }      

			cd->flags |= _BV(CHF_LEARN_NOW);
			cd->counter = 0;
		}
	}
}

void AlgorithmRepository::flashcount( BYTE outputChannel )
{
#if HARDWARE_VERSION < 4
	if( outputChannel >= PWM::getOutputChannels() )
	{
		outputChannel= 0;
	}
	PROGRAM * program= getCurrentProgram( outputChannel );

	program->flashcount= (1 + program->flashcount) & 3;
#endif
/*
#ifdef ENABLE_DEBUG_UART
		Uart::putch( 'C' );
		Uart::printHex( outputChannel );
		Uart::putch( ':' );
		Uart::printHex( program->flashcount );
		Uart::print_P( PSTR("\r\n"));
#endif
*/
}
