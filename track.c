// Ansteuerung Gleise
// Implementierung für Arduino Motor-Shield

// Ausgang A entspricht Track 0 und Ausgang B entspricht Track 1
// Hauptgleis standardmäßig an Ausgang A (Track 0)
// Programmiergleis an Ausgang B (Track 1)

#include "track.h"
#include <avr/io.h>

#define DIRA   ( 1 << PORTB4 )
#define DIRB   ( 1 << PORTB5 )
#define PWMA   ( 1 << PORTD3 )
#define PWMB   ( 1 << PORTB3 )
#define BRAKEA ( 1 << PORTB1 )
#define BRAKEB ( 1 << PORTB0 )

#define PORTB_MASK 0x3b /* bits 5, 4, 3, 1, 0 */
#define PORTD_MASK 0x08 /* bits 3 */

uint8_t main_track = 0; // Hauptgleis
uint8_t prog_track = 1; // Programmiergleis

void track_init( void )
{
    DDRB |= PORTB_MASK;
    DDRD |= PORTD_MASK;

    // Gleisspeisung aus und Polarisierung auf definierten Zustand setzen
    track_set_power( 0, 0 );
    track_set_power( 1, 0 );
    track_set_polarity( 0, 0 );
    track_set_polarity( 1, 0 );

    PORTB &= ~BRAKEA;
    PORTB &= ~BRAKEB;
}

void track_set_power( uint8_t track, uint8_t power )
{
    if ( track == 0 ) { // Arduino Motor-Shield Ausgang A
        if ( power ) {
            PORTD |= PWMA;
        }
        else {
            PORTD &= ~PWMA;
        }
    }
    else { // Arduino Motor-Shield Ausgang B
        if ( power ) {
            PORTB |= PWMB;
        }
        else {
            PORTB &= ~PWMB;
        }
    }
}

void track_set_polarity( uint8_t track, uint8_t polarity )
{
    if ( track == 0 ) { // Arduino Motor-Shield Ausgang A
        if ( polarity ) {
            PORTB |= DIRA;
        }
        else {
            PORTB &= ~DIRA;
        }
    }
    else { // Arduino Motor-Shield Ausgang B
        if ( polarity ) {
            PORTB |= DIRB;
        }
        else {
            PORTB &= ~DIRB;
        }
    }
}

void track_toggle_polarity( uint8_t track )
{
    if ( track == 0 ) {
        PORTB ^= DIRA;
    }
    else {
        PORTB ^= DIRB;
    }
}
