/*
 * Monitor.cpp
 *
 * Created: 23.12.2015 08:37:39
 *  Author: Jörg
 */ 

#include "SerialDebugMonitor.h"
#include "receiver.h"
#include "Comm.h"
#include "AlgorithmRepository.h"
#include "adc.h"

#ifdef DEBUG_COMPORT1

SerialDebugMonitor monitor;

PrintStream out( &uart2 );
UartNS uart2( UART1, 111111L, &monitor); //115200L

static char monitorBuffer[40];

SerialDebugMonitor::SerialDebugMonitor() : AtmegaMonitor( monitorBuffer, sizeof(monitorBuffer), &out) 
{  
}

BYTE SerialDebugMonitor::executeCommand( char ch )
{
  switch( ch )
  {
    case 'r':
    out.printf_P( PSTR("any input valid:%bx\r\n"), Receiver::isAnyInputValid());
    
    for( BYTE i=0;i < RECEIVER_CHANNELS;i++)
    {
      CHANNEL *ch= Receiver::getChannel(0);
      
      out.printf_P( PSTR("Channel %bx:\r\n"), i);
      
      out.printf_P( PSTR("Flags:        %bx\r\n"), ch->flags);
      out.printf_P( PSTR("lastChange: %wx\r\n"), ch->lastChange );
      out.printf_P( PSTR("value:      %wx\r\n"), ch->value );
      out.printf_P( PSTR("min value:  %wx\r\n"), ch->minValue );
      out.printf_P( PSTR("max value:  %wx\r\n"), ch->maxValue );
      out.printf_P( PSTR("B-value:      %bx\r\n"), ch->bValue);
    }
    return 1;

    case 'a':

    for(BYTE i=0;i < OUTPUT_CHANNELS;i++)
    {
      PROGRAM *pr= AlgorithmRepository::getCurrentProgram( i );
      BYTE inChannel= AlgorithmRepository::getInputChannel( i )-1;
      BYTE seg= AlgorithmRepository::getChannelSegment( i );
      ChannelData *cd= &AlgorithmRepository::channelData[i];

      out.printf_P( PSTR("CH%bx A%bx P:%wx FC:%bx "), i, pr->algorithm, pr->period, pr->flashcount );
      out.printf_P( PSTR("ICH:%bx SEG:%bx C:%wx S:%bx F:%bx\r\n"), inChannel, seg, cd->counter, cd->stickValue, cd->flags ) ;
    }
    return 1;

    case 'v':

    out.printf_P(PSTR("ADC: %wx\r\n"), Adc::getValue());
    out.printf_P(PSTR("LIM: %wx\r\n"), Adc::getLimit());
    return 1;
    
    case 'E':
    AlgorithmRepository::readFromEEPROM();
    return 1;
    
    case 'f':
    AlgorithmRepository::factoryDefault();
    AlgorithmRepository::writeToEEPROM();
    return 1;
  }
  
  return AtmegaMonitor::executeCommand(ch);
}

#endif /* DEBUG_COMPORT1 */
