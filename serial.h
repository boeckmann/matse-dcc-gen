#include <stdint.h>

#ifndef SERIAL_H
#define SERIAL_H

void serial_init( unsigned int baudrate );
void serial_putc( char data );
void serial_puts( char *data );
int serial_getc( char *data, uint8_t timeout );

int serial_data_available( void );
int serial_get_command( char *data, int max_len );

#endif // SERIAL_H
