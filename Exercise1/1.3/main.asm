.include "m328PBdef.inc"

.equ FOSC_MHZ=16	;MHz
.equ DEL_mS=50	;mS
.equ F1=(FOSC_MHZ*DEL_mS)-1	;number of cycles minus 1

 reset:
    ldi r24,low(RAMEND)
    out SPL,r24
    ldi r24,high(RAMEND)
    out SPH,r24  
    
	;Init PORTD as output
    ser r16         
    out DDRD, r16
	ldi r16, 0x01			; r16 is the counter
	
	ldi r24, low(F1)
    ldi r25, high(F1)
	
	start:
	set					; T=1 when moving left
	start_loop:
	out PORTD, r16
	rcall delay_mS	    ;  
	rcall delay_mS	    ;  
	rcall delay_mS	    ;  3*500 mS delay
	lsl r16
	cpi r16, 0
	breq rotate_right
	jmp start_loop
	
rotate_right:
	clt					; T=0 when moving right
	ldi r16, 0x80
	rcall delay_mS	    ;  500 mS delay
	inner:
	out PORTD, r16
	rcall delay_mS	    ;  
	rcall delay_mS	    ;  
	rcall delay_mS	    ;  3*500 mS delay
	lsr r16
	cpi r16, 0
	breq rotate_left
	jmp inner
	
rotate_left:
	ldi r16, 0x01
	rcall delay_mS	    
	jmp start

  
delay_mS:				; Function that requires (1000*(F1+1))-3 cycles
	movw r26, r24		; 1 cycle
	
    ldi	r23, 248	    ; (1 cycle)	
loop_inn1:
    dec r23		    	; 1 cycle
    nop			    	; 1 cycle
    brne loop_inn1	    ; 1 or 2 cycles
						
nop
delay_inner:		    ;total group delay 996 cycles
    ldi	r23, 249	    ; (1 cycle)	
loop_inn:
    dec r23		    	; 1 cycle
    nop			    	; 1 cycle
    brne loop_inn	    ; 1 or 2 cycles
     
    sbiw r26 ,1		    ; 2 cycles
    brne delay_inner	; 1 or 2 cycles
    ret			    	; 4 cycles