#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <string.h>

#include "motor.h"
#include "usart.h"
#include "cmd.h"

FILE usart_io = FDEV_SETUP_STREAM(usart_unbuff_putchar, usart_unbuff_getchar, _FDEV_SETUP_RW);

enum blink_modes {BRIGHT, BLINK, FLUID} blink_mode;
volatile uint8_t pot_val;
volatile uint8_t blink_buf;


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
    TIMSK1 &= ~(1 << ICIE1);
    TCCR1B &= 0xf8; /* clear CSxx */

    TCCR2B &= 0xf8; /* clear CSxx */
    TCCR2A = 0; /* mode 0 */
    TCNT2 = 0; /* clear counter */

    DDRD &= ~(_BV(3));

    switch (blink_mode) {
    case BRIGHT:
        OCR2B = pot_val;
        TCCR2A |= (1 << COM2B1); /* non-inverting mode */
        TCCR2A |= (1 << WGM21) | (1 << WGM20); /* fast PWM */
        TCCR2B |= (1 << CS21); /* set prescaler to 8 and starts PWM */
        break;
    case BLINK:
        /* 0.1s - 2s, half-time on, half-time off */
        ICR1 = 0x30D3; /* TODO */
        TCCR1B |= (1 << WGM12); /* CTC mode */
        TIMSK1 |= (1 << ICIE1); /* enable compare interrupt */
        TCCR1B |= (1 << CS12); /* prescaler: 256 */

        break;
    case FLUID:
        /* TODO */
        break;
    default:
        break;

    }

    sei();
}


void setup(void)
{
    cli();

    /* LED_BUILTIN / Arduino D13 / PB5 - output (status LED) */
    DDRB |= _BV(5);
    PORTB |= _BV(5);

    /* D? / PD3 - output (LED w/ PWM) */
    DDRD |= _BV(3);
    PORTB |= _BV(5);

    /* D? / PC6: input w/ pullup (button) */
    DDRC &= ~_BV(6);
    PORTC |= _BV(6);

    /* D? / PC6: analog input (potentiometer) */
    DDRC &= ~_BV(0);
    ADMUX = 0;                /* use ADC0 */
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

    set_led();

    puts("215514 READY.");
}

void loop(void)
{
    /* Button reading */
    if (bit_is_clear(PINC, 6))
    {
        button();
        set_led();
        while (bit_is_clear(PINC, 6)) {}
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
    pot_val = ADCH; /* potentiometer value */
    /* any processing here? */
    OCR2B = pot_val;
}

/* Timer1 interrupt - used for blink mode BLINK */
ISR (TIMER1_COMPA_vect) {
    blink_buf = !blink_buf;
    if (blink_buf) DDRD |= _BV(3);
    else DDRD &= ~(_BV(3));
}
