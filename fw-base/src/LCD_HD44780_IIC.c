#include "LCD_HD44780_IIC.h"

/* -------------------------------------
	HD44780 DEFINITIONS
	----------------------------------- */

/* PCF8574 expander bits 
7	6	5	4	3	2	1	0
RS	RW	E	BT	D7	D6	D5	D4
*/

// definitions of global variables
volatile unsigned char BLval = BACKLIGHT_ON;
volatile unsigned char RSval = RS_COMMAND;
//unsigned char RWval = RW_WRITE;

/* -------------------------------------
	USER FUNTIONS
	----------------------------------- */
//forms data ready to send
void LCDsendChar(char data)		                        
{
	RSval = RS_DATA;
	sendHalfByteLCD(data >> 4);
	sendHalfByteLCD(data & 0x0F);
	_delay_us(delayCommand);
}
//forms data ready to send
void LCDsendCommand(uint8_t data)	                        
{
	RSval = RS_COMMAND;
	sendHalfByteLCD(data >> 4);
	sendHalfByteLCD(data & 0x0F);
	_delay_us(delayCommand);
}
//Initializes LCD
void LCDinit(void)			                            
{
	unsigned char itr=0;
	
	TWIsetup();
	
	RSval = RS_COMMAND;
	for(itr=0;itr<3;itr++){
		sendHalfByteLCD(0x03);
		_delay_ms(6);
	}
	sendHalfByteLCD(0x02);
	_delay_ms(1);
	LCDsendCommand(HD44780_FUNCTION_SET | HD44780_FONT5x10 | HD44780_TWO_LINE | HD44780_4_BIT); // 5x10 font, two line, 4bit interface
	LCDsendCommand(HD44780_DISPLAY_ONOFF | HD44780_DISPLAY_OFF); // turn off display
	LCDsendCommand(HD44780_CLEAR); // clear DDRAM
	_delay_ms(500);
	LCDsendCommand(HD44780_ENTRY_MODE | HD44780_EM_SHIFT_CURSOR | HD44780_EM_INCREMENT);// inkrement addres, move coursore mode
	LCDsendCommand(HD44780_DISPLAY_ONOFF | HD44780_DISPLAY_ON | HD44780_CURSOR_OFF | HD44780_CURSOR_NOBLINK); // turn on LCD, cursor on, blink on
}
//Clears LCD
void LCDclr(void)
{
	LCDsendCommand(HD44780_CLEAR);
}		                        
//LCD cursor home
void LCDhome(void)
{
	LCDsendCommand(HD44780_HOME);
	_delay_ms(10);
}			                            
//Outputs string to LCD
void LCDstring(char* data, uint8_t nOfBytes){
	uint8_t i;
	
	if (!data) return; // check the pointer

	for(i=0; i<nOfBytes; i++)
		LCDsendChar(data[i]);
}
//Cursor to X Y position
void LCDGotoXY(uint8_t x, uint8_t y){
	LCDsendCommand(HD44780_DDRAM_SET | (x + (0x40 * y)));
}
//shift by n characters Right
void LCDshiftRight(uint8_t n){
	uint8_t i;
	for (i=0;i<n;i++)
		LCDsendCommand(HD44780_DISPLAY_CURSOR_SHIFT | HD44780_SHIFT_DISPLAY | HD44780_SHIFT_RIGHT);
}	
//shift by n characters Left                        
void LCDshiftLeft(uint8_t n){
	uint8_t i;
	for (i=0;i<n;i++)
		LCDsendCommand(HD44780_DISPLAY_CURSOR_SHIFT | HD44780_SHIFT_DISPLAY | HD44780_SHIFT_LEFT);
}	
//Underline cursor ON                            
void LCDcursorOn(void){
	LCDsendCommand(HD44780_DISPLAY_ONOFF | HD44780_DISPLAY_ON | HD44780_CURSOR_ON | HD44780_CURSOR_NOBLINK);
}	
//Underline blinking cursor ON	                            
void LCDcursorOnBlink(void){
	LCDsendCommand(HD44780_DISPLAY_ONOFF | HD44780_DISPLAY_ON | HD44780_CURSOR_ON | HD44780_CURSOR_BLINK);
}
//Cursor OFF
void LCDcursorOFF(void){
	LCDsendCommand(HD44780_DISPLAY_ONOFF | HD44780_DISPLAY_ON | HD44780_CURSOR_OFF | HD44780_CURSOR_NOBLINK);
}	
//LCD blank but not cleared	                        
void LCDblank(void){
	LCDsendCommand(HD44780_DISPLAY_ONOFF | HD44780_DISPLAY_OFF | HD44780_CURSOR_OFF | HD44780_CURSOR_NOBLINK);
}
//LCD visible
void LCDvisible(void){
	LCDsendCommand(HD44780_DISPLAY_ONOFF | HD44780_DISPLAY_ON | HD44780_CURSOR_OFF | HD44780_CURSOR_NOBLINK);
}
//Shift cursor left by n
void LCDcursorLeft(uint8_t n){
	uint8_t i;
	for (i=0;i<n;i++)
		LCDsendCommand(HD44780_DISPLAY_CURSOR_SHIFT | HD44780_SHIFT_CURSOR | HD44780_SHIFT_LEFT);
}
//Shift cursor right by n	                        
void LCDcursorRight(uint8_t n){
	uint8_t i;
	for (i=0;i<n;i++)
		LCDsendCommand(HD44780_DISPLAY_CURSOR_SHIFT | HD44780_SHIFT_CURSOR | HD44780_SHIFT_RIGHT);
}	

/* -------------------------------------
	IIC DEFINITIONS
	----------------------------------- */
// TWI setup functuons - some settings like
void TWIsetup(void){
	TWBR = BITRATE_REGISTER_VALUE;	// bit rate register
	TWSR = TWI_PRESCALER;				// prescaler
}

// TWI start signal
void TWIstart(void){
	TWCR = (1<<TWINT)|(1<<TWSTA)|(1<<TWEN);
	while (!(TWCR & (1<<TWINT))); // wait for the end of start condition
}
// TWI stop signal
void TWIstop(void){
	TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWSTO);
	while ((TWCR & (1<<TWSTO))); // wait for the end of stop condition
}
// TWI transmit one byte of data
void TWIwrite(char data){
	TWDR = data;
	TWCR = (1<<TWINT) | (1<<TWEN);
	while (!(TWCR & (1<<TWINT))); // wait for the end of transmision
}
// TWI read one byte of data
char TWIread(char ack){
	TWCR = ack
	? ((1 << TWINT) | (1 << TWEN) | (1 << TWEA)) // read with ACK
	: ((1 << TWINT) | (1 << TWEN)) ;			// read with NACK
	while (!(TWCR & (1<<TWINT)));				// wait for the end of transmision
	return TWDR;								// return recived byte
}                        

/* -------------------------------------
	BYTE SEND FUNTIONS
	----------------------------------- */
// set output for all 8 I/O of PCF8674 expander
void setOutputs8574(char data){
	TWIstart();
	TWIwrite(PC8574_ADDRESS<<1);
	TWIwrite(data);
	TWIstop();
	_delay_us(10);
}

// send half byte to LCD/HD44780 via PCF8574
void sendHalfByteLCD(char data){
	setOutputs8574(RSval | RW_WRITE | E_ASSERTED | BLval | (data<<4) );
	_delay_us(delayHalfByte);
	setOutputs8574(RSval | RW_WRITE | E_DESSERTED | BLval | (data<<4) );	
	_delay_us(delayHalfByte);
}

