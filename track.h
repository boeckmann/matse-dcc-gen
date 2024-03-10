#ifndef TRACK_H
#define TRACK_H

#include <stdint.h>

enum {
    POLARITY_POSITIVE, // Spannung track right - left ist positiv
    POLARITY_NEGATIVE  // Spannung track right - left ist negativ
};

extern uint8_t main_track; // Hauptgleis
extern uint8_t prog_track; // Programmiergleis

void track_init( void );
void track_set_power( uint8_t track, uint8_t power );

void track_toggle_polarity( uint8_t track );
void track_set_polarity( uint8_t track, uint8_t polarity );

#endif // TRACK_H
