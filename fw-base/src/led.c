#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "LCD_HD44780_IIC.h"

#include "led.h"

enum blink_modes blink_mode = BRIGHT;
volatile enum source_modes source_mode = POT;

volatile uint8_t pot_val = 0; /* raw POT value */
volatile uint8_t scaled_time = 0; /* scaled POT value (2 = 10ms) */
volatile uint8_t cmp_cnt = 0; /* CTC hit counter */
volatile  int8_t inc_dir = 1;

void calculate_timers(uint8_t pot, uint8_t sc_time)
{
    if (blink_mode == BRIGHT) OCR2B = 255-pot;
    else if (blink_mode == FLUID) OCR0A = sc_time;
}

void led_state_get()
{
    switch (blink_mode) {
    case FLUID:
        printf("FLUID:%d\n", scaled_time<<3);
        break;
    case BRIGHT:
        printf("FLUID:%d\n", pot_val);
        break;
    case BLINK:
        printf("BLINK:%d\n", scaled_time<<3);
        break;
    default:
        break;
    }
}

void led_state_set(char mode)
{
    switch (mode) {
    case 'F':
    case 'f':
        blink_mode = FLUID;
        puts("FLUID");
        source_mode = SER;
        break;
    case 'R':
    case 'r':
        blink_mode = BRIGHT;
        puts("BRIGHT");
        source_mode = SER;
        break;
    case 'L':
    case 'l':
        blink_mode = BLINK;
        puts("BLINK");
        source_mode = SER;
        break;
    default:
        puts("INVALID MODE");
        break;
    }

    set_led();
}

void led_value_set(uint8_t ac, char *av)
{
    uint16_t conv_num;
    char conv_str[5];

    memcpy(conv_str, av, ac);
    conv_str[ac] = '\0';
    conv_num = atoi(conv_str);

    switch (blink_mode) {
    case BRIGHT:
        if (conv_num <= 255 ) {
            pot_val = 255 - conv_num;
            source_mode = SER;
        }
        else pot_val = puts("TOO BIG");
        break;
    case FLUID:
    case BLINK:
        scaled_time = conv_num>>3;
        source_mode = SER;
        break;
    default:
        break;
    }

    calculate_timers(pot_val, scaled_time);
}

void set_led()
{
    cli();

    /* disable Timer0 */
    TCCR0B = 0; /* clear prescaler */
    TCCR0A = 0; /* mode 0 */
    TCNT0 = 0; /* clear counter */
    TIMSK0 = 0;
    /* disable Timer2 */
    TCCR2B = 0; /* clear prescaler */
    TCCR2A = 0; /* mode 0 */
    TCNT2 = 0; /* clear counter */
    TIMSK2 = 0;

    /* Set timers */
    switch (blink_mode) {
    case FLUID:
        TCCR0A = (1 << WGM21); /* Clear on compare */
        TCCR0B = (1 << CS02); /* set prescaler to 1024 */
        TIMSK0 = (1 << OCIE0A);
        OCR0A = scaled_time;
    case BRIGHT:
        TCCR2A = (1 << COM2B1) | (1 << WGM21) | (1 << WGM20); /* fast PWM, non-inverting mode */
        TCCR2B = (1 << CS22); /* clk/8 and start PWM */
        OCR2B = pot_val;
        break;
    case BLINK:
        /* 0.1s - 2s, half-time on, half-time off */
        TCCR2A = (1 << WGM21); /* Clear on compare */
        TCCR2B = (1 << CS22) | (1 << CS21) | (1 << CS20); /* set prescaler to 1024 */
        TIMSK2 = (1 << OCIE2A);
        OCR2A = 246; /* 20Hz - every 50ms */
        break;
    default:
        break;
    }

    switch (blink_mode) {
    case FLUID:
        puts("FLUID");
    	LCDGotoXY(10,1);
	    LCDstring(" FLUID", 6);
        break;
    case BRIGHT:
        puts("BRIGHT");
        LCDGotoXY(10,1);
        LCDstring("BRIGHT", 6);
        break;
    case BLINK:
        puts("BLINK");
        LCDGotoXY(10,1);
	    LCDstring(" BLINK", 6);
        break;
    default:
        break;
    }

    DDRD |= (1<<PD3);

    sei();
}

/* Timer0 CTC interrupt */
ISR(TIMER0_COMPA_vect)
{
    if (OCR2B == 0) inc_dir = 1;
    else if (OCR2B == 255) inc_dir = -1;
    OCR2B += inc_dir;
}

/* Timer2 CTC interrupt */
ISR(TIMER2_COMPA_vect)
{
    cmp_cnt++;

    if ((blink_mode == BLINK) && (cmp_cnt >= (scaled_time>>2))) {
        PORTD ^= (1<<PD3);
        cmp_cnt = 0;
    }
}
