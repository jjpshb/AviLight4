/*
 * USBUsart.cpp
 *
 * Created: 02.12.2012 07:31:32
 *  Author: Jörg
 */ 

#include <avr/io.h>

#include "USBUsart.h"

extern "C" 
{
#include "usb_serial.h"
};

#define CPU_PRESCALE(n) (CLKPR = 0x80, CLKPR = (n))

bool USBUsart::initialized;
pfCharReceived USBUsart::receivedFunc;
ByteReceiver *USBUsart::byteReceiver;

USBUsart::USBUsart( pfCharReceived _receivedFunc )
{
  receivedFunc= _receivedFunc;
  byteReceiver= 0;
  initialized= false;

	CPU_PRESCALE(0);
	usb_init();
}

USBUsart::USBUsart( ByteReceiver *_byteReceiver )
{
  receivedFunc= 0;
  byteReceiver= _byteReceiver;
  initialized= false;

  CPU_PRESCALE(0);
  usb_init();
}

bool USBUsart::isInitialized()
{ 
  return initialized; 
}

void USBUsart::cyclic()
{
  if( !initialized )
  {
    if( usb_configured() )
    {
      initialized= true;
      usb_serial_flush_input();
    }
  }
  else
  {
    if( receivedFunc != 0 )
    {
      if( char_avail() )
      {
        receivedFunc( getch() );
      }
    }
    if( byteReceiver != 0 )
    {
      if( char_avail() )
      {
        byteReceiver->received( getch() );
      }
    }
  }
}

bool USBUsart::char_avail(void)
{
  return usb_serial_available() > 0;
}

char USBUsart::getch(void)
{
  return usb_serial_getchar() & 0xff;
}
  
void USBUsart::putch( char c )
{
  usb_serial_putchar( c );
}

void USBUsart::received( BYTE data )
{
  USBUsart::putch( data );
}