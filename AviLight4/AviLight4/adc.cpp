/*
 * adc.cpp
 *
 * Created: 20.01.2013 15:15:42
 *  Author: Jörg
 */ 
#include <avr/io.h>

#include "adc.h"
#include "Comm.h"

#ifdef DEBUG_ADC
#include "PrintStream.h"
extern PrintStream out;
static BYTE cnt;
#endif

WORD Adc::value;
WORD Adc::limit;

void Adc::init()
{
  value= 0;
  
#if HARDWARE_VERSION == 4
  ADMUX = (1 << REFS0) | (0x02 << MUX0);
  ADCSRA= _BV(ADEN) | (7 << ADPS0);
  ADCSRB= _BV(MUX5);
#else
  PRR &= ~_BV(PRADC);
  ADMUX = (1 << REFS0) | (7 << MUX0);
  ADCSRA= _BV(ADEN) | (7 << ADPS0);
  ADCSRB= 0;
#endif

  ADCSRA |= _BV(ADSC);
}

void Adc::cyclic( BYTE tickerFlag )
{  
  if( (ADCSRA & _BV(ADSC) ) == 0)
  {
    value= ADC;
    
    ADCSRA |= _BV(ADSC);
  }
  
#ifdef DEBUG_ADC
  if( tickerFlag )
  {
    if( ++cnt == 0)
    {
      out.printf_P( PSTR("ADC: %wx"), value );
    }
  }
#endif
}

BYTE Adc::numberOfCells()
{
  if( value < TO_ADC( 4.3 ))  
  {
    return 1;
  }
  else if( value < TO_ADC( 8.5 ))
  {
    return 2;
  }
  else if( value < TO_ADC( 12.7 ))
  {
    return 3;
  }
  else if( value < TO_ADC( 16.9 ))
  {
    return 4;
  }
  else if( value < TO_ADC( 21.1 ))
  {
    return 5;
  }
  else
  {
    return 6;
  }
}

BYTE Adc::isLimitReached()
{
  return (limit != 0 && value < limit);  
}