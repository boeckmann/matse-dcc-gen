#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <util/atomic.h>

#include "serial.h"
#include "stream.h"
#include "train.h"

Train *trains = NULL;
Train *last_train = NULL;

Train *train_new( uint16_t addr )
{
    Train *train = malloc( sizeof( Train ) );
    if ( train == NULL ) {
        return NULL;
    }

    train_init( train, addr );

    if ( last_train ) {
        ATOMIC_BLOCK( ATOMIC_RESTORESTATE )
        {
            train->next = trains;
            last_train->next = train;
            last_train = train;
        }
    }
    else {
        ATOMIC_BLOCK( ATOMIC_RESTORESTATE )
        {
            train->next = train;
            trains = last_train = train;
        }
    }

    return train;
}


Train *train_init( Train *train, uint16_t addr )
{
    memset( train, 0, sizeof( Train ) );

    train->dcc_mode = DCC_MODE_28;
    train->addr = addr;
    train->direction = TRAIN_FORWARD;
    train->stream_type =
        SPEED_AND_DIR; // set to SPEED_AND_DIR on first scheduling

    return train;
}


void train_activate( Train *train )
{
    if ( !train->active ) {
        train->active = 1;
        num_addresses_active++;
    }
}

void train_deactivate( Train *train )
{
    if ( train->active ) {
        num_addresses_active--;
        train->active = 0;
    }
}


static void train_schedule_function( Train *train, uint8_t f );

void train_enable_function( Train *train, uint8_t f )
{
    if ( f > 68 ) {
        return;
    }
    train->functions[f >> 3] |= 1 << ( f & 7 );
    if ( f > 12 ) {
        // Aktiviere Ausgabe des Funktionspakets für Funktionen >12
        // 1. Bit = Funktionen 13-20, 2. Bit Funktionen 21-28 etc.
        // Funktionspakete für Funktionen 0-12 werden immer generiert.
        train->f_enabled |= 1 << ( ( f - 13 ) >> 3 );
    }
    train_schedule_function( train, f );
}

void train_disable_function( Train *train, uint8_t f )
{
    if ( f > 68 ) {
        return;
    }
    train->functions[f >> 3] &= ~( 1 << ( f & 7 ) );
    if ( f > 12 ) {
        train->f_enabled |= 1 << ( ( f - 13 ) >> 3 );
    }
    train_schedule_function( train, f );
}

void train_set_dcc_mode( Train *train, uint8_t dcc_mode )
{
    train->dcc_mode = dcc_mode;
    train_set_speed_and_dir( train, 0, train->direction );
}

void train_set_speed_and_dir( Train *train, uint8_t speed, uint8_t dir )
{
    if ( train->dcc_mode == DCC_MODE_14 && speed > 14 ) {
        speed = 14;
    }
    else if ( train->dcc_mode == DCC_MODE_28 && speed > 28 ) {
        speed = 28;
    }
    else if ( speed > 126 ) {
        speed = 126;
    }

    train->speed = speed;
    train->direction = dir;

    train->stream_type = SPEED_AND_DIR;

    if ( train->active ) {
        repeated_stream_count = 0;  // prevent ISR from reading train and type
        repeated_stream_type = STREAM_TRAIN;
        repeated_stream_train = train;
        repeated_stream_count = DEFAULT_STREAM_REPITITIONS;
    }
}

Train *train_by_addr( uint16_t addr )
{
    Train *train = trains;

    if ( train == NULL ) {
        return train_new( addr );
    }

    do {
        if ( train->addr == addr ) {
            return train;
        }
        train = train->next;

    } while ( train != trains );

    return train_new( addr );
}

static void train_schedule_function( Train *train, uint8_t f )
{
    uint8_t stream;

    if ( f < 5 ) {
        stream = FUNCTION_0_4;
    }
    else if ( f < 9 ) {
        stream = FUNCTION_5_8;
    }
    else if ( f < 13 ) {
        stream = FUNCTION_9_12;
    }
    else if ( f < 21 ) {
        stream = FUNCTION_13_20;
    }
    else if ( f < 29 ) {
        stream = FUNCTION_21_28;
    }
    else if ( f < 37 ) {
        stream = FUNCTION_29_36;
    }
    else if ( f < 45 ) {
        stream = FUNCTION_37_44;
    }
    else if ( f < 53 ) {
        stream = FUNCTION_45_52;
    }
    else if ( f < 61 ) {
        stream = FUNCTION_53_60;
    }
    else {
        stream = FUNCTION_61_68;
    }

    train->stream_type = stream;

    if ( train->active ) {
        repeated_stream_count = 0;  // prevent ISR from reading train and type
        repeated_stream_type = STREAM_TRAIN;
        repeated_stream_train = train;
        repeated_stream_count = DEFAULT_STREAM_REPITITIONS;
    }
}
