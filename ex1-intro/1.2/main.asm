.include "m328PBdef.inc"
    
.def A =r16
.def B =r17
.def C =r18
.def D =r19
.def F0 =r20
.def F1 =r21
    
ldi A, 0x45
ldi B, 0x23
ldi C, 0x21
ldi D, 0x01			; Initialize values
ldi r22, 5			; Initialize counter for 5 iterations
    

loop:
    mov F0, A
    mov r24, B
    com F0
    com r24
    and F0,r24
    mov r24, C
    com r24
    and F0, r24
    or F0, D
    com F0	    	; F0 = (A'B'C' + D)'
    
    mov F1, A
    com F1
    or F1, C
    mov r24, B
    mov r25, D
    com r24
    com r25
    or r24, r25
    and F1, r24	    ; F1 =(A'+ C)(B'+D')
    nop   
    inc A			; increment A by 1
    inc B			
    inc B			; increment B by 2
    inc C
    inc C
    inc C
    inc C			; increment C by 4
    inc D
    inc D
    inc D
    inc D
    inc D			; increment D by 5
    dec r22
    brne loop 