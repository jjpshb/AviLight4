#include "avr/io.h"

.global value2program
.global program2value

// divide r24 by 51
// r25 * 5 / 256
value2program:
		mov		r18, r24
		clr		r19
		clr		r25
		lsl		r18
		rol		r19
		lsl		r18
		rol		r19
		add		r24, r18
		adc		r25, r19
		mov		r24, r25
		clr		r25
		ret

program2value:
		mov		r18, r24
		lsl		r18
		add		r24, r18
		lsl		r18
		lsl		r18
		lsl		r18
		add		r24, r18
		lsl		r18
		add		r24, r18
		clr		r25
		ret

.end


