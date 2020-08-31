/* USART helper library
 * based on https://pcarduino.blogspot.com/2013/10/atmega328s-usart.html
 */

#include "usart.h"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/setbaud.h>

static volatile buffer_t usart_rx_buf, usart_tx_buf;


void usart_init()
{
    /* baud rate value - higher and lower byte */ 
    UBRR0H = UBRRH_VALUE;
    UBRR0L = UBRRL_VALUE;

#if USE_2X
    UCSR0A |= _BV(U2X0);
#else
    UCSR0A &= ~(_BV(U2X0));
#endif

    /* async serial: 8N1 */
    UCSR0C = _BV(UCSZ01) | _BV(UCSZ00);
    /* enable RX and TX */
    UCSR0B = _BV(RXEN0) | _BV(TXEN0);
}



/* Unbuffered IO */

int usart_unbuff_getchar(FILE *stream)
{
    while ((UCSR0A & _BV(RXC0)) == 0) {};            /* wait until data register is ready to read byte from it */
    return UDR0;                                     /* read byte */
}

int usart_unbuff_putchar(char c, FILE *stream)
{
    while ((UCSR0A & _BV(UDRE0)) == 0) {};           /* wait until data register is ready to push byte to it */
    UDR0 = c;                                        /* send byte */
    return c;
}


// /* Buffered IO */

int usart_getchar(FILE *stream)
{
    char c;
    /* check if ring buffer is empty */
    if (usart_rx_buf.head == usart_rx_buf.tail)
        return EOF;
    
    /* read byte from ring buffer */
    c = usart_rx_buf.ring[usart_rx_buf.tail];
    usart_rx_buf.tail = (usart_rx_buf.tail + 1) % SERIAL_RX_RING_SIZE;

    return c;
}

int usart_putchar(char c, FILE *stream)
{
    char n = EOF;
    uint8_t next = ((usart_tx_buf.head + 1) % SERIAL_TX_RING_SIZE);

    /* write byte to tx ring buffer */
    if (next != usart_tx_buf.tail) {
        usart_tx_buf.ring[usart_tx_buf.head] = c;
        usart_tx_buf.head = next;
        n = 0;

        /* enable data register empty interrupt */
        UCSR0B |= _BV(UDRIE0);
    }

    return n;
}


// /* Interrupt handlers for buffered IO */

ISR(USART_RX_vect, ISR_BLOCK)
{
    if (bit_is_clear(UCSR0A, FE0)) {
        /* read byte and calculate new head position */
        volatile unsigned char data = UDR0;
        UDR0 = data;
        volatile unsigned char next = ((usart_rx_buf.head + 1) % SERIAL_RX_RING_SIZE);
        if (next != usart_rx_buf.tail) {
            /* write to ring buffer */
            usart_rx_buf.ring[usart_rx_buf.head] = data;
            usart_rx_buf.head = next;			
        }
    }
    else {
        /* read anyway to clear interrupt flag */
        volatile unsigned char data __attribute__((unused)) = UDR0;
	}
}

ISR(USART_UDRE_vect, ISR_BLOCK) {
    /* send next byte if anything left in ring buffer */
    if (usart_tx_buf.head != usart_tx_buf.tail) {
        UDR0 = usart_tx_buf.ring[usart_tx_buf.tail];
        usart_tx_buf.tail = (usart_tx_buf.tail + 1) % SERIAL_TX_RING_SIZE;
    }
    else {
        /* mask interrupt, everything was sent */
        UCSR0B &= ~_BV(UDRIE0);
    }
}