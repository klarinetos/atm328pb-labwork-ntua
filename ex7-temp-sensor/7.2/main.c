#define F_CPU 16000000UL
#include<avr/io.h>
#include<util/delay.h>
#include<avr/interrupt.h>

#define THERM_PORT PORTD
#define THERM_DDR DDRD
#define THERM_PIN PIND
#define THERM_DQ PD4
#define THERM_INPUT_MODE() THERM_DDR&=~(1<<THERM_DQ)
#define THERM_OUTPUT_MODE() THERM_DDR|=(1<<THERM_DQ)
#define THERM_LOW() THERM_PORT&=~(1<<THERM_DQ)
#define THERM_HIGH() THERM_PORT|=(1<<THERM_DQ)


#define THERM_CMD_CONVERTTEMP 0x44
#define THERM_CMD_SKIPROM 0xcc
#define THERM_CMD_RSCRATCHPAD 0xbe
#define THERM_DECIMAL_STEPS_12BIT 625

uint8_t therm_reset(){
uint8_t i;
THERM_OUTPUT_MODE();
THERM_LOW();
_delay_us(480);
THERM_INPUT_MODE();
THERM_LOW();
_delay_us(100);
i=(THERM_PIN & (1<<THERM_DQ));
_delay_us(380);
//Return the value read from the presence pulse (0=OK, 1=WRONG)
if(i) return 0;					// No device connected
return 1;
}

void therm_write_bit(uint8_t bit){
THERM_OUTPUT_MODE();
THERM_LOW();
_delay_us(2);
if(bit) THERM_HIGH();
else THERM_LOW();
_delay_us(58);
THERM_INPUT_MODE();
THERM_LOW();
_delay_us(1);
}

uint8_t therm_read_bit(void){
uint8_t bit=0;
THERM_OUTPUT_MODE();
THERM_LOW();
_delay_us(2);
THERM_INPUT_MODE();
THERM_LOW();
_delay_us(10);
if(THERM_PIN&(1<<THERM_DQ)) bit=1;
_delay_us(49);
return bit;
}

uint8_t therm_read_byte(void){
uint8_t i=8, n=0;
while(i--){
n>>=1;
n|=(therm_read_bit()<<7);
}
return n;
}


void therm_write_byte(uint8_t byte){
uint8_t i=8;
while(i--){
therm_write_bit(byte&1);
byte>>=1;
}
}


uint16_t therm_read_temperature(){
uint8_t temperature[2];
uint16_t temp=0;
if(!therm_reset()) return 0x8000;
therm_write_byte(THERM_CMD_SKIPROM);
therm_write_byte(THERM_CMD_CONVERTTEMP);
while(!therm_read_bit());
therm_reset();
therm_write_byte(THERM_CMD_SKIPROM);
therm_write_byte(THERM_CMD_RSCRATCHPAD);
temperature[0]=therm_read_byte();
temperature[1]=therm_read_byte();
temp += temperature[1];
temp = temp << 8;
temp += temperature[0];
return temp;
}

uint8_t temp_integer(){
	uint16_t temp = therm_read_temperature();
	temp = temp >> 4;
	temp = temp & 0x3F;
	uint8_t result=0;
	result += temp;
	return result;	
}


int main() {
    DDRB = 0xFF;
	while(1){
		PORTB = temp_integer();
		_delay_us(1000);
	}
    return 0;
}


