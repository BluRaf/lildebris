#include <stdint.h>
#include <stdio.h>

#ifndef BAUD
    #define BAUD 9600
#endif

#ifndef SERIAL_RX_RING_SIZE
    #define SERIAL_RX_RING_SIZE 32
#endif

#ifndef SERIAL_TX_RING_SIZE
    #define SERIAL_TX_RING_SIZE 32
#endif

typedef struct buffer_s {
	/* ring buffer array */
	volatile unsigned char ring[SERIAL_RX_RING_SIZE];
	/* read header position */
	volatile uint8_t head;
	/* write tail position */
	volatile uint8_t tail;
} buffer_t;

extern void usart_init();
extern int usart_getchar(FILE *stream);
extern int usart_putchar(char c, FILE *stream);
extern int usart_unbuff_getchar(FILE *stream);
extern int usart_unbuff_putchar(char c, FILE *stream);

