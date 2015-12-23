#include "Receiver.h"
#include "Device.h"

#ifdef DEBUG_RECEIVER
#include "PrintStream.h"
extern PrintStream out;
#endif



extern "C" WORD div16by8( WORD d, BYTE dd );
extern "C" WORD mul8by8( WORD d, BYTE dd );

//BYTE Receiver::lastCounter;
BYTE Receiver::channelReverse;
volatile BYTE Receiver::lastPortValue asm("Receiver_lastPortValue");
volatile WORD Receiver::counter asm("Receiver_counter" );

volatile CHANNEL Receiver::ch[RECEIVER_CHANNELS] asm("Receiver_ch");

BYTE Receiver::learnStickMinMaxMode;
WORD Receiver::lastCounter;

void Receiver::init()
{
	TCCR1A= 0;
	TCCR1B= 1;
	TCCR1C= 0;
	TIMSK1= _BV( TOIE1 );

	counter= 0;
	learnStickMinMaxMode= 0;
	channelReverse= 0;
  lastCounter= 0;
  
BYTE i;
	for( i=0;i < RECEIVER_CHANNELS; i++ )
	{
		ch[i].flags = 0;
		ch[i].minValue = FACTORY_SET_MINIMUM;
		ch[i].maxValue = FACTORY_SET_MAXIMUM;
		ch[i].divisor = FACTORY_SET_DIVISOR;
		ch[i].failedCounter= 0;
		ch[i].lastValue= 0;
		ch[i].skipCounter= 0;
	}

#if HARDWARE_VERSION == 4
  REC0_DDR &= ~_BV(REC0);
  REC1_DDR &= ~_BV(REC1);
	ch[0].inbit= _BV( REC0 );
	ch[1].inbit= _BV( REC1 );
#elif RECEIVER_CHANNELS == 1
	REC_DDR &= ~( _BV( REC0 ) );
	REC_PCMSK = _BV( REC0 );
	ch[0].inbit= _BV( REC0 );
#elif RECEIVER_CHANNELS == 2
	REC_DDR &= ~( _BV( REC0 ) | _BV( REC1 ) );
	REC_PCMSK = _BV( REC0 ) | _BV( REC1 );
	ch[0].inbit= _BV( REC0 );
	ch[1].inbit= _BV( REC1 );
#elif RECEIVER_CHANNELS == 3
	REC_DDR &= ~( _BV( REC0 ) | _BV( REC1 ) | _BV( REC2 ));
	REC_PCMSK = _BV( REC0 ) | _BV( REC1 ) | _BV( REC2 );
	ch[0].inbit= _BV( REC0 );
	ch[1].inbit= _BV( REC1 );
	ch[2].inbit= _BV( REC2 );
#elif RECEIVER_CHANNELS == 4
	REC_DDR &= ~( _BV( REC0 ) | _BV( REC1 ) | _BV( REC2 ) | _BV( REC3 ));
	REC_PCMSK = _BV( REC0 ) | _BV( REC1 ) | _BV( REC2 ) | _BV( REC3 );
	ch[0].inbit= _BV( REC0 );
	ch[1].inbit= _BV( REC1 );
	ch[2].inbit= _BV( REC2 );
	ch[3].inbit= _BV( REC3 );
#endif

#if HARDWARE_VERSION == 4
  EICRB = _BV(ISC60);
  EIFR |= _BV(INTF6);
  EIMSK = _BV(INT6);
  
  PCMSK0 = _BV(PCINT4);
  PCIFR |= _BV(PCIF0);
  PCICR = _BV(PCIE0);  
#else
	PCICR |= _BV( REC_PCICR );
#endif
}

BYTE Receiver::isAnyInputValid( )
{
#ifdef DEBUGGING
	return 1;
#else
  for( BYTE i=0;i < RECEIVER_CHANNELS ;i++)
	{
  	if( ch[i].flags & _BV(CHF_VALID) )
    {
      return 1;
    }     
	}
  
	return 0;
#endif
}

BYTE Receiver::isInputValid( BYTE channel )
{
#ifdef DEBUGGING
	return 1;
#else
	if( channel < RECEIVER_CHANNELS )
	{
		return ch[channel].flags & _BV(CHF_VALID);
	}
	else
	{
		return 0;
	}
#endif
}

BYTE Receiver::getValue( BYTE channel )
{
	if( isInputValid( channel ) )
	{
		if( channel < RECEIVER_CHANNELS )
		{
			return ch[channel].bValue;
		}
	}

	return 0;
}

#ifdef ENABLE_DEBUG_UART
//BYTE count2= 0;
#endif 

void Receiver::cyclic( )
{
	checkInput();
	if( (lastCounter & 0xff00) != (counter & 0xff00) )
	{
    lastCounter= counter;
		checkChannelValid();

#ifdef ENABLE_DEBUG_UART
/*  if( count2++ > 80 )
	{
		if( ch[ 0 ].flags & _BV( CHF_VALID ))
		{
			Uart::printHex( ch[ 0 ].bValue );
		}
		else
		{
			Uart::putch( '*' );
			Uart::putch( '*' );
		}
		Uart::putch( ' ' );
		if( ch[ 1 ].flags & _BV( CHF_VALID ))
		{
			Uart::printHex( ch[ 1 ].bValue );
		}
		else
		{
			Uart::putch( '*' );
			Uart::putch( '*' );
		}

		Uart::crlf();
		count2= 0;
	}*/
#endif
	}
}

void Receiver::checkInput()
{
BYTE i;
volatile CHANNEL *pch;
BYTE mask= 1;

#ifdef DEBUG_RECEIVER
	pch= &ch[0];
  out.printf_P(PSTR("0 LC:%wx V:%wx F:%bx FC:%bx\r\n"), ch->lastChange, ch->value, ch->flags, ch->failedCounter);
  pch++;
  out.printf_P(PSTR("1 LC:%wx V:%wx F:%bx FC:%bx\r\n"), ch->lastChange, ch->value, ch->flags, ch->failedCounter);

#endif
	for( i=0;i < RECEIVER_CHANNELS; i++, mask <<= 1 )
	{
		pch= &ch[i];

		if( pch->flags & _BV( CHF_NEW_PULSE ) )
		{
			pch->flags &= ~_BV( CHF_NEW_PULSE );
			WORD v= pch->value;

			if( v > MIN_PULSE_LENGTH && v < MAX_PULSE_LENGTH )
			{
				pch->failedCounter= 0;
				pch->flags |= _BV(CHF_VALID);

				if( (SHORT)v < ((SHORT)pch->lastValue - MAX_DERIVATION) 
					|| (SHORT)v > ((SHORT)pch->lastValue + MAX_DERIVATION) )
				{
					pch->skipCounter++;
					
					if( pch->skipCounter < MAX_SKIP_COUNTER )
					{
						continue;
					}
				}

				pch->skipCounter= 0;
				pch->lastValue= v;
				if( learnStickMinMaxMode & mask )
				{
					// learn min and max Values
					if( v < pch->minValue )
					{
						pch->minValue= v;
					}
					if( v > pch->maxValue )
					{
						pch->maxValue= v;
					}

					pch->divisor = div16by8(pch->maxValue - pch->minValue, 255);

					if( pch->divisor < 16 )
					{
						pch->divisor= 16;
					}
					if( pch->divisor > 0x40 )
					{
						pch->divisor= 0x40;
					}					

					pch->maxValue= pch->minValue + mul8by8( pch->divisor, 255 );
				}
				else
				{
					if( v < pch->minValue )
					{
						v= pch->minValue;
					}
					if( v > pch->maxValue )
					{
						v= pch->maxValue;
					}
				}

				v -= pch->minValue;
				pch->bValue= div16by8(v, pch->divisor);
				if( channelReverse & (1 << (i << 1)) )
				{
					pch->bValue= ~pch->bValue;
				}
			}
		}
	}	
}

void Receiver::checkChannelValid()
{
BYTE i;
volatile CHANNEL *pch;

	for( i=0;i < RECEIVER_CHANNELS; i++ )
	{
		pch= &ch[i];

		if( pch->failedCounter < RECEIVER_FAILED )
		{
			pch->failedCounter++;
		}
		else
		{
			pch->flags &= ~_BV( CHF_VALID );
		}
	}
}

void Receiver::setChannelMode( BYTE inChannel, BYTE mode  )
{
	if( !isInputValid( inChannel ))
	{
		mode= 2;
	}

	BYTE mask;

	mode &= 3;
	inChannel &= 3;
	inChannel <<= 1;
	mode = (BYTE)(((BYTE)mode) << (BYTE)inChannel);
	mask = (BYTE)(((BYTE)3) << (BYTE)inChannel);

	channelReverse &= ~mask;
	channelReverse |= mode;
}

BYTE Receiver::getChannelMode( BYTE inChannel )
{
	inChannel &= 3;
	inChannel <<= 1;

	return (((BYTE)channelReverse) >> inChannel) & 3;
}

void Receiver::learnStickMinMax( BYTE bitMask )
{
	learnStickMinMaxMode= bitMask;
	for( BYTE i=0,mask=1;i < RECEIVER_CHANNELS; i++, mask <<= 1 )
	{
		if( learnStickMinMaxMode & mask )
		{
			ch[i].minValue = FACTORY_SET_MINIMUM + 0x100;
			ch[i].maxValue = FACTORY_SET_MAXIMUM - 0x100;
			ch[i].divisor = FACTORY_SET_DIVISOR;
		}
	}


#ifdef ENABLE_DEBUG_UART
	Comm::printf_P( PSTR("learn: %bx"), learnStickMinMaxMode );
#endif
}

BYTE Receiver::getLearnStickMode()
{
  return learnStickMinMaxMode;
}

WORD Receiver::getChannelMin( BYTE channel )
{
	return ch[ channel & 1 ].minValue;
}

void Receiver::setChannelMin( BYTE channel, WORD value )
{
	if( channel < RECEIVER_CHANNELS )
	{
		ch[ channel ].minValue= value;
	}
}

WORD Receiver::getChannelMax( BYTE channel )
{
	return ch[ channel & 1 ].maxValue;
}

void Receiver::setChannelMax( BYTE channel, WORD value )
{
	if( channel < RECEIVER_CHANNELS )
	{
		ch[ channel & 1 ].maxValue= value;
	}
}

CHANNEL *Receiver::getChannel( BYTE index )
{
  if( index < RECEIVER_CHANNELS )
  {
    return (CHANNEL *)&ch[index];
  }
  
  return (CHANNEL *)&ch[0];
}

volatile WORD *Receiver::getTickerAddr()
{ 
  return &counter; 
}

WORD Receiver::getTicker()
{ 
  return counter; 
}

