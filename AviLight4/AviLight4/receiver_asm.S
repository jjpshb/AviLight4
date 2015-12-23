#include "avr/io.h"
#include <avr/interrupt.h>

#include "receiver.h"
#include "device.h"

.extern Receiver_ch
.extern Receiver_lastPortValue
.extern Receiver_counter

.global TIMER1_OVF_vect

#if HARDWARE_VERSION == 2
.global PCINT0_vect
#elif HARDWARE_VERSION == 3
.global PCINT1_vect
#elif HARDWARE_VERSION == 4
.global INT6_vect
.global PCINT0_vect
#endif

.func TIMER1_OVF_vect
TIMER1_OVF_vect:
				push 	r16
				in 		r16, _SFR_IO_ADDR(SREG)
				push	r16

				lds		r16, Receiver_counter
				inc		r16
				sts		Receiver_counter, r16
        brne  noTick

				lds		r16, Receiver_counter+1
				inc		r16
				sts		Receiver_counter+1, r16
noTick:
				pop		r16
				out 	_SFR_IO_ADDR(SREG), r16
				pop 	r16
				reti
.endfunc

#define INPORT				r22
#define CHANGE				r23
#define LOOP_COUNTER	r24
#define COUNTL				r26
#define COUNTH				r27

// BUg in Simulator !
;.global INT1_vect
;INT1_vect:
#if HARDWARE_VERSION == 2

.func PCINT0_vect
PCINT0_vect:

#elif HARDWARE_VERSION == 3

.func PCINT1_vect
PCINT1_vect:

#elif HARDWARE_VERSION == 4

.func PCINT0_vect
PCINT0_vect:
				push 	r16
				in 		r16, _SFR_IO_ADDR(SREG)

				push	COUNTL
				push	COUNTH
				lds		COUNTL, TCNT1L
				lds		COUNTH, TCNT1H

				push	r16
				push	r17
				push	INPORT
				push	ZL
				push	ZH
				ldi		ZL, lo8( Receiver_ch )
				ldi		ZH, hi8( Receiver_ch )

 				in		INPORT, _SFR_IO_ADDR( REC0_PIN )
        rjmp  loop

.endfunc
.func INT6_vect
INT6_vect:
				push 	r16
				in 		r16, _SFR_IO_ADDR(SREG)

				push	COUNTL
				push	COUNTH
				lds		COUNTL, TCNT1L
				lds		COUNTH, TCNT1H

				push	r16
				push	r17
				push	INPORT
				push	ZL
				push	ZH
				ldi		ZL, lo8( Receiver_ch+CHANNEL_SIZE )
				ldi		ZH, hi8( Receiver_ch+CHANNEL_SIZE )
 				in		INPORT, _SFR_IO_ADDR( REC1_PIN )

#endif
#if HARDWARE_VERSION < 4
				push 	r16
				in 		r16, _SFR_IO_ADDR(SREG)

				push	COUNTL
				push	COUNTH
				lds		COUNTL, TCNT1L
				lds		COUNTH, TCNT1H

				push	r16
				push	r17
				push	INPORT
				push	CHANGE
				push	LOOP_COUNTER
				push	ZL
				push	ZH

				in		INPORT, _SFR_IO_ADDR( REC_PIN )
				lds		CHANGE, Receiver_lastPortValue
				sts		Receiver_lastPortValue, INPORT	; actual bit value
				eor		CHANGE, INPORT				; bit set on change

				ldi		ZL, lo8( Receiver_ch )
				ldi		ZH, hi8( Receiver_ch )
				ldi		LOOP_COUNTER, RECEIVER_CHANNELS
#endif

loop:
				ldd		r16, Z + OFF_INBIT
#if HARDWARE_VERSION < 4
				and		r16, CHANGE
				breq	nextChannel
#endif

				and		r16, INPORT
				breq	subtract

				std		Z + OFF_B0, COUNTL
				std		Z + OFF_B1, COUNTH
				rjmp	nextChannel

subtract:
				ldd		r16, Z + OFF_FLAGS
				sbr		r16, _BV( CHF_NEW_PULSE );
				std		Z + OFF_FLAGS, r16

				mov		r17, COUNTL
				ldd		r16, Z + OFF_B0
				sub		r17, r16
				std 	Z + OFF_VALUE0, r17
				
				mov		r17, COUNTH
				ldd		r16, Z + OFF_B1
				sbc		r17, r16
				std		Z + OFF_VALUE1, r17

nextChannel:
#if HARDWARE_VERSION < 4
				adiw	ZL, CHANNEL_SIZE
				dec		LOOP_COUNTER
				brne	loop
#endif

				pop		ZH
				pop		ZL
#if HARDWARE_VERSION < 4
				pop		LOOP_COUNTER
				pop		CHANGE
#endif
				pop		INPORT
				pop		r17
				pop		r16
				pop		COUNTH
				pop		COUNTL
				out 	_SFR_IO_ADDR(SREG), r16
				pop 	r16
				reti

.endfunc

.end
