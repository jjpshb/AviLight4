/*
 * AviLight4.cpp
 *
 * Created: 27.11.2015 12:53:28
 *  Author: Jörg
 */ 


#include <avr/io.h>
#include <avr/interrupt.h>

#include "Device.h"

#include "DelayTimerW.h"

#include "USBUsart.h"
#include "SerialDebugMonitor.h"
#include "receiver.h"
#include "Comm.h"
#include "AlgorithmRepository.h"
#include "adc.h"

bool getTickerFlag();
void initialisation();
void cyclicTasks( BYTE tickerFlag );
void nightRider();

// USB Uart
extern USBUsart usbUart;
Comm comm(&usbUart);
USBUsart usbUart(&comm);

// delay for yellow LED
DelayTimerWord delay( (WORD*)Receiver::getTickerAddr(), 244);

WORD oldTicker;

#ifdef DEBUG_OUTPUT
static BYTE oldValue[8];
#endif


int main(void)
{
  initialisation();
  
#ifdef DEBUG_COMPORT1  
  out.print_P( PSTR("\x1b[2J\x1b[HAvilight 4\r\n"));
#endif

  oldTicker= Receiver::getTicker();
  
  nightRider();  
  while(1)
  {
    bool tickerflag= getTickerFlag();
    cyclicTasks(tickerflag);
    
    AlgorithmRepository::program( tickerflag, 0);      
  }
}


void setOutput( BYTE channel, BYTE value )
{
  channel &= 7;
  
#ifdef DEBUG_OUTPUT  
  if( oldValue[channel] != value )
  {
    oldValue[channel]= value;
    out.printf_P(PSTR("set Out %bx to %bx\r\n"), channel, value);
  }
#endif
  
  if( value )
  {
    switch( channel )
    {
      case 0: PORT_OUT0 |= _BV(BIT_OUT0); break;
      case 1: PORT_OUT1 |= _BV(BIT_OUT1); break;
      case 2: PORT_OUT2 |= _BV(BIT_OUT2); break;
      case 3: PORT_OUT3 |= _BV(BIT_OUT3); break;
      case 4: PORT_OUT4 |= _BV(BIT_OUT4); break;
      case 5: PORT_OUT5 |= _BV(BIT_OUT5); break;
      case 6: PORT_OUT6 |= _BV(BIT_OUT6); break;
      case 7: PORT_OUT7 |= _BV(BIT_OUT7); break;
    }
  }
  else  
  {
    switch( channel )
    {
      case 0: PORT_OUT0 &= ~_BV(BIT_OUT0); break;
      case 1: PORT_OUT1 &= ~_BV(BIT_OUT1); break;
      case 2: PORT_OUT2 &= ~_BV(BIT_OUT2); break;
      case 3: PORT_OUT3 &= ~_BV(BIT_OUT3); break;
      case 4: PORT_OUT4 &= ~_BV(BIT_OUT4); break;
      case 5: PORT_OUT5 &= ~_BV(BIT_OUT5); break;
      case 6: PORT_OUT6 &= ~_BV(BIT_OUT6); break;
      case 7: PORT_OUT7 &= ~_BV(BIT_OUT7); break;
    }
  }
}

bool getTickerFlag()
{
 bool tickerflag= false;
 WORD ticker= Receiver::getTicker();
 if( oldTicker != ticker )
 {
   oldTicker= ticker;
   tickerflag= true;
 }  
 
 return tickerflag;
}

void initialisation()
{
  LED1_CONFIGURE();
  LED2_CONFIGURE();

#ifdef DEBUG_OUTPUT
  for(BYTE i=0;i < 8;i++ )
  {
    oldValue[i] = 1;
  }
#endif

  PORT_OUT0 &= ~_BV(BIT_OUT0);
  DDR_OUT0 |= _BV(BIT_OUT0);
  PORT_OUT1 &= ~_BV(BIT_OUT1);
  DDR_OUT1 |= _BV(BIT_OUT1);
  PORT_OUT2 &= ~_BV(BIT_OUT2);
  DDR_OUT2 |= _BV(BIT_OUT2);
  PORT_OUT3 &= ~_BV(BIT_OUT3);
  DDR_OUT3 |= _BV(BIT_OUT3);
  PORT_OUT4 &= ~_BV(BIT_OUT4);
  DDR_OUT4 |= _BV(BIT_OUT4);
  PORT_OUT5 &= ~_BV(BIT_OUT5);
  DDR_OUT5 |= _BV(BIT_OUT5);
  PORT_OUT6 &= ~_BV(BIT_OUT6);
  DDR_OUT6 |= _BV(BIT_OUT6);
  PORT_OUT7 &= ~_BV(BIT_OUT7);
  DDR_OUT7 |= _BV(BIT_OUT7);
  
  Receiver::init();
  Adc::init();

  LED1_OFF();
  LED2_OFF();
  sei();
  
  delay.start();
  
  AlgorithmRepository::init(); 
}

void cyclicTasks( BYTE tickerflag )
{
  if( USBUsart::isInitialized() )
  {
    LED2_ON();
  }
  else
  {
    LED2_OFF();
  }
    
  USBUsart::cyclic();
  
#ifdef DEBUG_COMPORT1
  uart2.cyclic();
#endif

  Receiver::cyclic();
  comm.cyclic(tickerflag);
  Adc::cyclic(tickerflag);

  if( delay.isDone() )
  {
    delay.start();
    LED1_TOGGLE();
  }  
}

void nightRider()
{
  BYTE loops= 0;
  BYTE output= 0;
  bool direction= true;
  BYTE counter= 0;
  
  setOutput(0, 1);
  while(loops < 48)
  {
    bool tickerflag= getTickerFlag();
    cyclicTasks(tickerflag);

    if( tickerflag )
    {
      if( counter++ > 10 )
      {
        counter= 0;
        
        setOutput( output, 0);
        if( direction )
        {
          if( output >= 7 )
          {
            direction= false;
          }
          else
          {
            output++;
          }
        }
        else
        {
          if( output == 0 )
          {
            direction= true;
          }
          else
          {
            output--;
          }
        }
        setOutput( output, 1);
        loops++;
      }      
    }
  }
  setOutput(0, 0);
}
