#include "serial.h"
#include <avr/io.h>
#include <stdint.h>
#define BAUD 9600
#include "ticks.h"
#include <util/setbaud.h>

#define SERIAL_TIMEOUT 200 // Anzahl an Ticks bis Timeout (2s)

void serial_init( unsigned int baudrate )
{
    UBRR0H = UBRRH_VALUE;
    UBRR0L = UBRRL_VALUE;

    UCSR0B = _BV( RXEN0 ) | _BV( TXEN0 );
    UCSR0C = ( 3 << UCSZ00 );
}


void serial_putc( char data )
{
    while ( !( UCSR0A & ( 1 << UDRE0 ) ) )
        ; // Warte bis Schreibbuffer leer
    UDR0 = (uint8_t)data;
}


void serial_puts( char *data )
{
    while ( *data ) {
        serial_putc( *data++ );
    }
}


int serial_getc( char *data, uint8_t timeout )
{
    uint8_t start_ticks = ticks;

    // Warte auf Byte in Lesebuffer
    while ( !( UCSR0A & ( 1 << RXC0 ) ) ) {
        // minimum 2 Ticks (ca. 10-20ms) warten, dann Timeout
        if ( timeout && (uint8_t)( ticks - start_ticks ) > timeout ) {
            return 0;
        }
    }

    if ( UCSR0A & ( _BV( FE0 ) | _BV( DOR0 ) | _BV( UPE0 ) ) ) {
        // Transferfehler?
        return 0;
    }

    *data = UDR0; // Byte lesen
    return 1;
}


int serial_data_available( void ) { return UCSR0A & ( 1 << RXC0 ); }


// Liest ein (möglicherweise leeres Kommando) ein. Sollte das erste Byte
// kein Kommandostartbyte '!' sein, wird ein leeres Kommando und Erfolg
// zurückgegeben, um von Fehlern bei der eigentlichen Übertragung zu
// unterscheiden.
// Rückgabe: 1=Erfolg, 0=Fehler
int serial_get_command( char *data, int max_len )
{
    char data_byte = 0;
    int len = 0;
    *data = '\0';

    if ( !serial_data_available() ) {
        return 1;
    }
    serial_getc( &data_byte, 2 );
    // Kein Kommandostart? Interpretiere als leeres Kommando (kein Fehler)
    if ( data_byte != '!' ) {
        return 1;
    }

    // Lese Kommando
    while ( 1 ) {
        // Nach Kommandostart werden Timeouts etc. als Fehler gewertet
        if ( !serial_getc( &data_byte, SERIAL_TIMEOUT ) ) {
            return 0;
        }
        if ( len >= max_len ) {
            return 0;
        }

        if ( ( data_byte == '\n' ) || ( data_byte == '\r' ) ) {
            data[len] = 0;
            return 1;
        }
        else {
            data[len++] = data_byte;
        }
    }
    return 0;
}
