.include "m328PBdef.inc"
	
.equ FOSC_MHZ=16	;MHz
.equ DEL_mS=50		;mS (1-4095)
.equ Dbounce=20	;mS 
.equ DEL_NU=FOSC_MHZ*DEL_mS		;delay_mS routine: (1000*DEL_NU+6) cycles = F1
.equ DBcycles=FOSC_MHZ*Dbounce	;delay cycles for debouncing

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
	
	clr r26
	out DDRD, r26		;Initialize PORTD as input
	ser r26
	out DDRB, r26		;Initialize PORTB as output
	out DDRC, r26		;Initialize PORTC as output
	out PORTD, r26 		;Pull-up resistor enable
	
;Enable Interrupts
ldi r24, (1<<ISC11)|(0<<ISC10)	;Interrupt at the falling edge of INT1
sts EICRA, r24 		
ldi r24, (1<<INT1)|(0<<INT0)				;Enable the INT1 interrupt(PD3)
out EIMSK, r24
clr r17							; r17 is the interrupt counter
sbic PIND, 6
sei								; Enable global interrupts

;Main program


loop1:	
	clr r26
	loop3:
    ldi r21, 10
loop2:
	sbic PIND, 6
	rjmp unpressed
	cli
	ldi r20, (1<<INTF1)
	out EIFR, r20			;Clear INTF1
	rjmp main
unpressed:
	sei		 
	main:						; Disable interrupts if PD6 is pressed
	out PORTB, r26
	ldi r24, low(DEL_NU)
	ldi r25, high(DEL_NU)	; set delay (number of cycles)
	rcall delay_mS
	dec r21
	brne loop2	
	inc r26
	cpi r26, 16		; compare r26 with 16
	breq loop1
	rjmp loop3
	
	
	
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

ISR1:
	push r25
	push r24
	in r24, SREG
	push r24			;Save r24, r25, SREG
debouncing:
	ldi r24, (1<<INTF1)
	out EIFR, r24			;Clear INTF1
	ldi r24, low(DBcycles)
	ldi r25, high(DBcycles)	; set delay (number of cycles)
	rcall delay_mS			; wait 5ms
	sbic EIFR, 1		; If INTF1 is 0 exit debouncing
	rjmp debouncing
	
	cpi r17, 0x1F		; Compare r17 with 31
	brne flow
	subi r17, 0x20		; Subtract 32
flow:
	inc r17
	out PORTC, r17		; Show result
	pop r24
	out SREG, r24
	pop r24
	pop r25				;Restore r24, r25, SREG
	reti