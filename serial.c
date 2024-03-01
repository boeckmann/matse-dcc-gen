//
// Created by bernd on 26.05.18.
//

#include <avr/io.h>
#include "serial.h"
#define BAUD 9600
#include <util/setbaud.h>

void serial_init(unsigned int baudrate)
{
    UBRR0H = UBRRH_VALUE;
    UBRR0L = UBRRL_VALUE;

    UCSR0B = _BV(RXEN0) | _BV(TXEN0);
    UCSR0C =  (3<<UCSZ00);
}

void serial_putc(char data)
{
    while (!(UCSR0A & (1<<UDRE0)));
    UDR0 = (uint8_t)data;
}

void serial_puts(char *data)
{
    while (*data) serial_putc(*data++);
}

int serial_getc(char *data)
{
    while (!(UCSR0A & (1<<RXC0)));
    *data = UDR0;
    return 1;
    //TODO: error handling!
}

int serial_recv_cmd(char *data, int max_len)
{
    char data_byte = 0;
    int len = 0;

    while (!serial_getc(&data_byte) || (data_byte != '!'));   // wait for start of command

    while (1) {
        if (!serial_getc(&data_byte)) return 0;
        if (len >= max_len) return 0;

        if ((data_byte == '\n') || (data_byte == '\r')) {
            data[len] = 0;
            return 1;
        } else {
            data[len++] = data_byte;
        }
    }
    return 0;
}
