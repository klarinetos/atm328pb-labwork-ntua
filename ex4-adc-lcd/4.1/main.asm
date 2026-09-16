.include "m328PBdef.inc"

.equ PD0=0
.equ PD1=1
.equ PD2=2
.equ PD3=3
.equ PD4=4
.equ PD5=5
.equ PD6=6
.equ PD7=7

.org 0x0
    rjmp reset
.org 0x2A
	rjmp ADCint
	reti
	
	
reset:

	ldi r24, LOW(RAMEND)
	out SPL, r24
	ldi r24, HIGH(RAMEND)
	out SPH, r24			;Initialize Stack Pointer
	ser r24
    out DDRD, r24 				; set PORTD as output
	out DDRB, r24

;ldi r16, (1<<WGM10) | (1<<COM1A1)	; Non-inverting Mode
;sts TCCR1A, r16						; Fast PWM, 8-bit Mode
;ldi r16, (1<<WGM12) | (1<<CS11)		; Prescaler is 8 (frequency = f.clk/8)
;sts TCCR1B, r16
ldi r16, 0b01100010					; 
sts ADMUX, r16						; 
ldi r16, 0b10001110					;
sts ADCSRA, r16
clr r24
rcall lcd_init
ldi r24, low(100)
ldi r25, high(100) 			; delay 100 mS 
rcall wait_msec
rcall lcd_clear_display
ldi r24, low(1000)
ldi r25, high(1000) 			; delay 1000 mS
rcall wait_msec


lds r17, ADCSRA
ldi r18, 0b01000000
or r18, r17
sts ADCSRA, r18

main:
sei
rjmp main


ADCint:
	rcall lcd_clear_display
	ldi r24, 5
	lds r19, ADCH
	mov r20, r19
	subi r20, 255
	breq extrem
	mul r19, r24
	ldi r24, 0x30
	add r24, r1
	call lcd_data
	ldi r24, 0x2C
	call lcd_data
	lsr r0
	lsr r0
	lsr r0
	lsr r0
	lsr r0
	breq zero
	dec r0
	breq one
	dec r0
	breq two
	dec r0
	breq three
	dec r0
	breq four
	dec r0
	breq five
	dec r0
	breq six
	dec r0
	rjmp seven

zero: 
	ldi r24, 0x30
	call lcd_data
	ldi r24, 0x30
	call lcd_data
	rjmp exit
	
one: 
	ldi r24, 0x31
	call lcd_data
	ldi r24, 0x32
	call lcd_data
	rjmp exit
	
two: 
	ldi r24, 0x32
	call lcd_data
	ldi r24, 0x35
	call lcd_data
	rjmp exit
	
three: 
	ldi r24, 0x33
	call lcd_data
	ldi r24, 0x37
	call lcd_data
	rjmp exit
	
extrem:
	ldi r24, 0x34
	call lcd_data
	ldi r24, 0x2C
	call lcd_data
	ldi r24, 0x39
	call lcd_data
	ldi r24, 0x39
	call lcd_data
	rjmp exit
	
four: 
	ldi r24, 0x35
	call lcd_data
	ldi r24, 0x30
	call lcd_data
	rjmp exit
	
five: 
	ldi r24, 0x36
	call lcd_data
	ldi r24, 0x32
	call lcd_data
	rjmp exit
	
six: 
	ldi r24, 0x37
	call lcd_data
	ldi r24, 0x35
	call lcd_data
	rjmp exit
	
seven: 
	ldi r24, 0x38
	call lcd_data
	ldi r24, 0x37
	call lcd_data
	rjmp exit
	



	
exit:	
	ldi r24, 0x56
	call lcd_data				; Output Volts
	ldi r24, low(1000)
    ldi r25, high(1000)
    rcall wait_msec 			; delay 1 Sec 
	
reti


write_2_nibbles:
    push r24        		; save r24(LCD_Data)     
    in r25 ,PIND      		; read PIND   
    andi r25 ,0x0f 
    andi r24 ,0xf0      	; r24[3:0] Holds previus PORTD[3:0] 
    add r24 ,r25 			; r24[7:4] <-- LCD_Data_High_Byte 
    out PORTD ,r24  
    sbi PORTD ,PD3 			; Enable  Pulse 
    nop
    nop
    cbi PORTD ,PD3
    
    pop r24   				; Recover r24(LCD_Data) 
    swap r24 
    andi r24 ,0xf0 			; r24[3:0] Holds previus PORTD[3:0]   
    add r24 ,r25  			; r24[7:4] <-- LCD_Data_Low_Byte 
    out PORTD ,r24
    
    sbi PORTD ,PD3 			; Enable  Pulse 
    nop
    nop
    cbi PORTD ,PD3
    
    ret



lcd_data:
    sbi PORTD ,PD2 					; LCD_RS=1(PD2=1), Data 
    rcall write_2_nibbles     		; send data 
    ldi r24 ,250                  
    ldi r25 ,0                    	; Wait 250uSec 
    rcall wait_usec
    ret 
	
	
lcd_command:
    cbi PORTD ,PD2         			; LCD_RS=0(PD2=0), Instruction  
    rcall write_2_nibbles    		; send Instruction 
    ldi r24 ,250                  
    ldi r25 ,0                   	; Wait 250uSec 
    rcall wait_usec
    ret



lcd_clear_display:
    
    ldi r24 ,0x01           ; clear display command 
    rcall lcd_command
    
    ldi r24 ,low(5)  
    ldi r25 ,high(5)  		; Wait 5 mSec 
    rcall wait_msec      
    ret



lcd_init:

    ldi r24 ,low(200)  
    ldi r25 ,high(200)  			; Wait 200 mSec 
    rcall wait_msec    
    ldi r24 ,0x30  					; command to switch to 8 bit mode 
    out PORTD ,r24 
    sbi PORTD ,PD3 					; Enable  Pulse 
    nop
    nop
    cbi PORTD ,PD3
    ldi r24 ,250  
    ldi r25 ,0 						; Wait 250uSec  
    rcall wait_usec           
    ldi r24 ,0x30  					; command to switch to 8 bit mode 
    out PORTD ,r24 
    sbi PORTD ,PD3 					; Enable  Pulse  
    nop
    nop
    cbi PORTD ,PD3
    ldi r24 ,250  
    ldi r25 ,0 						; Wait 250uSec  
    rcall wait_usec            
    ldi r24 ,0x30  					; command to switch to 8 bit mode 
    out PORTD ,r24 
    sbi PORTD ,PD3 					; Enable  Pulse  
    nop
    nop
    cbi PORTD ,PD3
    ldi r24 ,250                  
    ldi r25 ,0                    	; Wait 250uSec 
    rcall wait_usec
    ldi r24 ,0x20                	; command to switch to 4 bit mode 
    out PORTD ,r24
    sbi PORTD ,PD3 					; Enable  Pulse 
    nop
    nop
    cbi PORTD ,PD3
    ldi r24 ,250                  
    ldi r25 ,0                    	; Wait 250uSec 
    rcall wait_usec    
    ldi r24 ,0x28              		;  5x8 dots, 2 lines 
    rcall lcd_command
    ldi r24 ,0x0c                	; dislay on, cursor off 
    rcall lcd_command      
	rcall lcd_clear_display 
 
    ldi r24 ,0x06                	; Increase address, no display shift 
    rcall lcd_command           
    ret 



wait_msec:

    push r24 					; 2 cycles 
    push r25 					; 2 cycles 
    ldi r24 , low(999) 			; 1 cycle 
    ldi r25 , high(999)			; 1 cycle 
    rcall wait_usec 			; 998.375 usec   
    pop r25 					; 2 cycles 
    pop r24 					; 2 cycles 
    nop  						; 1 cycle 
    nop  						; 1 cycle 
    sbiw r24 , 1 				; 2 cycles  
    brne wait_msec 				; 1 or 2 cycles 

    ret  						; 4 cycles 
    



wait_usec:
    sbiw r24 ,1  				; 2 cycles (2/16 usec) 
    call delay_8cycles 			; 4+8=12 cycles
    brne wait_usec 				; 1 or 2 cycles 
    ret
	
delay_8cycles:
	nop
	nop
	nop
	nop
	ret