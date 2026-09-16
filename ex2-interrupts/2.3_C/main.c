#define F_CPU 16000000UL
#include<avr/io.h>
#include<util/delay.h>
#include<util/atomic.h>
#include<avr/interrupt.h>


int main()
{
    DDRB=0xFF;      		// Set PORTB as output
	EIMSK = (1<<INT1);		// Enable INT1
	EICRA = (1<<ISC11)|(0<<ISC10);	// Interrupt is sensed at the falling edge
	sei();	// Enable all interrupts
	PORTB = 0x00;	// Initially turn off all LEDs
	
	while(1)
	{
	
    }}



int DELAY;

//interrupt[EXT_INT1]
 ISR(INT1_vect){
	DELAY = 50;
	
	PORTB = 0xFF;			// Turn on all the LEDs
	_delay_ms(500);			// for 0.5s
	PORTB = 0x01;			// Keep only PB0 LED on
	sei();				// Enable all interrupts
	while(DELAY>0){
	_delay_ms(50);
	DELAY--;
	}
    reti();
}