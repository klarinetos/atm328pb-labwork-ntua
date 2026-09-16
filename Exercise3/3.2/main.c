#define F_CPU 16000000UL
#include<avr/io.h>
#include<util/delay.h>


unsigned char str[] = {5,25,46,66,87,107,127,148,169,189,210,230,251};

int main(){

TCCR1A = (1<<WGM10)|(1<<COM1A1);	// Non-inverting Mode, Fast PWM, 8-bit Mode
TCCR1B = (1<<WGM12)|(1<<CS11);		// Prescaler is 8 (frequency = f.clk/8)	
ADMUX = 0b01100001;					// 5V ref, left-adjusted, ADC01
ADCSRA = 0b10000110;				// Prescale is 64
	
	
DDRB=0b001111;      				// Set PB1 as output, PB4,PB5 as input
DDRD=0xFF;							// Set PORTD as output

unsigned int x = 6;
unsigned char z,y;
OCR1AL = str[x];					// Initial dc 50%



while(1){
z = PINB&0b010000;
if((!(z))&&(x<12)){					// If PB4 is pressed
	 x++ ;							// Increase dc by 8%
	_delay_ms(50);
}
z = PINB&0b100000;
if((!(z))&&(x>0)){					// If PB5 is pressed
	x--;							// Decrease dc by 8%
	_delay_ms(50);
}

OCR1AL = str[x];

ADCSRA |= (1<<ADSC);
z = ADCSRA;
z &= 0b01000000;
while(z){
z = ADCSRA;
z &= 0b01000000;}					// Check until ADC is ready

y = ADCH;

if(y<32){z=0x01;}
else if(y<64){z=0x02;}
else if(y<96){z=0x04;}
else if(y<128){z=0x08;}
else if(y<160){z=0x10;}
else if(y<192){z=0x20;}
else if(y<224){z=0x40;}
else{z=0x80;}

PORTD = z;							// Output result
_delay_ms(100);

}
return 0;
}