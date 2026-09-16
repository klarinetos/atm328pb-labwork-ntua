#define F_CPU 16000000UL
#include<avr/io.h>
#include<util/delay.h>
#include<avr/interrupt.h>


uint8_t one_wire_reset(){
	DDRD |= (1 << PD4);					// Set PD4 as output
	PORTD &= ~(1 << PD4);				
	_delay_us(480);						// 480us reset pulse
	
	DDRD &= ~(1 << PD4);				// Set PD4 as input
	PORTD &= ~(1 << PD4);				// Disable pull-up
	_delay_us(100);						// Wait 100us for devices to transmit the presence pulse
	
uint8_t input;
	input = PIND;						// Read PORTD
	_delay_us(380);
	input = input & (1 << PD4) ;		// Check bit PD4
	if(input) return 0;					// If PD4=1 no device detected, return 0
	return 1;							// Else return 1
}


uint8_t one_wire_receive_bit(){
	DDRD |= (1 << PD4);					// Set PD4 as output
	PORTD &= ~(1 << PD4);				// Set PD4 LOW
	_delay_us(2);						// Wait 2us
	
	DDRD &= ~(1 << PD4);				// Set PD4 as input
	PORTD &= ~(1 << PD4);				// Disable pull-up
	_delay_us(10);						// Wait 10us
	
uint8_t input;
	input = PIND;						// Read PORTD
	input = input & (1 << PD4) ;		// Check bit PD4
	_delay_us(49);						// Wait 49us
	
	if(input) return 1;					// If PD4=1 return 1
	return 0;							// Else return 0
}


void one_wire_transmit_bit(uint8_t bit){
	DDRD |= (1 << PD4);					// Set PD4 as output
	PORTD &= ~(1 << PD4);				// Set PD4 LOW
	_delay_us(2);						// Wait 2us
	
	if(bit%2) {PORTD |= (1 << PD4);}	// If bit[0]=1 output 1
	else {PORTD &= ~(1 << PD4);}		// Else output 0
	_delay_us(58);						// Wait 58us
	
	DDRD &= ~(1 << PD4);				// Set PD4 as input
	PORTD &= ~(1 << PD4);				// Disable pull-up
	_delay_us(1);						// Wait 1us	
}


uint8_t one_wire_receive_byte(){
	
uint8_t result = 0;

for(int i = 0; i < 8; i++){
	result = result >> 1;
	if(one_wire_receive_bit()) result += 0x80;
}
return result;	
}


void one_wire_transmit_byte(uint8_t byte){
for(int i = 0; i < 8; i++){
	one_wire_transmit_bit(byte);
	byte = byte >> 1;
}	
}


uint16_t temperature(){
	
	
}
















