#define F_CPU 16000000UL
#include<avr/io.h>
#include<util/delay.h>


void write_2_nibbles(unsigned char data){

unsigned char previous, next;
previous = PIND;
previous &= 0x0F;
next = previous;
previous += (data & 0xF0);		// data_high_byte
PORTD = previous;
enable_pulse();
data &= 0x0F ;					// data_low_byte
data = data << 4 ;
next += data;
PORTD = next;
enable_pulse();

}

void lcd_data(unsigned char data){

PORTD |= 0b00000100;			// LCD_RS=1(PD2=1), Data 
write_2_nibbles(data);
_delay_us(250);


}

void lcd_command(unsigned char command){

PORTD &= 0b11111011;				//LCD_RS=0(PD2=0), Instruction
write_2_nibbles(command);
_delay_us(250);

return;}

void lcd_clear_display(){

lcd_command(0x01);
_delay_ms(5);


}

void lcd_init(){

_delay_ms(200);
unsigned char temp = 0x30;
PORTD = temp;
enable_pulse();
for(int i=2; i>0; i--){
_delay_us(250);
PORTD = temp;					// command to switch to 8 bit mode 
enable_pulse();}
_delay_us(250);
temp = 0x20;
PORTD = temp;					// command to switch to 4 bit mode 
enable_pulse();
_delay_us(250);
lcd_command(0x28);				// 5x8 dots, 2 lines
lcd_command(0x0C);				// dislay on, cursor off
lcd_clear_display();
lcd_command(0x06);				// Increase address, no display shift


}


void enable_pulse(){
PORTD |= 0b00001000;			// Enable Pulse
_delay_us(1);
PORTD &= 0b11110111;			// Disable Pulse
}

int main(){

TCCR1A = (1<<WGM10)|(1<<COM1A1);	// Non-inverting Mode, Fast PWM, 8-bit Mode
TCCR1B = (1<<WGM12)|(1<<CS11);		// Prescaler is 8 (frequency = f.clk/8)	
ADMUX = 0b01100010;					// 5V ref, left-adjusted, ADC02
ADCSRA = 0b10000001;				// Prescale is 2

DDRD=0xFF;							// Set PORTD as output
unsigned char z;
double y;
lcd_init();
_delay_ms(100);
lcd_clear_display();
_delay_ms(1000);
unsigned int i=0;
unsigned int t;

while(1)
{
ADCSRA |= (1<<ADSC);
z = ADCSRA;
z &= 0b01000000;
while(z){
z = ADCSRA;
z &= 0b01000000;
}

lcd_clear_display();
z = ADCH;
y = (double) z;
y = (y*5)/256;
for(i = 0; i<500; i++){
if(y<0.01*(i+1)){
break;
}
}
t = i/100;
z = (unsigned char) t;				// Integer part
z += 0x30;
lcd_data(z);
lcd_data(0x2C);
i = i%100;
t = i/10;
z = (unsigned char) t;				// First decimal point
z += 0x30;
lcd_data(z);
i = i%10;
t = i;
z = (unsigned char) t;				// Second decimal point
z += 0x30;
lcd_data(z);

_delay_ms(1000);
}



return 0;
}