#define F_CPU 16000000UL
#include<avr/io.h>
#include<util/delay.h>
#include<avr/interrupt.h>
#include<stdio.h>


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

#define FOSC 16000000 // Clock Speed
#define BAUD 9600
#define MYUBRR FOSC/16/BAUD-1

#define PCA9555_0_ADDRESS 0x40       			//A0=A1=A2=0 by hardware 
#define TWI_READ    1    						// reading from twi device 
#define TWI_WRITE   0    						// writing to twi device 
#define SCL_CLOCK  100000L  					// twi clock in Hz 

//Fscl=Fcpu/(16+2*TWBR0_VALUE*PRESCALER_VALUE)
#define TWBR0_VALUE ((F_CPU/SCL_CLOCK)-16)/2

//----------- Master Transmitter/Receiver -------------------
#define TW_START         0x08 
#define TW_REP_START 	 0x10 

//---------------- Master Transmitter ----------------------
#define TW_MT_SLA_ACK 	 0x18 
#define TW_MT_SLA_NACK   0x20 
#define TW_MT_DATA_ACK   0x28 

//---------------- Master Receiver ----------------
#define TW_MR_SLA_ACK  	 0x40 
#define TW_MR_SLA_NACK   0x48 
#define TW_MR_DATA_NACK  0x58 

#define TW_STATUS_MASK  0b11111000
#define TW_STATUS (TWSR0 & TW_STATUS_MASK)


// PCA9555 REGISTERS
typedef enum {
  REG_INPUT_0                = 0, 
  REG_INPUT_1                = 1, 
  REG_OUTPUT_0               = 2,
  REG_OUTPUT_1               = 3,
  REG_POLARITY_INV_0         = 4,
  REG_POLARITY_INV_1         = 5,
  REG_CONFIGURATION_0    	 = 6,
  REG_CONFIGURATION_1        = 7
} PCA9555_REGISTERS;


//initialize TWI clock
void twi_init()
{
  TWSR0 = 0;                        // PRESCALER_VALUE=1
  TWBR0 = TWBR0_VALUE;              // SCL_CLOCK  100KHz
}

// Read one byte from the twi device (request more data from device)
unsigned char twi_readAck()
{ 
TWCR0 = (1<<TWINT) | (1<<TWEN) | (1<<TWEA);  
while(!(TWCR0 & (1<<TWINT)));     
     
return TWDR0; 
}

//Read one byte from the twi device, read is followed by a stop condition 
unsigned char twi_readNak()
{ 
TWCR0 = (1<<TWINT) | (1<<TWEN); 
while(!(TWCR0 & (1<<TWINT))); 
     
return TWDR0; 
}

// Issues a start condition and sends address and transfer direction.
// return 0 = device accessible, 1 = failed to access device
unsigned char twi_start(unsigned char address)
{
   uint8_t   twi_status;

// send START condition 
TWCR0 = (1<<TWINT) | (1<<TWSTA) | (1<<TWEN); 

// wait until transmission completed 
while(!(TWCR0 & (1<<TWINT))); 
 
// check value of TWI Status Register. 
twi_status = TW_STATUS & 0xF8; 
if ((twi_status != TW_START) && (twi_status != TW_REP_START)) return 1; 
 
// send device address 
TWDR0 = address; 
TWCR0 = (1<<TWINT) | (1<<TWEN); 
 
// wail until transmission completed and ACK/NACK has been received 
while(!(TWCR0 & (1<<TWINT)));

// check value of TWI Status Register. 
twi_status = TW_STATUS & 0xF8; 
if ( (twi_status != TW_MT_SLA_ACK) && (twi_status != TW_MR_SLA_ACK) ) 
    { return 1; }

return 0; 
}

// Send start condition, address, transfer direction.
// Use ack polling to wait until device is ready
void twi_start_wait(unsigned char address)
{
    uint8_t   twi_status;
  while(1){
  
// send START condition
TWCR0 = (1<<TWINT) | (1<<TWSTA) | (1<<TWEN); 
       
// wait until transmission completed 
while(!(TWCR0 & (1<<TWINT)));
        
// check value of TWI Status Register. 
twi_status = TW_STATUS & 0xF8;     
if ( (twi_status != TW_START) && (twi_status != TW_REP_START)){ continue; }
      
// send device address 
TWDR0 = address;
TWCR0 = (1<<TWINT) | (1<<TWEN);
        
// wail until transmission completed 
while(!(TWCR0 & (1<<TWINT)));
        
// check value of TWI Status Register.   
twi_status = TW_STATUS & 0xF8;    
if ( (twi_status == TW_MT_SLA_NACK )||(twi_status ==TW_MR_DATA_NACK) )      
{      
     /* device busy, send stop condition to terminate write operation */ 
TWCR0 = (1<<TWINT) | (1<<TWEN) | (1<<TWSTO);  

// wait until stop condition is executed and bus released 
while(TWCR0 & (1<<TWSTO)); 

continue; 
     
} 
break; 
     }
}


// Send one byte to twi device, Return 0 if write successful or 1 if write failed
unsigned char twi_write(unsigned char data)
{   
// send data to the previously addressed device 
TWDR0 = data;
TWCR0 = (1<<TWINT) | (1<<TWEN); 
 
// wait until transmission completed 
while(!(TWCR0 & (1<<TWINT))); 
 
if( (TW_STATUS & 0xF8) != TW_MT_DATA_ACK) return 1; 
 
return 0; 
}

// Send repeated start condition, address, transfer direction 
//Return: 0 device accessible
//        1 failed to access device
unsigned char twi_rep_start(unsigned char address)
{
    return twi_start(address);
}


// Terminates the data transfer and releases the twi bus
void twi_stop()
{
// send stop condition 
TWCR0 = (1<<TWINT) | (1<<TWEN) | (1<<TWSTO); 
 
// wait until stop condition is executed and bus released 
while(TWCR0 & (1<<TWSTO)); 
}

void PCA9555_0_write(PCA9555_REGISTERS reg, uint8_t value)
{
      twi_start_wait(PCA9555_0_ADDRESS + TWI_WRITE);
      twi_write(reg);
      twi_write(value);
      twi_stop();
}

uint8_t PCA9555_0_read(PCA9555_REGISTERS reg)
{
uint8_t ret_val;
       
      twi_start_wait(PCA9555_0_ADDRESS + TWI_WRITE); 
      twi_write(reg);
      twi_rep_start(PCA9555_0_ADDRESS + TWI_READ);
      ret_val = twi_readNak();
      twi_stop(); 
      
      return ret_val;
}

void enable_pulse(){
	uint8_t state;
	state = PCA9555_0_read(REG_INPUT_0);
	state |= 0b00001000;
	PCA9555_0_write(REG_OUTPUT_0, state);			// Enable Pulse
	_delay_us(1);
	state &= 0b11110111;
	PCA9555_0_write(REG_OUTPUT_0, state);			// Disable Pulse
}


void write_2_nibbles(unsigned char data){		// Presumes PEX0 is cofigured as output

unsigned char previous, next;
previous = PCA9555_0_read(REG_INPUT_0);
previous &= 0x0F;
next = previous;
previous += (data & 0xF0);		// data_high_byte
PCA9555_0_write(REG_OUTPUT_0, previous);
enable_pulse();
data &= 0x0F ;					// data_low_byte
data = data << 4 ;
next += data;
PCA9555_0_write(REG_OUTPUT_0, next);
enable_pulse(); 
}

void lcd_data(unsigned char data){				// Presumes PEX0 is cofigured as output
uint8_t state;
state = PCA9555_0_read(REG_INPUT_0);
state |= 0b00000100; 							// LCD_RS=1(PD2=1), Data 
PCA9555_0_write(REG_OUTPUT_0, state);
write_2_nibbles(data);
_delay_us(250);
}

void lcd_command(unsigned char command){		// Presumes PEX0 is cofigured as output
uint8_t state;
state = PCA9555_0_read(REG_INPUT_0);
state &= 0b11111011; 							//LCD_RS=0(PD2=0), Instruction
PCA9555_0_write(REG_OUTPUT_0, state);
write_2_nibbles(command);
_delay_us(250);
}

void lcd_clear_display(){

lcd_command(0x01);
_delay_ms(5);

return;}

void lcd_init(){
_delay_ms(200);
PCA9555_0_write(REG_CONFIGURATION_0, 0x00); 			// Set EXT_PORT0 as output
_delay_ms(200);
unsigned char temp = 0x30;
PCA9555_0_write(REG_OUTPUT_0, temp);
enable_pulse();

for(int i=2; i>0; i--){
_delay_us(250);
PCA9555_0_write(REG_OUTPUT_0, temp);			// command to switch to 8 bit mode 
enable_pulse(); }

_delay_us(250);
temp = 0x20;
PCA9555_0_write(REG_OUTPUT_0, temp);			// command to switch to 4 bit mode 				
enable_pulse();
_delay_us(250);
lcd_command(0x28);				// 5x8 dots, 2 lines
lcd_command(0x0C);				// dislay on, cursor off
lcd_clear_display();
lcd_command(0x06);				// Increase address, no display shift
}

void lcd_string(unsigned char *str){
//lcd_clear_display();
int i=0;
while(str[i]){
lcd_data(str[i]);
i++;
}
}

void USART_Init( unsigned int ubrr)
{
UCSR0A=0;
/*Set_baud_rate*/
UBRR0H = (unsigned char)(ubrr>>8);
UBRR0L = (unsigned char)ubrr;
//Enable receiver and transmitter 
UCSR0B = (1<<RXEN0)|(1<<TXEN0);
/*2stop_bit*/
UCSR0C = (3<<UCSZ00);

}

void usart_transmit(uint8_t data)
{
while(!(UCSR0A&(1<<UDRE0)));
UDR0=data;
}

uint8_t usart_receive(){
while(!(UCSR0A&(1<<RXC0)));
return UDR0;}

void usart_transmit_str(const unsigned char* str){
	uint16_t i = 0;
	while(str[i]){
		usart_transmit(str[i++]);
	}
	usart_transmit('\n');
}

void usart_receive_str(){
	unsigned char result[50];
	unsigned char i=0;
	do{
		result[i++]=usart_receive();
        	}
	while(result[i-1]!='\n');
	result[i-1]='\0';							// Null
	lcd_string(result);
    _delay_ms(1000);
	return result;
}


void usart_command(const unsigned char* message){				// Sends data and returns the answer
	usart_transmit_str(message);
	usart_receive_str();
}

void usart_send_payload(int temper, uint8_t adch,unsigned char *stat){					// temper is the temperature*10, adch is the high register for the pressure
unsigned char buffer[160];

int i1=temper/100;			// decades
temper -= 100*i1;			
int i2 = temper/10;			// units
int i3 = temper%10;			// 1st decimal point

double pres = (double) adch;
pres = (pres*20)/255;									// Pressure from 0-20 cm H20
unsigned int p = (int) (pres*10);
int p1=p/100;			// decades
p -= 100*p1;			
int p2 = p/10;			// units
int p3 = p%10;			// 1st decimal point


sprintf(buffer,"[{\"name\":\"temperature\",\"value\":\"%d%d.%d\"},{\"name\":\"pressure\",\"value\":\"%d%d.%d\"},{\"name\":\"team\",\"value\":\"59\"},{\"name\":\"status\",\"value\":\"%s\"}]", i1,i2,i3,p1,p2,p3,stat);
usart_transmit_str(buffer);
}

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


int therm_read_temperature(){			
	
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
double result = (double) temp;
int ret;
result = result/16;
result *= 10;
													//one decimal point accuracy
return (int) result;								// return temperature*10
}


uint8_t scan_row(uint8_t row){
	
uint8_t temp = 0x01;
	temp = temp << (row - 1); 							// 
	PCA9555_0_write(REG_OUTPUT_1, ~temp);				// output zero to bit IO1_row
	temp = PCA9555_0_read(REG_INPUT_1);
	temp = ~temp;
	temp = temp >> 4;
	return temp;										// return the state of the pressed buttons
	
}


uint16_t scan_keypad(){

uint16_t result = 0x00;
	for(uint8_t i = 1; i < 5; i++){
		result = result << 4;
		result += scan_row(i);
		}
	return result;
}

// Returns the buttons that are pressed for the first time
uint16_t scan_keypad_rising_edge(){
	static uint16_t pressed = 0x00;
	uint16_t bounce, temp;
	
	bounce = scan_keypad();
	_delay_ms(15);
	temp = scan_keypad();
	bounce = (bounce ^ temp);								// 
	bounce = ~bounce;										//
	bounce = bounce & temp;									// Keep only the bits that haven't changed for debouncing
	
	temp = pressed ^ bounce;
	temp = temp & bounce;									// Keep only the bits that changed from 0 -> 1 (were pressed now)
	pressed = bounce;										// Update current state of pressed buttons
	
	return temp;											
}

// Returns the buttons that are still pressed 
uint16_t scan_keypad_pressed(){
	uint16_t bounce;
	uint16_t temp;
	
	bounce = scan_keypad();
	_delay_ms(15);
	temp = scan_keypad();
	bounce = (bounce ^ temp);								// 
	bounce = ~bounce;										//
	bounce = bounce & temp;									// Keep only the bits that haven't changed for debouncing
											
	return bounce;
}


uint8_t keypad_to_ascii(){
	const uint8_t codes[] = {'1', '2', '3', 'A', '4', '5', '6', 'B', '7', '8', '9', 'C', '*', '0', '#', 'D'};
	uint8_t counter = 0;
	uint16_t keypad = scan_keypad_rising_edge();
	//uint16_t keypad = scan_keypad_rising_edge();
	while(counter<16){
		if(keypad%2){return codes[counter];}
		keypad = keypad >> 1;
		counter++;
	}
	return 0;
}


int main() {
	
    twi_init();
	lcd_init();
	therm_reset();
	USART_Init(MYUBRR);
	PCA9555_0_write(REG_CONFIGURATION_1, 0xF0); 			// Set EXT_PORT1_0-3 as output and EXT_PORT1_4-7 as input
	
	ADMUX = 0b01100000;					// 5V ref, left-adjusted, ADC0
	ADCSRA = 0b10000110;				// Prescaler is 64
	int tempr;
	unsigned char statuses[3][20]={"OK","CHECKTEMP","CHECKPRESSURE", "NURSECALL"};
	unsigned char *status=statuses[0];
	uint8_t z,y,key_pressed=0;
	
while(1){	
	status=statuses[0];
	
//usart_command("ESP:restart");
//_delay_ms(2000);
lcd_clear_display();
lcd_data('1');
lcd_data('.');
usart_command("ESP:connect");
_delay_ms(2000);
lcd_clear_display();
lcd_data('2');
lcd_data('.');
usart_command("ESP:url:\"http://192.168.1.250:5000/data\"");
_delay_ms(2000);

tempr = therm_read_temperature();		// returns temperature*10
	tempr = therm_read_temperature();
	tempr += 120; 							// Add 13 deegres C
	
	if((tempr<340)|(tempr>370)){
		status = statuses[1];
		}
	
	
ADCSRA |= (1<<ADSC);
z = ADCSRA;
z &= 0b01000000;
while(z){
z = ADCSRA;
z &= 0b01000000;}						// Wait until ADSC is cleared

y = ADCH;

	if((y<51)|(y>153)){
		status=statuses[2];
		}

unsigned char buffer[160];
int i1=tempr/100;			// decades
tempr -= 100*i1;			
int i2 = tempr/10;			// units
int i3 = tempr%10;			// 1st decimal point

double pres = (double) y;
pres = (pres*20)/255;									// Pressure from 0-20 cm H20
unsigned int p = (int) (pres*10);
int p1=p/100;			// decades
p -= 100*p1;			
int p2 = p/10;			// units
int p3 = p%10;			// 1st decimal point
sprintf(buffer,"ESP:payload:[{\"name\":\"temperature\",\"value\":\"%d%d.%d\"},{\"name\":\"pressure\",\"value\":\"%d%d.%d\"},{\"name\":\"team\",\"value\":\"59\"},{\"name\":\"status\",\"value\":\"%s\"}]", i1,i2,i3,p1,p2,p3,status);

lcd_clear_display();
lcd_data('3');
lcd_data('.');
usart_command(buffer);
_delay_ms(2000);

lcd_clear_display();
lcd_data('4');
lcd_data('.');
usart_command("ESP:transmit");

_delay_ms(2000);
}
    return 0;
}

