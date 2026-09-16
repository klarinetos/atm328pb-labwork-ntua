#define F_CPU 16000000UL

#include<avr/io.h>
#include<util/delay.h>
#include<avr/interrupt.h>


#define PCA9555_0_ADDRESS 0x40       			// A0=A1=A2=0 by hardware 
#define TWI_READ    1    						// reading from twi device 
#define TWI_WRITE   0    						// writing to twi device 
#define SCL_CLOCK  100000L  					// twi clock in Hz 

//Fscl=Fcpu/(16+2*TWBR0_VALUE*PRESCALER_VALUE)
#define TWBR0_VALUE ((F_CPU/SCL_CLOCK)-16)/2

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
	pressed = bounce;
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
	uint16_t keypad = scan_keypad_pressed();
	//uint16_t keypad = scan_keypad_rising_edge();
	while(counter<16){
		if(keypad%2){return codes[counter];}
		keypad = keypad >> 1;
		counter++;
	}
	return 0;
}

int main(void) {

 twi_init();
	PCA9555_0_write(REG_CONFIGURATION_1, 0xF0); 			// Set EXT_PORT1_0 as output and EXT_PORT1_4-7 as input
	DDRB=0xFF;												// Set PORTB as output
	
	uint8_t val = 0x00;
	
	while(1){
		val = keypad_to_ascii(); 
		if(val=='1') val = 0x01;
		else if(val=='5') val = 0x02;
		else if(val=='9') val = 0x04;
		else if(val=='D') val = 0x08;
		else val = 0;
		
		PORTB = val;										// Output result
	}
	

	
return 0;
}