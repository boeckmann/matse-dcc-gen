#include "stream.h"
#include "serial.h"
#include "track.h"
#include "train.h"
#include <stddef.h>
#include <util/atomic.h>

volatile uint8_t repeated_stream_count = 0;  // Anzahl an Wiederholungen für aktuellen Stream
uint8_t repeated_stream_type;
Train *repeated_stream_train = NULL;

volatile uint8_t num_addresses_active = 0;

int8_t estop = 0; // Nothalt

static Stream estop_stream = { .length = 3, .data = { 0x00, 0x01, 0x01 } };

static Stream estop128_stream = { .length = 4,
                                  .data = { 0x00, 0x3f, 0x01, 0x3e } };

static Stream idle_stream = { .length = 3, .data = { 0xff, 0x00, 0xff } };

static Stream reset_stream = { .length = 3, .data = { 0xff, 0x00, 0xff } };


void activate_emergency_stop( int8_t stop ) { estop = stop; }

void stream_request( uint8_t stream, uint8_t repeat, Train *train )
{
    ATOMIC_BLOCK( ATOMIC_RESTORESTATE ) {}
}

static void gen_speed_stream( Train *train, Stream *stream );
static void gen_function_stream( Train *train, Stream *stream,
                                 int functions );

void gen_train_stream( Train *train, Stream *stream )
{
    if ( train->stream_type == SPEED_AND_DIR ) {
        return gen_speed_stream( train, stream );
    }
    else {
        return gen_function_stream( train, stream, train->stream_type );
    }
}


static void plan_next_train_stream( Train *train )
{
    uint8_t *stream = &train->stream_type;
    uint8_t f_enabled = train->f_enabled;

    *stream += 1;

    switch ( *stream ) {
    case SPEED_AND_DIR:
    case FUNCTION_0_4:
    case FUNCTION_5_8:
    case FUNCTION_9_12:
        return;
    case FUNCTION_13_20:
        if ( f_enabled & 0x01 ) {
            return;
        }
        *stream += 1;
    case FUNCTION_21_28:
        if ( f_enabled & 0x02 ) {
            return;
        }
        *stream += 1;
    case FUNCTION_29_36:
        if ( f_enabled & 0x04 ) {
            return;
        }
        *stream += 1;
    case FUNCTION_37_44:
        if ( f_enabled & 0x08 ) {
            return;
        }
        *stream += 1;
    case FUNCTION_45_52:
        if ( f_enabled & 0x10 ) {
            return;
        }
        *stream += 1;
    case FUNCTION_53_60:
        if ( f_enabled & 0x20 ) {
            return;
        }
        *stream += 1;
    case FUNCTION_61_68:
        if ( f_enabled & 0x40 ) {
            return;
        }
        *stream = SPEED_AND_DIR;
    default:
        *stream = SPEED_AND_DIR;
    }
}

static uint8_t stream_type = STREAM_IDLE;
static Train *stream_train = NULL;
static uint8_t stream_odd = 0;

// Läuft im Kontext der ISR
void plan_next_stream( void )
{
    stream_odd = ~stream_odd; // markiert jedes zweite Datenpaket

    // Nothalt hat Priorität
    if ( estop ) {
        stream_type = ( stream_odd ) ? STREAM_ESTOP : STREAM_ESTOP128;
        return;
    }

    // Aktueller Stream soll wiederholt werden
    if ( repeated_stream_count > 0 ) {
        stream_type = repeated_stream_type;
        stream_train = repeated_stream_train;
        repeated_stream_count--;
        return;
    }

    // Idle Kommando senden wenn keine Züge aktiv ist. Weiterhin jedes zweite
    // Paket als Idle Kommando senden wenn nur ein Zug aktiv ist, um 5ms
    // Zeitintervall zwischen den Paketen an die gleiche Adresse einzuhalten.
    if ( !num_addresses_active || ( num_addresses_active == 1 && stream_odd ) ) {
        stream_type = STREAM_IDLE;
    }
    else {
        stream_type = STREAM_TRAIN;
        if ( stream_train == NULL ) {
            stream_train = trains;
        }
        else {
            do {
                stream_train = stream_train->next;
            } while ( !stream_train->active );
        }
    }
}


Stream *build_next_stream( void ) {
    static Stream stream;

    switch ( stream_type ) {
    case STREAM_RESET:
        return &reset_stream;
    case STREAM_TRAIN:
        gen_train_stream( stream_train, &stream );
        plan_next_train_stream( stream_train );
        return &stream;
    case STREAM_ESTOP: return &estop_stream;
    case STREAM_ESTOP128: return &estop128_stream;
    default:
        return &idle_stream;
    }
}


static inline void update_checksum( Stream *stream )
{
    uint8_t i = 0;
    uint8_t checksum = 0;

    for ( ; i < stream->length - 1; i++ ) {
        checksum ^= stream->data[i];
    }

    stream->data[stream->length - 1] = checksum;
}


static void gen_speed_stream( Train *train, Stream *stream )
{
    uint8_t *data = stream->data;

    // Zugaddresse kodieren
    if ( train->addr < 128 ) {
        stream->length = 3;
        *data++ = train->addr;
    }
    else {
        stream->length = 4;
        *data++ = 0xc0 | ( train->addr >> 8 );
        *data++ = train->addr & 0xff;
    }

    // Geschwindigkeit und Richtung kodieren
    if ( train->dcc_mode == DCC_MODE_128 ) {
        *data++ = 0x3f;
        *data = ( ( train->direction == TRAIN_FORWARD ) ? 0x80 : 0 ) |
                ( ( train->speed > 0 ) ? train->speed + 1 : 0 );
        stream->length++; // ein extra-Byte für 128 Fahrstufen
    }
    else {
        *data = 0x40 | ( ( train->direction == TRAIN_FORWARD ) ? 0x20 : 0 );
        if ( train->dcc_mode == DCC_MODE_28 ) {
            uint8_t spd = ( train->speed > 0 ) ? train->speed + 3 : 0;
            *data |= ( ( spd & 1 ) << 4 ) | ( spd >> 1 );
        }
        else {
            *data |= ( ( train->speed > 0 ) ? train->speed + 1 : 0 ) |
                     ( ( train->functions[0] & 1 ) ? 0x10 : 0 );
        }
    }

    update_checksum( stream );
}


static void gen_function_stream( Train *train, Stream *stream, int functions )
{
    uint8_t *data = stream->data;
    uint8_t fdata;

    // Zugaddresse kodieren
    if ( train->addr < 128 ) {
        stream->length = 3;
        *data++ = train->addr;
    }
    else {
        stream->length = 4;
        *data++ = 0xc0 | ( train->addr >> 8 );
        *data++ = train->addr & 0xff;
    }

    switch ( functions ) {
    case FUNCTION_0_4: // Funktionen 0-4 codieren
        fdata = ( ( train->functions[0] >> 1 ) & 0x0f ) |
                ( ( train->functions[0] << 4 ) & 0x10 );
        *data = 0x80 | fdata;
        break;
    case FUNCTION_5_8: // Funktionen 5-8 codieren
        fdata = ( train->functions[0] >> 5 ) |
                ( ( train->functions[1] << 3 ) & 0x08 );
        *data = 0xb0 | fdata;
        break;
    case FUNCTION_9_12: // Funktionen 9-12 codieren
        fdata = ( train->functions[1] >> 1 ) & 0x0f;
        *data = 0xa0 | fdata;
        break;
    case FUNCTION_13_20: // Funktionen 13-20 codieren
        fdata = ( train->functions[1] >> 5 ) |
                ( ( train->functions[2] << 3 ) & 0xf8 );
        *data++ = 0xde;
        *data = fdata;
        stream->length++;
        break;
    case FUNCTION_21_28: // Funktionen 21-28 codieren
        fdata = ( train->functions[2] >> 5 ) |
                ( ( train->functions[3] << 3 ) & 0xf8 );
        *data++ = 0xdf;
        *data = fdata;
        stream->length++;
        break;
    case FUNCTION_29_36:
        fdata = ( train->functions[3] >> 5 ) |
                ( ( train->functions[4] << 3 ) & 0xf8 );
        *data++ = 0xd8;
        *data = fdata;
        stream->length++;
        break;
    case FUNCTION_37_44:
        fdata = ( train->functions[4] >> 5 ) |
                ( ( train->functions[5] << 3 ) & 0xf8 );
        *data++ = 0xd9;
        *data = fdata;
        stream->length++;
        break;
    case FUNCTION_45_52:
        fdata = ( train->functions[5] >> 5 ) |
                ( ( train->functions[6] << 3 ) & 0xf8 );
        *data++ = 0xda;
        *data = fdata;
        stream->length++;
        break;
    case FUNCTION_53_60:
        fdata = ( train->functions[6] >> 5 ) |
                ( ( train->functions[7] << 3 ) & 0xf8 );
        *data++ = 0xdb;
        *data = fdata;
        stream->length++;
        break;
    case FUNCTION_61_68:
        fdata = ( train->functions[7] >> 5 ) |
                ( ( train->functions[8] << 3 ) & 0xf8 );
        *data++ = 0xdc;
        *data = fdata;
        stream->length++;
        break;
    }

    update_checksum( stream );
}
