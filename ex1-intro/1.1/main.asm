.include "m328PBdef.inc"

.equ FOSC_MHZ=16	;MHz
.equ DEL_mS=100	;mS
.equ F1=(FOSC_MHZ*DEL_mS)-1	;number of cycles minus 1
	
 reset:
    ldi r24,low(RAMEND)
    out SPL,r24
    ldi r24,high(RAMEND)
    out SPH,r24  
    
    ser r24
    out DDRD,r24
    out PORTD,r24
    
    clr r26
    
loop1:
    ldi r24, low(F1)
    ldi r25, high(F1)
    rcall delay_mS
    com r26
    out PORTD,r26
    rjmp loop1
    
    
delay_mS:				; Function that requires (1000*(F1+1))-3 cycles
    ldi	r23, 248	    ; (1 cycle)	
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
 
    ret			    	; 4 cycles