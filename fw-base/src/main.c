#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdio.h>
#include <string.h>

#include "usart.h"
#include "cmd.h"
#include "led.h"
#include "LCD_HD44780_IIC.h"

FILE usart_io = FDEV_SETUP_STREAM(usart_putchar, usart_getchar, _FDEV_SETUP_RW);

char numstr[4];
char numstr2[6];
char letter;


uint8_t map(uint8_t x, uint8_t in_min, uint8_t in_max, uint8_t out_min, uint8_t out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}


void setup(void)
{
    cli();

    /* LED_BUILTIN / Arduino D13 / PB5 - output (status LED) */
    DDRB |= (1<<PB5);
    PORTB &= ~(1<<PB5);

    /* D3 / PD3 - output (LED w/ PWM) */
    DDRD |= (1<<PD3);
    //PORTD |= (1<<PD3);

    /* A0 / PC0: input w/ pullup (button) */
    DDRC &= ~(1<<PC0);
    PORTC |= (1<<PC0);

    /* A1 / PC1: analog input (potentiometer) */
    DDRC &= ~(1<<PC1);
    ADMUX = 1;                /* use ADC1 */
    ADMUX |= (1 << REFS0);    /* use AVcc as the reference */
    ADMUX |= (1 << ADLAR);    /* Right adjust for 8 bit resolution */

    ADCSRA |= (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0); /* 128 prescale for 16Mhz */
    ADCSRA |= (1 << ADATE);   /* Set ADC Auto Trigger Enable */
    
    ADCSRB = 0;               /* 0 for free running mode */

    ADCSRA |= (1 << ADEN);    /* Enable the ADC */
    ADCSRA |= (1 << ADIE);    /* Enable Interrupts */

    ADCSRA |= (1 << ADSC);    /* Start the ADC conversion */

    /* Initialize USART */
    usart_init();
    stdin = stdout = &usart_io;

    /* Unlock interrupts */
    sei();

	LCDinit();
	LCDhome();

    set_led();

    puts("215514 READY.");
}

void loop(void)
{
    /* Button reading */
    if (bit_is_clear(PINC, 0))
    {
        PORTB |= (1<<PB5);
        ADCSRA |= (1 << ADIE);    /* Enable Interrupts */
        if (blink_mode >= FLUID) blink_mode = BRIGHT;
        else blink_mode++;
        set_led();
        while (bit_is_clear(PINC, 0)) {}
        PORTB &= ~(1<<PB5);
    }

    LCDGotoXY(0,0);
    switch (source_mode)
    {
    case POT:
        snprintf(numstr, 4, "%3d", pot_val);
        LCDstring("Pot:", 4);
        LCDstring(numstr, 3);
        break;
    case SER:
        LCDstring("Serial ", 7);
        break;
    default:
        break;
    }

    LCDGotoXY(0,1);
    if (blink_mode != BRIGHT) {
        LCDstring("Okr.:", 5);
        snprintf(numstr2, 6, "%4d", scaled_time<<3);
        LCDstring(numstr2, 4);
    }
    else {
        LCDstring("         ", 9);
    }

    LCDGotoXY(9,0);
    if (blink_mode != BLINK) {
        snprintf(numstr, 4, "%3d", OCR2B);
	    LCDstring("PWM:", 4);
        LCDstring(numstr, 3);
    }
    else {
        LCDstring("         ", 7);
    }

    read_cmd();
}

int main(void)
{
    setup();

    for (;;) {
        loop();
    }

    return 0;
}

/* ADC interrupt - used for setting new Timer2/PWM value */
ISR(ADC_vect)
{
    source_mode = POT;
    pot_val = ADCH;
    scaled_time = map(pot_val, 0, 255, 12, 245);
    calculate_timers(pot_val, scaled_time);
}