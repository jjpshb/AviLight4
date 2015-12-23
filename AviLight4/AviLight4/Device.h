/*
 * Device.h
 *
 * Created: 02.12.2012 07:34:21
 *  Author: Jörg
 */ 


#ifndef DEVICE_H_
#define DEVICE_H_

#define __PRO_MICRO_32U4

#include "ProMico32u4.h"
#include <avr/io.h>

#define REVISION  "792"

// enable COM1 as debug Port
//#define DEBUG_COMPORT1

//#define DEBUG_COMMUNICATION
//#define DEBUG_ALGO
//#define DEBUG_RECEIVER
//#define DEBUG_ADC
//#define DEBUG_EEPROM
//#define DEBUG_OUTPUT

// max. number of segments per Output Channel
#define MAX_SEGMENTS			    5
#define MAX_PROGRAMS_SWITCH		3

#define RECEIVER_CHANNELS       2
#define HARDWARE_VERSION        4

#define VERSION								"1.0"
#define VERSION_REVISION			VERSION "." REVISION
#define PSTR_FIRMWARE_VERSION	PSTR( VERSION_REVISION )

#define ALGORITHMS_PWM				2
#define ALGORITHMS_SWITCH			4

// modify the value2program and program2value assembler routines
#define PROGRAM_HYSTERESIS	0x08

// the number of input channels (Receiver)
#define RECEIVER_CHANNELS		  2

// the number of output channels (PWM Output)
#define PWM_CHANNELS			    0
// the number of output channels (Switch Output)
#define SWITCH_CHANNELS		    8
// high current outputs
#define SWITCH_POWER_CHANNELS 0

#define OUTPUT_CHANNELS		(PWM_CHANNELS + SWITCH_CHANNELS + SWITCH_POWER_CHANNELS)
#define ALL_CHANNELS      (WORD)((1 << OUTPUT_CHANNELS)-1)


#define FACTORY_SET_MINIMUM	0x3d20
#define FACTORY_SET_MAXIMUM 0x7c80
#define FACTORY_SET_DIVISOR	0x3f

// The Receiver Inputs
#define REC0_PIN			PINB
#define REC0_DDR			DDRB
#define REC0          4

#define REC1_PIN			PINE
#define REC1_DDR			DDRE
#define REC1          6

// Outputs
#define PORT_OUT0     PORTF
#define DDR_OUT0      DDRF
#define BIT_OUT0      4
#define PORT_OUT1     PORTF
#define DDR_OUT1      DDRF
#define BIT_OUT1      5
#define PORT_OUT2     PORTF
#define DDR_OUT2      DDRF
#define BIT_OUT2      6
#define PORT_OUT3     PORTF
#define DDR_OUT3      DDRF
#define BIT_OUT3      7
#define PORT_OUT4     PORTB
#define DDR_OUT4      DDRB
#define BIT_OUT4      1
#define PORT_OUT5     PORTC
#define DDR_OUT5      DDRC
#define BIT_OUT5      6
#define PORT_OUT6     PORTB
#define DDR_OUT6      DDRB
#define BIT_OUT6      5
#define PORT_OUT7     PORTB
#define DDR_OUT7      DDRB
#define BIT_OUT7      6


#define F_CPU 16000000L

#ifdef DEBUG_COMPORT1

#define ATMEGA_MONITOR_PORTS    'p'
#define ATMEGA_MONITOR_TIMER    't'
#define ATMEGA_MONITOR_MEMORY   'm'
#define ATMEGA_MONITOR_CPU      'c'
#define ATMEGA_MONITOR_USART    'u'
//#define ATMEGA_MONITOR_I2C      'i'
#define ATMEGA_MONITOR_EEPROM   'e'

#endif /* DEBUG_COMPORT1 */
#endif /* DEVICE_H_ */