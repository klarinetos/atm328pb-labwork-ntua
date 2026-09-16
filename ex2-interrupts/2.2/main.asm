.include "m328PBdef.inc"
	
.equ FOSC_MHZ=16	;MHz
.equ DEL_mS=1000		;mS (1-4095)
;.equ Dbounce=5		;mS 
.equ DEL_NU=FOSC_MHZ*DEL_mS		;delay_mS routine: (1000*DEL_NU+6) cycles = F1
;.equ DBcycles=FOSC_MHZ*Dbounce	;delay cycles for debouncing

.org 0x0
	rjmp reset
.org 0x2
	rjmp ISR0
	reti

reset:
;Initialize Stack Pointer
	ldi r24, LOW(RAMEND)
	out SPL, r24
	ldi r24, HIGH(RAMEND)
	out SPH, r24
	
	clr r24
	out DDRB, r24		;Initialize PORTB as input
	ser r24
	out DDRC, r24		;Initialize PORTC as output
	out PORTB, r24 		;Pull-up resistor enable
	
;Enable Interrupts
ldi r24, (1<<ISC01)|(0<<ISC00)	;Interrupt at the falling edge of INT0
sts EICRA, r24 		
ldi r24, (1<<INT0)				;Enable the INT0 interrupt(PD2)
out EIMSK, r24
sei								; Enable Global Interrupts

;Main program
loop1:	
	clr r26
loop2:						
	out PORTC, r26
	ldi r24, low(DEL_NU)
	ldi r25, high(DEL_NU)	; set delay (number of cycles)
	rcall delay_mS
	inc r26
	cpi r26, 32		; compare r26 with 32
	breq loop1
	rjmp loop2

ISR0:
	in r10, PINB		; Save the state of PΙΝB

	push r25
	push r24
	in r24, SREG
	push r24			;Save r24, r25, SREG
deboucing:
	ldi r24, (1<<INTF0)
	out EIFR, r24			;Clear INTF0
	ldi r24, low(16*5)
	ldi r25, high(16*5)	; set delay (number of cycles)
	rcall delay_mS			; wait 5ms
	sbic EIFR, 0		; If INTF0 is 0 continue
	rjmp deboucing
	
	ldi r19, 0b00011111	; counter for buttons pressed
	ldi r18, 5			; counter for 5 iterations
count_buttons:
	dec r18
	ror r10
	brcc check
	lsr r19
check:
	cpi r18, 0
	brne count_buttons
	
	out PORTC, r19		; output result
	ldi r24, low(DEL_NU)
	ldi r25, high(DEL_NU)	; set delay (number of cycles)
	rcall delay_mS			; wait 1000ms to see the result
	pop r24
	out SREG, r24
	pop r24
	pop r25				;Restore r24, r25, SREG
	reti
	
; delay of 1000*F1+6 cycles
delay_mS:

; total delay of next 4 instruction group = 1+(249*4-1) = 996 cycles
   ldi	r23, 249	    ; (1 cycle)	
loop_inn:
    dec r23		    	; 1 cycle
    nop			    	; 1 cycle
    brne loop_inn	    ; 1 or 2 cycles
     
    sbiw r24 ,1		    ; 2 cycles , NOTE: the contents of r24,r25 are changed!!
    brne delay_mS		; 1 or 2 cycles
 
    ret			    	; 4 cycles