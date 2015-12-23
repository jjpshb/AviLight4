/*
 * Monitor.h
 *
 * Created: 23.12.2015 08:35:34
 *  Author: Jörg
 */ 


#ifndef MONITOR_H_
#define MONITOR_H_


#include "Device.h"

#ifdef DEBUG_COMPORT1

#include "AtmegaMonitor.h"
#include "PrintStream.h"
#include "UartNS.h"

// Debugging, prints output to uart2
extern PrintStream out;
// uart used for debug output, using on chip Uart
extern UartNS uart2;

class SerialDebugMonitor : public AtmegaMonitor
{
  public:
  SerialDebugMonitor();
    
  protected:

  virtual BYTE executeCommand( char ch );
};


#endif /* DEBUG_COMPORT1 */

#endif /* MONITOR_H_ */