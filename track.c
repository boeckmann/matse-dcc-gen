#include<avr/io.h>
#include "track.h"

#define DIRA (1<<PORTB4)
#define DIRB (1<<PORTB5)
#define PWMA (1<<PORTD3)
#define PWMB (1<<PORTB3)
#define BRAKEA (1<<PORTB1)
#define BRAKEB (1<<PORTB0)

#define PORTB_MASK 0x3b     /* bits 5, 4, 3, 1, 0 */
#define PORTD_MASK 0x08     /* bits 3 */


void track_init( void )
{
    DDRB |= PORTB_MASK;
    DDRD |= PORTD_MASK;

	track_a_set_power( 0 );
	track_b_set_power( 0 );
	track_a_set_polarity( 0 );

    PORTB &= ~BRAKEA;
    PORTB &= ~BRAKEB;
}

void track_a_set_power( int power )
{
	if ( power ) {
	    PORTD |= PWMA;
	}
	else {
		PORTD &= ~PWMA;		
	}
}

void track_b_set_power( int power )
{
	if ( power ) {
	    PORTB |= PWMB;
	}
	else {
		PORTB &= ~PWMB;		
	}
}

void track_a_set_polarity( int polarity )
{
    if ( polarity ) {
        PORTB |= DIRA;
    }
    else {
        PORTB &= ~DIRA;        
    }
}

void track_a_toggle_polarity( void )
{
    PORTB ^= DIRA;
}
