#include <avr/eeprom.h>

#include "Comm.h"
#include "receiver.h"
#include "AlgorithmRepository.h"
#include "adc.h"

#ifdef DEBUG_COMMUNICATION
#include "PrintStream.h"
extern PrintStream out;
#endif

extern "C" BYTE value2program( BYTE x );
extern "C" BYTE program2value( BYTE x );

/*BYTE Comm::mode;
BYTE Comm::buffer[ BUFFER_SIZE ];
BYTE Comm::sendBuffer[ BUFFER_SIZE ];
BYTE Comm::recptr;
BYTE Comm::offlineCounter;*/

Comm::Comm( ByteReceiver *_byteReceiver ) : byteReceiver(_byteReceiver)
{
	mode= COMM_MODE_DISCONNECTED;
	recptr= 0;
	offlineCounter= 0;
}

void Comm::cyclic( BYTE ticker )
{
  if( ticker )
  {
    if( offlineCounter != 0xff )
    {
      offlineCounter++;
    }
  }
}

BYTE Comm::isHostConnected()
{
  return offlineCounter != 0xff;  
}

void Comm::received( BYTE data )
{
	switch( mode )
	{
	case COMM_MODE_DISCONNECTED:
		if( data == STX )
		{
			recptr= 0;
			mode= COMM_RECEIVING;
		}
		break;

	case COMM_RECEIVING:
		if( data == ESCAPE )
		{
			mode= COMM_ESCAPE;
		}
	  else if( data == ETX )
		{
			processCommand();
			mode= COMM_MODE_DISCONNECTED;
			recptr= 0;
		}
		else
		{
			if( recptr < BUFFER_SIZE )
			{
				buffer[ recptr++ ]= data;
			}
			else
			{
				mode= COMM_MODE_DISCONNECTED;
				recptr= 0;
			}
		}
		break;

	case COMM_ESCAPE:
		if( recptr < BUFFER_SIZE )
		{
			buffer[ recptr++ ]= data;
			mode= COMM_RECEIVING;
		}
		else
		{
			mode= COMM_MODE_DISCONNECTED;
			recptr= 0;
		}
		break;

	default:
		mode= COMM_MODE_DISCONNECTED;
		recptr= 0;
	}
}

void Comm::processCommand()
{
#ifdef DEBUG_COMMUNICATION
  if( buffer[0] != 4 && buffer[0] != 11  &&  buffer[0] != 17 )
  {
    out.printf_P( PSTR("CMD: %bx %bx %bx\r\n"), buffer[0], buffer[1], buffer[2] );
  }
#endif

  offlineCounter= 0;
	if( buffer[0] == CMD_SET_PROGRAMM)
	{
		putch( STX );
		putch( CMD_TERMINAL );

		for( BYTE i=0;i < recptr;i++ )
		{
			print_hex( buffer[i] );
			putch(' ');
		}

		putch( ETX );
	}
  
	switch( buffer[0] )
	{
		case CMD_PING:
			sendBuffer[0]= 0;
			writeAnswer( buffer[0], sendBuffer, 1 );
			break;
		case CMD_INFO:
			writePSTR( buffer[0], PSTR_FIRMWARE_VERSION );
			break;
		case CMD_CHANNEL_INFO:
			sendBuffer[0]= PWM_CHANNELS;
			sendBuffer[1]= SWITCH_CHANNELS+SWITCH_POWER_CHANNELS;
			sendBuffer[2]= RECEIVER_CHANNELS;
      sendBuffer[3]= 2+MAX_SEGMENTS;
			writeAnswer( CMD_CHANNEL_INFO, sendBuffer, 4 );
			break;

		case CMD_RECEIVER:
			processReceiver();
			break;

		case CMD_SET_RECEIVER_CHANNEL_MODE:
			setReceiverChannelMode();
			// no break

		case CMD_GET_RECEIVER_CHANNEL_MODE:
			getReceiverChannelMode();
			break;

		case CMD_SET_CONTROLLING_CHANNEL:
			setControllingInputChannels();
			// no break

		case CMD_GET_CONTROLLING_CHANNEL:
			getControllingInputChannels();
			break;

		case CMD_GET_PROGRAMM:
			getProgram();
			break;

		case CMD_SET_PROGRAMM:
			setProgram();
			break;

    case CMD_SET_BATTERY_LIMIT:
      setVoltageLimit();
    
    case CMD_GET_VOLTAGE:
      getVoltage();
      break;      
      
    case CMD_WRITE_TO_EEPROM:
      AlgorithmRepository::writeToEEPROM();
      writeAnswer( buffer[0], sendBuffer, 1 );
      break;
      
    case CMD_READ_FROM_EEPROM:
      AlgorithmRepository::readFromEEPROM();
      AlgorithmRepository::initAlgorithms();
      writeAnswer( buffer[0], sendBuffer, 1 );
      break;
      
    case CMD_DUMP_PROGRAM_ARRAY:
      dumpProgramArray();
      break;
      
    case CMD_DUMP_EEPROM:
      dumpEeprom();
      break;
    
    case CMD_DUMP_RECEIVER:
      dumpReceiver();
      break;
      
    case CMD_DUMP_GLOBALS:
      dumpGlobals();
      break;
      
    case CMD_DUMP_CHANNELS:
      dumpChannels();
      break;
      
    case CMD_ENTER_BOOTLOADER:
      writeAnswer( buffer[0] );
      for(WORD w = 0;w  < 10000;w++)
      {        
        asm( "nop" ) ;
        asm( "nop" ) ;
      }
      //BOOTLOADER_START();
      break;
      
    case CMD_SET_LEARN_STICKMODE:
      if( recptr == 2 )
      {
        Receiver::learnStickMinMax( buffer[1] );
      }
      // kein break;

    case CMD_GET_LEARN_STICKMODE:
      sendBuffer[0]= Receiver::getLearnStickMode();
      writeAnswer( buffer[0], sendBuffer, 1 );
      break;

		default:

			putch( STX );
			putch( CMD_TERMINAL );

			for( BYTE i=0;i < recptr;i++ )
			{
				print_hex( buffer[i] );
				putch(' ');
			}

			putch( ETX );

			writeAnswer( CMD_UNKNONWN );
	}
}

void Comm::dumpChannel( BYTE ch )
{
  PROGRAM *p= AlgorithmRepository::getCurrentProgram(ch);
  ChannelData *cd= &AlgorithmRepository::channelData[ch];
  printf_P(PSTR("CH%bx IC:%bx S:%bx A:%bx OV:%bx"), ch, AlgorithmRepository::getInputChannel(ch), AlgorithmRepository::getChannelSegment(ch), p->algorithm,
    cd->oldValue);
}

void Comm::dumpEeprom()
{
  WORD addr= 0;
  for( BYTE i=0;i < 16; i++ )
  {
    putch( STX );
    putch( CMD_TERMINAL );
    for( BYTE ii=0;ii < 32;ii++ )
    {
      print_hex( eeprom_read_byte( (uint8_t *)addr++ ) );
      putch(' ');
    }
    
  	putch( ETX );
  }
}

void Comm::dumpProgramArray()
{
  BYTE *addr= (BYTE *)&AlgorithmRepository::programArray[0][0];
  for( BYTE i=0;i < 16; i++ )
  {
    putch( STX );
    putch( CMD_TERMINAL );
    for( BYTE ii=0;ii < 32;ii++ )
    {
      print_hex( *addr++ );
      putch(' ');
    }
    
    putch( ETX );
  }
}

void Comm::dumpReceiver()
{
  for(BYTE i=0;i < Receiver::numberOfInputChannels();i++)
  {
    CHANNEL *ch= Receiver::getChannel(i);
    
    printf_P(PSTR(" Bit:%bx F:%bx V:%wx"), ch->inbit, ch->flags, ch->value );
  }
}

void Comm::dumpChannels()
{
  BYTE ch;
  
  for(ch= 0;ch < OUTPUT_CHANNELS; ch++)
  {
    dumpChannel(ch);
  } 
}

void Comm::dumpGlobals()
{
  printf_P(PSTR("AvO:%wx CHE:%wx"), AlgorithmRepository::availableOuputs, AlgorithmRepository::channelEnable);
}

void Comm::getControllingInputChannels()
{
	byteReceiver->received( STX );

	sendSequence( buffer[0] );
	sendSequence( OUTPUT_CHANNELS );
	for( BYTE i=0; i < OUTPUT_CHANNELS;i ++)
	{
		sendSequence( AlgorithmRepository::getInputChannel( i ) );
	}

	byteReceiver->received( ETX );
}

void Comm::setControllingInputChannels()
{
	if( recptr == 3 )
	{
		AlgorithmRepository::setInputChannel( buffer[1], buffer[2] );
	}
}

void Comm::setReceiverChannelMode()
{
	if( recptr == 3 )
	{
		Receiver::setChannelMode( buffer[1], buffer[2] );
	}
}

void Comm::getReceiverChannelMode()
{
	BYTE i;
	BYTE *dest= &sendBuffer[1];
	sendBuffer[0]= Receiver::numberOfInputChannels();
	for(i = 0;i < Receiver::numberOfInputChannels();i++)	
	{
		*dest++= Receiver::getChannelMode( i );
		*dest++= 0;
	}

	writeAnswer( CMD_GET_RECEIVER_CHANNEL_MODE, sendBuffer, 1 + Receiver::numberOfInputChannels() );
}

void Comm::processReceiver()
{
	BYTE i;
	BYTE *dest= &sendBuffer[1];
	sendBuffer[0]= Receiver::numberOfInputChannels();
	for(i = 0;i < Receiver::numberOfInputChannels();i++)
	{
		BYTE value= Receiver::getValue( i );
		if( Receiver::isInputValid( i ) )
		{
			*dest++ = value2program( value );
		}
		else
		{
			*dest++ = 0xff;
		}

		*dest++= value;
	}

	writeAnswer( CMD_RECEIVER, sendBuffer, 1 + 2 * Receiver::numberOfInputChannels() );
}

void Comm::getProgram()
{
	if( recptr == 3 )
	{
		sendBuffer[0]= buffer[1];
		if(sendBuffer[0] >= OUTPUT_CHANNELS)
		{
			sendBuffer[0]= 0;
		}
		sendBuffer[1]= buffer[2];
		if( sendBuffer[1] >= (2 + MAX_SEGMENTS) )
		{
			sendBuffer[1]= 0;
		}
		
		BYTE *p= (BYTE *)AlgorithmRepository::getProgram( sendBuffer[0], sendBuffer[1] );

		sendBuffer[2]= *p++;
		sendBuffer[3]= *p++;
		sendBuffer[4]= *p++;
		sendBuffer[5]= *p++;

		writeAnswer( buffer[0], sendBuffer, 6 );
	}
}

void Comm::setProgram()
{
	sendBuffer[0]= buffer[1];
	if(sendBuffer[0] >= OUTPUT_CHANNELS)
	{
		sendBuffer[0]= 0;
	}
	sendBuffer[1]= buffer[2];
	if( sendBuffer[1] >= (2 + MAX_SEGMENTS) )
	{
		sendBuffer[1]= 0;
	}
		
	BYTE *p= (BYTE *)AlgorithmRepository::getProgram( sendBuffer[0], sendBuffer[1] );

	if( p != 0 )
	{
		*p++= buffer[3];
		sendBuffer[2]= buffer[3];
		*p++= buffer[4];
		sendBuffer[3]= buffer[4];
		*p++= buffer[5];
		sendBuffer[4]= buffer[5];
		*p++= buffer[6];
		sendBuffer[5]= buffer[6];

    AlgorithmRepository::initAlgorithm( sendBuffer[0] );
    
		PROGRAM *pr= AlgorithmRepository::getProgram( sendBuffer[0], sendBuffer[1] );

		printf_P( PSTR("CH:%bx S:%bx A:%bx P:%wx F:%bx"), sendBuffer[0], sendBuffer[1], pr->algorithm, pr->period, pr->flashcount );
	}
	else
	{
		sendBuffer[2]= 0;
		sendBuffer[3]= 0;
		sendBuffer[4]= 0;
		sendBuffer[5]= 0;
	}

	writeAnswer( buffer[0], sendBuffer, 6 );
}

void Comm::getVoltage()
{
  WORD value= Adc::getValue();
  
  sendBuffer[0]= value & 0xff;
  sendBuffer[1]= (value >> 8) & 0xff;
  
  value= Adc::getLimit();
  sendBuffer[2]= value & 0xff;
  sendBuffer[3]= (value >> 8) & 0xff;
  
  writeAnswer(buffer[0], sendBuffer, 4);
}

void Comm::setVoltageLimit()
{
  WORD value= buffer[1] | (buffer[2] << 8);
  
  Adc::setLimit( value );
}

void Comm::sendSequence( BYTE c )
{
	if( c == ETX || c == ESCAPE )
	{
		byteReceiver->received( ESCAPE );
	}
	byteReceiver->received( c );
}

void Comm::writeAnswer( BYTE command )
{
#ifdef DEBUG_COMMUNICATION
  if( command != 4 && command != 11  && command != 17 )
  { 
    out.printf_P( PSTR("S CMD: %bx\r\n"), command);
  }
#endif

	byteReceiver->received( STX );
	sendSequence( command );
	byteReceiver->received( ETX );
}

void Comm::writePSTR( BYTE command, const char *answer )
{
#ifdef DEBUG_COMMUNICATION
  if( command != 4 && command != 11  && command != 17 )
  {
    out.printf_P( PSTR("S CMD: %bx '"), command);
    out.print_P(answer);
    out.print_P( PSTR("'\r\n"));
  }
#endif

	byteReceiver->received( STX );
	sendSequence( command );
	char c;

	while( (c = pgm_read_byte( answer++ )) != 0 )
	{
		sendSequence( c );
	}

	byteReceiver->received( ETX );
}

void Comm::writeAnswer( BYTE command, const unsigned char *answer, BYTE size )
{
#ifdef DEBUG_COMMUNICATION
  if( command != 4 && command != 11 && command != 17 )
  {
    out.printf_P( PSTR("S CMD: %bx: "), command);
    out.dumpMemory((const char *)0, (unsigned char*)answer, size);
    out.print_P( PSTR("\r\n"));
  }
#endif

	byteReceiver->received( STX );
	sendSequence( command );

	while( size > 0 )
	{
		size--;
		sendSequence( *answer++ );
	}

	byteReceiver->received( ETX );
}
