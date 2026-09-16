.include "m328PBdef.inc"

.equ FOSC_MHZ=16	;MHz
.equ DEL_mS=500		;mS (1-4095)
.equ DEL_NU=FOSC_MHZ*DEL_mS		;delay_mS routine


.cseg

.org 0x0
    rjmp reset
	
table:
.DB	5,25,46,66,87,107,127,148,169,189,210,230,251 

reset:

;Initialize Stack Pointer
	ldi r24, LOW(RAMEND)
	out SPL, r24
	ldi r24, HIGH(RAMEND)
	out SPH, r24

	ldi r24, low(DEL_NU)
	ldi r25, high(DEL_NU)	; set delay (number of cycles)


ldi r16, (1<<WGM10) | (1<<COM1A1)	; Non-inverting Mode
sts TCCR1A, r16						; Fast PWM, 8-bit Mode
ldi r16, (1<<WGM12) | (1<<CS11)		; Prescaler is 8 (frequency = f.clk/8)
sts TCCR1B, r16

ldi r16, 0b111111
out DDRB, r16			; Initialize PORTB as output
clr r16
out DDRD, r16			; Initialize PORTD as input
out PORTB, r16			; Turn off all LEDs
ser r16
out PORTD, r16			; Pull-up resistors

ldi ZH, HIGH(table*2)
ldi ZL, LOW(table*2)
adiw ZL, 6				; Initial pointer Z value is 127
ldi r17, 6				; counter for boundary control

lpm r18, Z
sts OCR1AL, r18			; Initial dc is 50%


main:
	sbis PIND, 1
	rjmp PD1_pressed
	sbis PIND, 2
	rjmp PD2_pressed
	
	rjmp main
	
PD1_pressed:
	rcall delay_mS
	cpi r17, 12
	breq main
	inc r17
	adiw ZL, 1
	lpm r18, Z
	sts OCR1AL, r18	
	rjmp main
	
PD2_pressed:
	rcall delay_mS
	cpi r17, 0
	breq main
	dec r17
	sbiw ZL, 1
	lpm r18, Z
	sts OCR1AL, r18	
	rjmp main
	
delay_mS:				; Function that requires (1000*(F1+1))-3 cycles
    push r24		    ; (2 cycles)	    
    push r25		    ; (2 cycles) Save r24:r25
	
	ldi	r23, 246	    ; (1 cycle)	
loop_inn1:
    dec r23		    	; 1 cycle
    nop			    	; 1 cycle
    brne loop_inn1	    ; 1 or 2 cycles
						
nop
nop
delay_inner:		    ;total group delay 996 cycles
    ldi	r23, 249	    ; (1 cycle)	
loop_inn:
    dec r23		    	; 1 cycle
    nop			    	; 1 cycle
    brne loop_inn	    ; 1 or 2 cycles
     
    sbiw r24 ,1		    ; 2 cycles , NOTE: the contents of r24,r25 are changed!!
    brne delay_inner	; 1 or 2 cycles
 
	pop r25		    	; (2 cycles)
    pop	r24		    	; (2 cycles) Restore r24:r25
    ret			    	; 4 cycles