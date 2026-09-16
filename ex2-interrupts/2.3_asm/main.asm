.include "m328PBdef.inc"

.equ FOSC_MHZ=16	;MHz
.equ DEL_mS=500		;mS (1-4095)
.equ Dbounce=5		;mS 
.equ DEL_NU=(FOSC_MHZ*DEL_mS)-1		;cycles F1-1
.equ DBcycles=(FOSC_MHZ*Dbounce)-1  ;delay cycles for debouncing
.equ Counter = 51

.org 0x0
	rjmp reset
.org 0x4
	rjmp ISR1
	reti

reset:
	;Initialize Stack Pointer
	ldi r24, LOW(RAMEND)
	out SPL, r24
	ldi r24, HIGH(RAMEND)
	out SPH, r24
	
	ser r16
	out DDRB, r16		; Set PORTB as output
	clr r16
	out PORTB, r16		; Turn off all leds
	ldi r16, (1<<INT1)
	out EIMSK, r16		; Enable INT1 in EIMSK
	ldi r16, (1<<ISC11)|(0<<ISC10)
	sts EICRA, r16		; Interrupt is sensed at the falling edge of INT1 
	sei
	
	loop:
	rjmp loop			; main program

ISR1:
	push r25
	push r24
	in r24, SREG
	push r24			;Save r24, r25, SREG
deboucing:
	ldi r24, (1<<INTF1)
	out EIFR, r24			;Clear INTF1
	ldi r24, low(DBcycles)
	ldi r25, high(DBcycles)	; set delay (number of cycles)
	rcall delay_mS			;
	sbic EIFR, 1		; If INTF1 is 0 continue
	rjmp deboucing
	
	ldi r16, 0xFF
	out PORTB, r16			; Turn on all leds
	ldi r24, low(DEL_NU)
	ldi r25, high(DEL_NU)	; set delay (number of cycles)
	rcall delay_mS
	ldi r16, 0x01
	out PORTB, r16			; Turn on only PB0
	sei
	
	ldi r17, Counter
	ldi r24, low(50*16)
	ldi r25, high(50*16)	; set delay 50ms (number of cycles)
rep:
	rcall delay_mS
	dec r17
	brge rep
	
	ldi r16, 0x00
	out PORTB, r16			; Turn off all leds
	
	pop r24
	out SREG, r24
	pop r24
	pop r25				;Restore r24, r25, SREG
	reti
	


delay_mS:				; Function that requires (1000*(F1+1))-4 cycles
    movw r26, r24		; 1 cycle,		NOTE: the contents of r26,r27 are changed
	ldi	r23, 248	    ; 1 cycle
loop_inn1:
    dec r23		    	; 1 cycle
    nop			    	; 1 cycle
    brne loop_inn1	    ; 1 or 2 cycles
						

delay_inner:		    ;total group delay 996 cycles
    ldi	r23, 249	    ; 1 cycle
loop_inn:
    dec r23		    	; 1 cycle
    nop			    	; 1 cycle
    brne loop_inn	    ; 1 or 2 cycles
     
    sbiw r26 ,1		    ; 2 cycles
    brne delay_inner	; 1 or 2 cycles
 
    ret			    	; 4 cycles