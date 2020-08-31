#ifndef __LCD_HD44780_IIC__
#define __LCD_HD44780_IIC__

#include <avr/io.h>
#include <util/delay.h>
#include "LCD_HD44780_IIC_config.h"

/* -------------------------------------
	USER FUNTIONS
	----------------------------------- */
void LCDsendChar(char);		                        //forms data ready to send
void LCDsendCommand(uint8_t);	                        //forms data ready to send
void LCDinit(void);			                            //Initializes LCD
void LCDclr(void);				                        //Clears LCD
void LCDhome(void);			                            //LCD cursor home
void LCDstring(char*, uint8_t);	                    //Outputs string to LCD
void LCDGotoXY(uint8_t, uint8_t);	                    //Cursor to X Y position
void LCDshiftRight(uint8_t);	                        //shift display by n characters Right
void LCDshiftLeft(uint8_t);	                            //shift display by n characters Left
void LCDcursorOn(void);		                            //Underline cursor ON
void LCDcursorOnBlink(void);	                        //Underline blinking cursor ON
void LCDcursorOFF(void);		                        //Cursor OFF
void LCDblank(void);		 	                        //LCD blank but not cleared
void LCDvisible(void);			                        //LCD visible
void LCDcursorLeft(uint8_t);	                        //Shift cursor left by n
void LCDcursorRight(uint8_t);	                        //shif cursor right by n

/* -------------------------------------
	HD44780 DEFINITIONS
	----------------------------------- */
#define delayHalfByte 25
#define delayCommand 100

#define HD44780_CLEAR				0x01

#define HD44780_HOME				0x02

#define HD44780_ENTRY_MODE			0x04
#define HD44780_EM_SHIFT_CURSOR		0
#define HD44780_EM_SHIFT_DISPLAY	0x01
#define HD44780_EM_DECREMENT		0
#define HD44780_EM_INCREMENT		0x02

#define HD44780_DISPLAY_ONOFF		0x08
#define HD44780_DISPLAY_OFF			0
#define HD44780_DISPLAY_ON			0x04
#define HD44780_CURSOR_OFF			0
#define HD44780_CURSOR_ON			0x02
#define HD44780_CURSOR_NOBLINK		0
#define HD44780_CURSOR_BLINK		0x01

#define HD44780_DISPLAY_CURSOR_SHIFT	0x10
#define HD44780_SHIFT_CURSOR		0
#define HD44780_SHIFT_DISPLAY		0x08
#define HD44780_SHIFT_LEFT			0
#define HD44780_SHIFT_RIGHT			0x04

#define HD44780_FUNCTION_SET		0x20
#define HD44780_FONT5x7				0
#define HD44780_FONT5x10			0x04
#define HD44780_ONE_LINE			0
#define HD44780_TWO_LINE			0x08
#define HD44780_4_BIT				0
#define HD44780_8_BIT				0x10

#define HD44780_CGRAM_SET			0x40

#define HD44780_DDRAM_SET			0x80

/* -------------------------------------
	IIC DEFINITIONS
	----------------------------------- */

#define BACKLIGHT_ON	(0x08)
#define BACKLIGHT_OFF	(0x00)
#define RS_COMMAND		(0x00)
#define RS_DATA			(0x01)
#define RW_WRITE		(0x00)
#define RW_READ			(0x02)
#define E_ASSERTED		(0x04)
#define E_DESSERTED		(0x00)

/* SETUP OF TWI INTERFACE		*/
#define TWI_PRESCALER_1 ((0<<TWPS1)|(0<<TWPS0))
#define TWI_PRESCALER_4 ((0<<TWPS1)|(1<<TWPS0))
#define TWI_PRESCALER_16 ((1<<TWPS1)|(0<<TWPS0))
#define TWI_PRESCALER_64 ((1<<TWPS1)|(1<<TWPS0))

/* available values are <1;255> */
#define BITRATE_REGISTER_VALUE 72
/* available values are _1, _4, _16, _64 */
#define TWI_PRESCALER TWI_PRESCALER_1
// for calculation of SCL freq. see page 222 in ATmega328P Data Sheet

// SCL frequency = 40kHz

#define ACK 1
#define NOACK 0

// TWI setup functions - some settings like 
void TWIsetup(void);
// TWI start signal
void TWIstart(void);
// TWI stop signal
void TWIstop(void);
// TWI transmit one byte of data
void TWIwrite(char data);
// TWI read one byte of data
char TWIread(char ack);

/* -------------------------------------
	BYTE SEND FUNTIONS
	----------------------------------- */
// set output for all 8 I/O of PCF8674 expander
void setOutputs8574(char data);
// send half byte to LCD/HD44780 via PCF8574
void sendHalfByteLCD(char data);
#endif
