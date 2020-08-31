#include <avr/io.h>
#include <string.h>

#include "usart.h"
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

            if (cmd_arg_buf == '\n' || cmd_arg_buf == '\r')
            {
                puts("READY.");
                call_cmd(cmd_arg_cnt, cmd_args);
                cmd_arg_cnt = 0;
                break;
            }
            else {
                cmd_args[cmd_arg_cnt] = cmd_arg_buf;
            }
        }

        if (cmd_arg_cnt)
        {
            puts("SYNTAX ERROR");
        }
    }
}

/* Command processing */
void call_cmd(uint8_t arg_cnt, char *args)
{
    puts("RUN");
    if (arg_cnt)
    {
        switch (args[0])
        {
            case 'N':
                puts("y+");
                break;
            case 'S':
                puts("y-");
                break;
            case 'W':
                puts("x-");
                break;
            case 'E':
                puts("x+");
                break;
            case '.':
                puts("STOP");
                break;
            default:
                puts("UNIMPLEMENTED");
                break;
        }
    }
    puts("READY.");
}