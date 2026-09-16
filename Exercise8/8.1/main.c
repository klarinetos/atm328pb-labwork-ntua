#define F_CPU 16000000UL
#include<avr/io.h>
#include<util/delay.h>
#include<avr/interrupt.h>

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
	unsigned char result[20];
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


int main(){
	
twi_init();
PCA9555_0_write(REG_CONFIGURATION_0, 0x00); 			// Set EXT_PORT0 as output
lcd_init();
USART_Init(MYUBRR);										// Initializations

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
lcd_clear_display();
lcd_data('3');
lcd_data('.');
usart_command("ESP:payload:[{\"name\":\"team\",\"value\":\"5\"}]");
_delay_ms(2000);
lcd_clear_display();
lcd_data('4');
lcd_data('.');
usart_command("ESP:transmit");


return 0;
}