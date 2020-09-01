#include <avr/io.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "usart.h"
#include "led.h"
#include "cmd.h"

char cmd_args[CMD_ARG_MAX+1];
uint8_t cmd_arg_cnt;
char cmd_arg_buf;

void read_cmd()
{
    /* UART reading */
    if (getchar() == 'Z')
    {
        /* COMMAND START */
        puts("LOADING");
        for (cmd_arg_cnt = 0; cmd_arg_cnt <= CMD_ARG_MAX; cmd_arg_cnt++)
        {
            while ((cmd_arg_buf = getchar()) == EOF) {};

            if (cmd_arg_buf == '\n' || cmd_arg_buf == '\r') {
                puts("READY.");
                call_cmd(cmd_arg_cnt, cmd_args);
                cmd_arg_cnt = 0;
                break;
            }
            else {
                cmd_args[cmd_arg_cnt] = cmd_arg_buf;
            }
        }

        if (cmd_arg_cnt) {
            puts("SYNTAX ERROR");
        }
    }
}


/* Command processing */
void call_cmd(uint8_t arg_cnt, char *args)
{
    puts("RUN");
    if (arg_cnt) {
        switch (args[0]) {
        case 'R':
        case 'r':
            led_state_get();
            break;
        case 'M':
        case 'm':
            if (arg_cnt == 2) led_state_set(args[1]);
            else puts("SYNTAX ERROR");
            break;
        case 'S':
        case 's':
            ADCSRA &= ~(1 << ADIE);    /* disable interrupts */
            printf("%d\n", arg_cnt);
            led_value_set(arg_cnt-1, args+1);
            break;
        default:
            puts("UNIMPLEMENTED");
            break;
        }
    }
    puts("READY.");
}