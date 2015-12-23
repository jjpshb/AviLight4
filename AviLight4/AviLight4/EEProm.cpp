#include <avr/eeprom.h>

#include "AlgorithmRepository.h"

#include "Receiver.h"
#include "Comm.h"
#include "adc.h"

#ifdef DEBUG_EEPROM
#include "PrintStream.h"
extern PrintStream out;
#endif

void AlgorithmRepository::readFromEEPROM()
{
#ifndef DEBUGGING

	WORD addr= 0;
	BYTE c1= eeprom_read_byte( (uint8_t *)addr++ );
	BYTE c2= eeprom_read_byte( (uint8_t *)addr++ );
	BYTE c3= eeprom_read_byte( (uint8_t *)addr++ );

#ifdef DEBUG_EEPROM
	out.printf_P( PSTR("EEPROM:%bx %bx %bx"), c1, c2, c3 );
#endif

	if( c1 == 'J' && c2 == 'P' && c3 == ((RECEIVER_CHANNELS << 5) | OUTPUT_CHANNELS))
	{
    BYTE i;
    
		eeprom_read_block( &programArray, (const void *)addr, (size_t)sizeof( programArray ) );
		addr += sizeof( programArray );
		eeprom_read_block( &inputChannel, (const void *)addr, (size_t)sizeof( inputChannel ) );
		addr += sizeof( inputChannel );

		for( i = 0; i < RECEIVER_CHANNELS; i++)
		{
			Receiver::setChannelMin( i, eeprom_read_word( (const uint16_t *)addr ) );
			addr += sizeof( WORD );
			Receiver::setChannelMax( i, eeprom_read_word( (const uint16_t *)addr ) );
			addr += sizeof( WORD );
		}

		Receiver::setChannelModeByte( eeprom_read_byte( (const uint8_t *) addr) );
    addr++;
    Adc::setLimit( eeprom_read_word((const uint16_t *) addr) );
    
#ifdef DEBUG_EEPROM
  out.printf_P(PSTR("EEPROM Size: %wx\r\n"), addr);
#endif
    
	  for( i=0;i < PWM_CHANNELS;i++ )
	  {
  	  for( BYTE ii= 0;ii < 2+MAX_SEGMENTS;ii++)
  	  {
        if( programArray[i][ii].algorithm >= ALGORITHMS_PWM )
        {
          programArray[i][ii].algorithm= 0;
        }
      }
    }          
	  for( i=PWM_CHANNELS;i < OUTPUT_CHANNELS;i++ )
	  {
  	  for( BYTE ii= 0;ii < 2+MAX_SEGMENTS;ii++)
  	  {
    	  if( programArray[i][ii].algorithm >= ALGORITHMS_SWITCH )
    	  {
      	  programArray[i][ii].algorithm= 0;
    	  }
  	  }
	  }
	}
	else
	{
		factoryDefault();
	}
#endif
}

void AlgorithmRepository::factoryDefault()
{
#ifdef ENABLE_DEBUG_UART
  Comm::printf_P( PSTR("Factory default!") );
#endif
	for( BYTE i=0;i < OUTPUT_CHANNELS;i++ )
	{
    inputChannel[i]= 0;
		for( BYTE ii= 0;ii < 2+MAX_SEGMENTS;ii++)
		{
			programArray[i][ii].algorithm= 0;
			programArray[i][ii].period= 400;
		}
	}

	for( BYTE i = 0; i < RECEIVER_CHANNELS; i++)
	{
		Receiver::setChannelMin( i, FACTORY_SET_MINIMUM );
		Receiver::setChannelMax( i, FACTORY_SET_MAXIMUM );
	}
	Receiver::setChannelModeByte( 0 );
  Adc::setLimit(0);

	writeToEEPROM();
}

void AlgorithmRepository::writeToEEPROM()
{
#ifndef DEBUGGING

#ifdef ENABLE_DEBUG_UART
	Comm::printf_P( PSTR("Store in EEPROM"));
#endif
	WORD addr= 0;
	eeprom_write_byte( (uint8_t *)addr++, 'J' );
	eeprom_write_byte( (uint8_t *)addr++, 'P' );
	eeprom_write_byte( (uint8_t *)addr++, ((RECEIVER_CHANNELS << 5) | OUTPUT_CHANNELS) );

	eeprom_write_block( (const void *)&programArray, (void *)addr, (size_t)sizeof( programArray ) );
	addr += sizeof( programArray );
	eeprom_write_block( (const void *)&inputChannel, (void *)addr, (size_t)sizeof( inputChannel ) );
	addr += sizeof( inputChannel );

	for( BYTE i = 0; i < RECEIVER_CHANNELS; i++)
	{
		eeprom_write_word( (uint16_t *)addr, Receiver::getChannelMin( i ));
		addr += sizeof( WORD );
		eeprom_write_word( (uint16_t *)addr, Receiver::getChannelMax( i ));
		addr += sizeof( WORD );
	}

	eeprom_write_byte( (uint8_t *) addr, Receiver::getChannelModeByte( ));
  addr++;
  eeprom_write_word((uint16_t *) addr, Adc::getLimit());
 #endif
}
