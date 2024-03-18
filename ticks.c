#include "ticks.h"
#include <avr/io.h>
#include <avr/interrupt.h>

// 100Hz tick counter
volatile uint8_t ticks = 0;

void tick_counter_init( void )
{
	TCCR1B = (1 << WGM12) | (1 << CS12); // CTC Modus + prescaler 256
	OCR1AH = 0x2;			// 16000000MHz / 256 / 100 - 1= 624 => 100Hz
	OCR1AL = 0x70;			// 624 = 0x270
	TIMSK1 = (1 << OCIE1A);	// enable interrupt

	DDRD |= 0x10;			// PORTD4 als output
}

ISR (TIMER1_COMPA_vect)
{
    sei();  // Interrupts reaktivieren damit Signal-ISR
            // nicht aus dem Tritt kommt
    PORTD ^= (1 << PORTD4);	// PORTD4 toggeln
    ticks++;
    PORTD ^= (1 << PORTD4);	// PORTD4 toggeln
}
