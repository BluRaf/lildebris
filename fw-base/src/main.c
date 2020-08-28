#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <string.h>

#include "motor.h"
#include "usart.h"
#include "cmd.h"

FILE usart_io = FDEV_SETUP_STREAM(usart_unbuff_putchar, usart_unbuff_getchar, _FDEV_SETUP_RW);

enum blink_modes {BRIGHT, BLINK, FLUID} blink_mode = BLINK;
volatile uint8_t pot_val = 0; /* raw POT value */
volatile uint8_t scaled_time = 0; /* scaled POT value (1 = 10ms) */
volatile uint8_t cmp_cnt = 0; /* CTC hit counter */


uint8_t map(uint8_t x, uint8_t in_min, uint8_t in_max, uint8_t out_min, uint8_t out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}


void button()
{
    if (blink_mode >= FLUID)
    {
        blink_mode = BRIGHT;
    }
    else
    {
        blink_mode++;
    }
}


void set_led()
{
    cli();

    /* disable PWM and clear interrupt */
    TCCR2B = 0; /* clear prescaler */
    TCCR2A = 0; /* mode 0 */
    TCNT2 = 0; /* clear counter */

    switch (blink_mode) {
    case BRIGHT:
        TCCR2A = (1 << COM2B1) | (1 << WGM21) | (1 << WGM20); /* fast PWM, non-inverting mode */
        TCCR2B = (1 << CS22); /* clk/8 and start PWM */
        OCR2B = pot_val;
        break;
    case BLINK:
        /* 0.1s - 2s, half-time on, half-time off */
        TCCR2A = (1 << WGM21); /* Clear on compare */
        TCCR2B = (1 << CS22) | (1 << CS21) | (1 << CS20); /* set prescaler to 1024 */
        TIMSK2 |= (1 << OCIE2A);
        OCR2A = 246; /* 20Hz - every 50ms */
        break;
    case FLUID:
        /* TODO */
        break;
    default:
        break;

    }

    DDRD |= _BV(3);

    sei();
}


void setup(void)
{
    cli();

    /* LED_BUILTIN / Arduino D13 / PB5 - output (status LED) */
    DDRB |= _BV(5);
    PORTB |= _BV(5);

    /* D3 / PD3 - output (LED w/ PWM) */
    DDRD |= _BV(3);
    //PORTD |= _BV(3);

    /* A0 / PC0: input w/ pullup (button) */
    DDRC &= ~_BV(0);
    PORTC |= _BV(0);

    /* A1 / PC1: analog input (potentiometer) */
    DDRC &= ~_BV(1);
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
    //usart_init();
    //stdin = stdout = &usart_io;

    /* Unlock interrupts */
    sei();

    set_led();

    //puts("215514 READY.");
}

void loop(void)
{
    /* Button reading */
    if (bit_is_clear(PINC, 0))
    {
        //button();
        //puts("DUPA");
        set_led();
        while (bit_is_clear(PINC, 0)) {}
    }
    /* read_cmd(); */
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
    pot_val = ADCH;
    scaled_time = map(pot_val, 0, 255, 6, 122);
    if (blink_mode == BRIGHT) OCR2B = 255-pot_val;
}

/* Timer2 overflow */
ISR(TIMER2_COMPA_vect)
{
    cmp_cnt++;

    if ((blink_mode == BLINK) && (cmp_cnt >= (scaled_time>>1))) {
        PORTD ^= _BV(3);
        cmp_cnt = 0;
    }
}
