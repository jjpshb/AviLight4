/*
 * USBUsart.h
 *
 * Created: 02.12.2012 07:30:52
 *  Author: Jörg
 */ 


#ifndef USBUSART_H_
#define USBUSART_H_

#include "ByteReceiver.h"

typedef void (*pfCharReceived)( char ch );

class USBUsart : public ByteReceiver
{
public:
  USBUsart( pfCharReceived _receivedFunc );
  USBUsart( ByteReceiver *byteReceiver );
  
  //static USBUsart instance;
  
  static void cyclic();
  
  static bool isInitialized();

  static bool char_avail(void);

  static char getch(void);
  
  static void putch( char c );
  
  virtual void received( BYTE data );
  
private:

  static bool initialized;
  static pfCharReceived receivedFunc;
  static ByteReceiver *byteReceiver;
};





#endif /* USBUSART_H_ */