//
// Created by bernd on 26.05.18.
//

#include <stdint.h>

#ifndef RAIL03_SERIAL_H
#define RAIL03_SERIAL_H

void serial_init(unsigned int baudrate);
void serial_putc(char data);
void serial_puts(char *data);
int serial_getc(char *data);

int serial_recv_cmd(char *data, int max_len);

#endif //RAIL03_SERIAL_H
