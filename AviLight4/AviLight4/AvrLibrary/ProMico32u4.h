/*
 * ProMicro32u4.h
 *
 * Created: 09.11.2014
 *  Author: Jörg
 */ 


#ifndef PRO_MICRO_H_
#define PRO_MICRO_H_

#define NUMBER_OF_LEDS      2
#define NUMBER_OF_BUTTONS   0

#define LED1                0
#define LED1_PORT           PORTB
#define LED1_DDR            DDRB
#define LED1_PIN            PINB
#define LED1_CONFIGURE()    LED1_DDR |= _BV(LED1)
#define LED1_ON()           LED1_PORT &= ~_BV(LED1)
#define LED1_OFF()          LED1_PORT |= _BV(LED1)
#define LED1_TOGGLE()       PINB |= _BV(LED1)

#define LED2                5
#define LED2_PORT           PORTD
#define LED2_DDR            DDRD
#define LED2_PIN            PIND
#define LED2_CONFIGURE()    LED2_DDR |= _BV(LED2)
#define LED2_ON()           LED2_PORT &= ~_BV(LED2)
#define LED2_OFF()          LED2_PORT |= _BV(LED2)
#define LED2_TOGGLE()       LED2_PIN |= _BV(LED2)



#endif /* PRO_MICRO_H_ */