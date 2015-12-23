

.global div16by8
.global mul8by8

.func div16by8

;
; Divide r25:r24 by r22
;
div16by8:
			ldi		r19, 8
			clr		r18
div8loop:

			lsl		r24
			rol		r25
			rol		r18

			cp		r25, r22
			cpc		r18, r1
			brcs	div8noSub

			sub		r25, r22
			sbc		r18, r1
			inc		r24

div8noSub:
			dec		r19
			brne	div8loop

			clr		r25
			ret

.endfunc

.func mul8by8
; Multiply r24 by r22, Result in r25:r24
mul8by8:
			ldi		r21, 9
			clr		r25
			sub		r18, r18

mul8by8loop:
			brcc	mul8byNoAdd

			add		r25, r22
			adc		r18, r1

mul8byNoAdd:
			ror		r18
			ror		r25
			ror		r24

			dec		r21
			brne	mul8by8loop
			ret
.endfunc

			
.end
