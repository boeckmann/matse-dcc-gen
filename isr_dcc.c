#include "stream.h"
#include "ticks.h"
//#include "track.h"
#include <avr/interrupt.h>
#include <avr/io.h>
#include <stdint.h>

#define NUM_SYNC_BITS 17  // Anzahl Synchronisationsbits
#define SHORT_PULSE   115 // entspricht 58us für Eins-Bit
#define LONG_PULSE    199 // entspricht 100us für Null-Bit

typedef enum { FIRST_HALF, SECOND_HALF } PeriodState;

typedef enum { SYNC, DATA_START, DATA } StreamState;

typedef enum { BIT_ZERO, BIT_ONE } Bit;

#define DIRA ( 1 << PORTB4 )
#define DIRB ( 1 << PORTB5 )


static inline void track_toggle_polarity( uint8_t track )
{
    if ( track == 0 ) {
        PORTB ^= DIRA;
    }
    else {
        PORTB ^= DIRB;
    }
}

// Timer Interrupt zur Generierung des Gleissignals
// Der Interrupt ist dem Gleissignal immer eine Periode voraus! Das resultiert
// daraus, dass die Interruptauslösung Timing-kritisch ist. Daher wird die
// nächste Interruptauslösung als erste Aktion durch die ISR initialisiert. Der
// Verzögerungswert (0 oder 1 Bit) muss daher bereits in der vorherigen
// ISR Auslösung bestimmt werden.
ISR( TIMER0_COMPA_vect )
{
    static uint8_t data_bit_pos = 0;
    static uint8_t sync_bit_pos = 0;

    static StreamState stream_state = SYNC;
    static Bit current_bit = BIT_ZERO;
    static PeriodState period_state = FIRST_HALF;

    static Stream *stream;
    static uint8_t *data;
    static uint8_t data_bit_length;

    track_toggle_polarity( 1 );

    // zuerst nächste Interruptauslösung initialisieren, da Timing-kritisch!
    if ( current_bit ) {
        OCR0A = SHORT_PULSE; // Eins-Bit
    }
    else {
        OCR0A = LONG_PULSE; // Null/Bit
    }

    sei();

    track_toggle_polarity( 0 ); // Gleis umpolarisieren

    // Ab hier Statusmaschine zur Bestimmung des nächsten Bit
    if ( period_state == SECOND_HALF ) {
        period_state = FIRST_HALF;

        switch ( stream_state ) {
        case SYNC:
            current_bit = BIT_ONE;
            if ( sync_bit_pos == NUM_SYNC_BITS - 2 ) {
                plan_next_stream();
                sync_bit_pos++;
            }
            else if ( sync_bit_pos == NUM_SYNC_BITS - 1 ) {
                stream_state = DATA_START;
                sync_bit_pos = 0;

                //PORTB ^= 1 << PORTB1; // Toggle input
                stream = build_next_stream();
                data = stream->data;
                data_bit_length = stream->length << 3;
                data_bit_pos = 0;
                //PORTB ^= 1 << PORTB1; // Toggle input
            }
            else {
                sync_bit_pos++;
            }
            break;

        case DATA_START:
            current_bit = BIT_ZERO;
            stream_state = DATA;
            break;

        case DATA:
            // Bit an aktueller Datenstromposition für das Senden ans Gleis
            // maskieren und testen
            if ( data[data_bit_pos >> 3] &
                 ( 1 << ( 7 - ( data_bit_pos & 7 ) ) ) ) {
                current_bit = BIT_ONE;
            }
            else {
                current_bit = BIT_ZERO;
            }

            // An Bytegrenze testen ob Paketende erreicht ist, wenn ja
            // weiter mit Synchronisation, ansonsten Startbit (0) senden
            if ( ( data_bit_pos & 7 ) == 7 ) {
                if ( data_bit_pos == ( data_bit_length - 1 ) ) {
                    stream_state = SYNC;
#if 0
                    ++ticks; // "Systemzeit" erhöhen
#endif
                }
                else {
                    stream_state = DATA_START;
                }
            }

            data_bit_pos++;

            break;
        }
    }
    else {
        period_state = SECOND_HALF;
    }

    track_toggle_polarity( 1 );
}
