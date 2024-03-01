#ifndef TRACK_H

enum {
	POLARITY_POSITIVE,	// Spannung track right - left ist positiv
	POLARITY_NEGATIVE	// Spannung track right - left ist negativ
};

void track_init( void );
void track_a_set_power( int );
void track_b_set_power( int );

void track_a_toggle_polarity( void );
void track_a_set_polarity( int polarity );

#endif // TRACK_H
