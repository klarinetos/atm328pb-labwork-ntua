#define F_CPU 16000000UL
#include<avr/io.h>
#include<util/delay.h>
#include<avr/interrupt.h>

void enable_pulse(void);

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

}

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


void lcd_string(unsigned char *str){
lcd_clear_display();
int i=0;
while(str[i]){
lcd_data(str[i]);
i++;
}
}


unsigned char z;
int mode = 0;


int main(){
	
ADMUX = 0b01100010;					// 5V ref, left-adjusted, ADC02
ADCSRA = 0b10001001;				// Prescale is 2, enable ADC interrupt

DDRB=0xFF;							// Set PORTB as output
DDRD=0xFF;							// Set PORTD as output
_delay_ms(100);
lcd_init();
_delay_ms(100);
lcd_clear_display();
_delay_ms(100);


ADCSRA |= (1<<ADSC);
z = ADCSRA;
z &= 0b01000000;
while(z){
z = ADCSRA;
z &= 0b01000000;
}
z = ADCH;							// initialize from 1st conversion
if(z<51){
mode=0;
}
else mode = 1;						
unsigned char change = 1;			// indicate the change of status

while(1){

if(mode == 0){					// If level < 1V

if(change==1){
PORTB = 0b000001;	
char clr[] = "CLEAR";				
lcd_string(clr);}

}					

else {								// Level is > 1V 
if(z<92){							// Level is 1-1.8V 
PORTB = 0b000011;
_delay_ms(300);
PORTB = 0x00;
_delay_ms(300);
}
else if(z<133){						// Level is 1.8-2.6V
PORTB = 0b000111;
_delay_ms(300);
PORTB = 0x00;
_delay_ms(300);
}					
else if(z<174){ 					// Level is 2.6-3.4V
PORTB = 0b001111;
_delay_ms(300);
PORTB = 0x00;
_delay_ms(300);
}
else if(z<215){ 					// Level is 3.4-4.2V
PORTB = 0b011111;
_delay_ms(300);
PORTB = 0x00;
_delay_ms(300);
}
else{								// Level is 4.2-5V
PORTB = 0b111111;
_delay_ms(300);
PORTB = 0x00;
_delay_ms(300);
}

if(change==1) {
char gas[] = "GAS DETECTED";				
lcd_string(gas);
}

}

ADCSRA |= (1<<ADSC);
z = ADCSRA;
z &= 0b01000000;
while(z){
z = ADCSRA;
z &= 0b01000000;
}
z = ADCH;
if(z<51){
if(mode==0){change=0;}
else {mode=0;
change=1;}
}
else {
if(mode==0) {
mode = 1;
change=1;}
else change = 0;}

}



return 0;
}

