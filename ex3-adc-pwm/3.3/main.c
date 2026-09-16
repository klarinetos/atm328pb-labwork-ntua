#define F_CPU 16000000UL
#include<avr/io.h>
#include<util/delay.h>


unsigned char str[] = {5,25,46,66,87,107,127,148,169,189,210,230,251};

int main(){

TCCR1A = (1<<WGM10)|(1<<COM1A1);	// Non-inverting Mode, Fast PWM, 8-bit Mode
TCCR1B = (1<<WGM12)|(1<<CS10);		// Prescaler is 1 (frequency = f.clk)
ADMUX = 0b01100000;					// 5V ref, left-adjusted, ADC0
ADCSRA = 0b10000110;				// Prescaler is 64
	
DDRB=0b111111;      				// Set PORTB as output
DDRD=0x00;							// Set PORTD as input

unsigned int DC_VALUE = 6;
int mode=1;
unsigned char z,y;

OCR1AL = str[DC_VALUE];				// Initial dc 50%



while(1){

z = PIND;
z &=0b01000000 ;						// PD6
if(!z){_delay_ms(50);
mode = 1;}

z = PIND;
z &=0b10000000 ;						// PD7
if(!z){_delay_ms(50);
mode = 2;}


if(mode==1){							// Mode 1
OCR1AL = str[DC_VALUE];
z = PIND;
z&=0b00000010;
_delay_ms(60);
if((!(z))&&(DC_VALUE<12)){				// If PD1 is pressed
	 DC_VALUE++ ;						// Increase dc by 8%
	_delay_ms(50);
}

z = PIND;
z&=0b00000100;
if((!(z))&&(DC_VALUE>0)){				// If PD2 is pressed
	DC_VALUE--;							// Decrease dc by 8%
	_delay_ms(50);
}

OCR1AL = str[DC_VALUE];
}							

else if(mode==2){ 						// Mode 2

ADCSRA |= (1<<ADSC);
z = ADCSRA;
z &= 0b01000000;
while(z){
z = ADCSRA;
z &= 0b01000000;}						// Wait until ADSC is cleared

y = ADCH;
OCR1AL = y;
_delay_ms(20);
}

}

return 0;
}