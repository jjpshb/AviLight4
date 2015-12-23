#include "BlinkAlgorithm.h"

#include "Comm.h"

void switchOff( ChannelData *cd, PROGRAM *program );
void switchOn( ChannelData *cd, PROGRAM *program );
void switchFlash( ChannelData *cd, PROGRAM *program );
void switchPulse( ChannelData *cd, PROGRAM *program );

void pwmOn( ChannelData *cd, PROGRAM *program );

static const PF_BLINK_ALGORITHM algos[ALGORITHMS_SWITCH] = { &switchOff, &switchFlash, &switchPulse, &switchOn } ;

static const PF_BLINK_ALGORITHM algosPwm[ALGORITHMS_PWM] = { &switchOff, &pwmOn } ;

BYTE algorithm(ChannelData *cd, PROGRAM *program, BYTE flag )
{
/*
if( flag & 0x80 )
{
	Comm::printf_P( PSTR("A:%bx SO:%bx ADDR:%wx"), program->algorithm, 
	  (sizeof( algos )/sizeof( algos[0] )), algos[ program->algorithm ] );
}*/

	BYTE cdFlags= cd->flags & ~( _BV( CHF_TICKER ) | _BV( CHF_COUNTER_REACHED ));
	if( flag & _BV(AFL_TICKER) )
	{
		cd->counter--;
		if( cd->counter == 0 )
		{
			cd->counter= program->period;
			cdFlags |= _BV( CHF_COUNTER_REACHED );
		}
		cdFlags |= _BV( CHF_TICKER );
	}

	cd->flags= cdFlags;

	if( flag & _BV(AFL_SWITCH_ALGO) )
	{
		if( program->algorithm < (sizeof( algos )/sizeof( algos[0] )))
		{
			algos[ program->algorithm ](cd, program );
		}
	}
	else
	{
		if( program->algorithm < (sizeof( algosPwm )/sizeof( algosPwm[0] )))
		{
			algosPwm[ program->algorithm ](cd, program );
		}
	}

	cd->flags &= ~_BV( CHF_INIT );

	return cd->oldValue;
}

void switchOff( ChannelData *cd, PROGRAM *program )
{
	cd->oldValue= 0;
}

void switchOn( ChannelData *cd, PROGRAM *program )
{
	cd->oldValue= 0xff;
}

void pwmOn( ChannelData *cd, PROGRAM *program )
{
	cd->oldValue= program->flashcount;
}

void switchFlash( ChannelData *cd, PROGRAM *program )
{
	if( cd->flags & _BV( CHF_COUNTER_REACHED ) )
	{
		if( cd->flags & _BV( CHF_OUTPUT_ON ) )
		{
			cd->flags &= ~_BV( CHF_OUTPUT_ON );

			if( cd->flashCounter != 0 )
			{
				cd->flashCounter--;
				cd->flags |= _BV( CHF_OUTPUT_OFF_FLASH );
				cd->counter= (program->flashcount >> 2) + 1;
			}
			else
			{
				cd->flags &= ~_BV( CHF_OUTPUT_OFF_FLASH );
			}
			cd->oldValue= 0;
		}
		else 
		{
			if( !( cd->flags & _BV( CHF_OUTPUT_OFF_FLASH ) ) )
			{
				cd->flashCounter= program->flashcount & 3;
			}

			cd->counter= (program->flashcount >> 2) + 1;
			cd->flags |= _BV( CHF_OUTPUT_ON );
			cd->oldValue= 0xff;
		}
	}
}

void switchPulse( ChannelData *cd, PROGRAM *program )
{
	if( cd->flags & _BV( CHF_INIT ) )
	{
		cd->oldValue= 0xff;
	}
	else if( cd->flags & _BV( CHF_COUNTER_REACHED ) )
	{
		cd->oldValue= 0;
	}
}
