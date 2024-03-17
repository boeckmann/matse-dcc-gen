#include <avr/interrupt.h>
#include <avr/io.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <util/atomic.h>
#include <util/delay.h>

#include "cmd.h"
#include "packet.h"
#include "serial.h"
#include "ticks.h"
#include "track.h"
#include "train.h"
#include "version.h"

#define DCC_DEBUG

#if 0
volatile uint16_t hiqhfreq_counter = 0;


void wait_ms(uint16_t delay)
{
    if (delay < 1) delay = 1;
    uint16_t end = hiqhfreq_counter + delay;
    while (hiqhfreq_counter != end);
}
#endif

void avr_init()
{
    TCCR0A = ( 1 << WGM01 ); // CTC Modus
    TCCR0B |= ( 1 << CS01 ); // Prescaler 8
    OCR0A = 255;
    TIMSK0 |= ( 1 << OCIE0A ); // Compare Interrupt für Timer 0 erlauben
                               // 1000Hz counter initialisieren
                               // TCCR2A = (1<<WGM21);
                               // TCCR2B |= (1<<CS22);   // Prescaler 64
                               // OCR2A = 249;
                               // TIMSK2 |= (1<<OCIE2A);
}

static char cmd[16];

int main( void )
{
    uint8_t last_tick = ticks;

    avr_init();
    track_init();

    serial_init( 9600 );

    sei(); // Global Interrupts aktivieren

    serial_puts( version_string );
    serial_puts( "\r\n" );

    track_set_power( main_track, 1 );

    while ( 1 ) {
        if ( serial_get_command( cmd, sizeof( cmd ) ) ) {
            if ( *cmd ) { // Teste, ob Kommando nicht leer
                if ( cmd_process( cmd ) ) {
                    serial_puts( "?OK\r\n" );
                }
                else {
                    serial_puts( "?ERR\r\n" );
                }
            }
        }
        else {
            serial_puts( "?ERR [transmission]\r\n" );
        }

        if ( (uint8_t)( ticks - last_tick ) > 100 ) {
            // ca., wirklich ca. 500ms Intervall
            last_tick = ticks;
        }
    }

    return 0;
}

#if 0
ISR (TIMER2_COMPA_vect)
{
    sei();  // Interrupts reaktivieren damit Signal-ISR
            // nicht aus dem Tritt kommt
    hiqhfreq_counter++;
}
#endif