/*
 * LEDs.h
 *
 * Created: 22.12.2015 11:13:07
 *  Author: Jörg
 */ 


#ifndef LEDS_H_
#define LEDS_H_

	/* Includes: */
	#include "../LUFA/LUFA/Common/Common.h"

	/* Preprocessor Checks: */
	#if !defined(__INCLUDE_FROM_LEDS_H)
	#error Do not include this file directly. Include LUFA/Drivers/Board/LEDS.h instead.
	#endif

	/* Private Interface - For use in library only: */
	#if !defined(__DOXYGEN__)
	/* Macros: */
	#define LEDS_PORTB_LEDS       (LEDS_LED1)
	#define LEDS_PORTD_LEDS       (LEDS_LED2)
	#endif

	/* Public Interface - May be used in end-application: */
	/* Macros: */
	/** LED mask for the first LED on the board. */
	#define LEDS_LED1        (1 << 0)

	/** LED mask for the second LED on the board. */
	#define LEDS_LED2        (1 << 5)

	/** LED mask for all the LEDs on the board. */
	#define LEDS_ALL_LEDS    (LEDS_LED1 | LEDS_LED2)

	/** LED mask for none of the board LEDs. */
	#define LEDS_NO_LEDS     0

	/* Inline Functions: */
	#if !defined(__DOXYGEN__)
	static inline void LEDs_Init(void)
	{
  	DDRB  |=  LEDS_PORTB_LEDS;
  	PORTB |=  LEDS_PORTB_LEDS;
  	DDRD  |=  LEDS_PORTD_LEDS;
  	PORTD |=  LEDS_PORTD_LEDS;
	}

	static inline void LEDs_Disable(void)
	{
  	DDRB  &= ~LEDS_PORTB_LEDS;
  	PORTB &= ~LEDS_PORTB_LEDS;
  	DDRD  &= ~LEDS_PORTD_LEDS;
  	PORTD &= ~LEDS_PORTD_LEDS;
	}

	static inline void LEDs_TurnOnLEDs(const uint8_t LEDMask)
	{
  	PORTB &= ~(LEDMask & LEDS_PORTB_LEDS);
  	PORTD &= ~(LEDMask & LEDS_PORTD_LEDS);
	}

	static inline void LEDs_TurnOffLEDs(const uint8_t LEDMask)
	{
  	PORTB |=  (LEDMask & LEDS_PORTB_LEDS);
  	PORTD |=  (LEDMask & LEDS_PORTD_LEDS);
	}

	static inline void LEDs_SetAllLEDs(const uint8_t LEDMask)
	{
  	PORTB = ((PORTB |  LEDS_PORTB_LEDS) & ~(LEDMask & LEDS_PORTB_LEDS));
  	PORTD = ((PORTD |  LEDS_PORTD_LEDS) & ~(LEDMask & LEDS_PORTD_LEDS));
	}

	static inline void LEDs_ChangeLEDs(const uint8_t LEDMask,
	const uint8_t ActiveMask)
	{
  	PORTB = ((PORTB |  (LEDMask & LEDS_PORTB_LEDS)) & ~(ActiveMask & LEDS_PORTB_LEDS));
  	PORTD = ((PORTD |  (LEDMask & LEDS_PORTD_LEDS)) & ~(ActiveMask & LEDS_PORTD_LEDS));
	}

	static inline void LEDs_ToggleLEDs(const uint8_t LEDMask)
	{
  	PINB  = (LEDMask & LEDS_PORTB_LEDS);
  	PIND  = (LEDMask & LEDS_PORTD_LEDS);
	}

	static inline uint8_t LEDs_GetLEDs(void) ATTR_WARN_UNUSED_RESULT;
	static inline uint8_t LEDs_GetLEDs(void)
	{
  	return ((~PORTB & LEDS_PORTB_LEDS) | (~PORTD & LEDS_PORTD_LEDS));
	}
	#endif



#endif /* LEDS_H_ */