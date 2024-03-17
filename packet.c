#include "packet.h"
#include "train.h"
#include <stddef.h>
#include <util/atomic.h>

volatile uint8_t num_addresses_active = 0;

int8_t estop = 0; // Nothalt

static Packet estop_packet = { .length = 3, .data = { 0x00, 0x41, 0x41 } };

static Packet estop128_packet = { .length = 4,
                                  .data = { 0x00, 0x3f, 0x81, 0xbe } };

static Packet idle_packet = { .length = 3, .data = { 0xff, 0x00, 0xff } };

static Packet reset_packet = { .length = 3, .data = { 0xff, 0x00, 0xff } };


void activate_emergency_stop( int8_t stop ) { estop = stop; }

void packet_request( uint8_t packet, uint8_t repeat, Train *train )
{
    ATOMIC_BLOCK( ATOMIC_RESTORESTATE )
    {
    }
}

static void gen_speed_packet( Train *train, Packet *pkt );
static void gen_function_packet( Train *train, Packet *pkt, int functions );

void gen_train_packet( Train *train, Packet *pkt )
{
    if ( train->stream == SPEED_AND_DIR ) {
        return gen_speed_packet( train, pkt );
    }
    else {
        return gen_function_packet( train, pkt, train->stream );
    }
}

static void next_train_stream( Train *train )
{
again:
    if ( train->stream == FUNCTION_61_68 ) {
        train->stream = SPEED_AND_DIR;
        return;
    }

    train->stream++;

    if ( train->stream >= FUNCTION_13_20 ) {
        serial_puts("x");
        goto again;
//        if ( !(train->f_enabled & (1 << (train->stream - FUNCTION_13_20) ))) {
//            goto again;
//        }
    }
}

// Läuft im Kontext der ISR
Packet *schedule_next_packet()
{
    static uint8_t odd = 0;
    static uint8_t packet_type = PACKET_IDLE;
    static Packet packet;
    static Train *train = NULL;

    odd = ~odd; // markiert jedes zweite Datenpaket

    // Nothalt hat Priorität
    if ( estop ) {
        return ( odd ) ? &estop_packet : &estop128_packet;
    }

#if 0
    // Soll Paket "außer der Reihe" gesendet werden?
    if ( requested_packet.repeat_count > 0 ) {
        packet_type = requested_packet.type;
        train = requested_packet.train;
        requested_packet.repeat_count--;

        // Scheduling überspringen und direkt Paket generieren, wenn
        // Paketwiederholung ansteht
        goto generate;
    }

    if ( packet_type == PACKET_RESET ) {
        // Nach Rücksetzbefehl zunächst Idle Pakete senden
        requested_packet.type = PACKET_IDLE;
        requested_packet.repeat_count = 9; // Insgesamt 10 Idle Pakete
        packet_type = PACKET_IDLE;
        goto generate;
    }
#endif
    // Idle Kommando senden wenn keine Züge aktiv ist. Weiterhin jedes zweite
    // Paket als Idle Kommando senden wenn nur ein Zug aktiv ist, um 5ms
    // Zeitintervall zwischen den Paketen an die gleiche Adresse einzuhalten.
    //if ( !num_trains_active || ( num_trains_active == 1 && odd ) ) {

    if ( !num_addresses_active || (num_addresses_active == 1 && odd)) {
        packet_type = PACKET_IDLE;
    }
    else {
        packet_type = PACKET_TRAIN;
        do {
            train = train->next;
        } while ( !train->active );
    }

    switch ( packet_type ) {
    case PACKET_RESET:
        return &reset_packet;
    case PACKET_TRAIN:
        gen_train_packet( train, &packet );
        next_train_stream( train );
        return &idle_packet;
    default:
        return &idle_packet;
    }
}

static inline void update_checksum( Packet *pkt )
{
    uint8_t i = 0;
    uint8_t checksum = 0;

    for ( ; i < pkt->length - 1; i++ ) {
        checksum ^= pkt->data[i];
    }

    pkt->data[pkt->length - 1] = checksum;
}

static void gen_speed_packet( Train *train, Packet *pkt )
{
    uint8_t *data = pkt->data;

    // Zugaddresse kodieren
    if ( train->addr < 128 ) {
        pkt->length = 3;
        *data++ = train->addr;
    }
    else {
        pkt->length = 4;
        *data++ = 0xc0 | ( train->addr >> 8 );
        *data++ = train->addr & 0xff;
    }

    // Geschwindigkeit und Richtung kodieren
    if ( train->dcc_mode == DCC_MODE_128 ) {
        *data++ = 0x3f;
        *data = ( ( train->direction == TRAIN_FORWARD ) ? 0x80 : 0 ) |
                ( ( train->speed > 0 ) ? train->speed + 1 : 0 );
        pkt->length++; // ein extra-Byte für 128 Fahrstufen
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

    update_checksum( pkt );
}

static void gen_function_packet( Train *train, Packet *pkt, int functions )
{
    uint8_t *data = pkt->data;
    uint8_t fdata;

    // Zugaddresse kodieren
    if ( train->addr < 128 ) {
        pkt->length = 3;
        *data++ = train->addr;
    }
    else {
        pkt->length = 4;
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
        pkt->length++;
        break;
    case FUNCTION_21_28: // Funktionen 21-28 codieren
        fdata = ( train->functions[2] >> 5 ) |
                ( ( train->functions[3] << 3 ) & 0xf8 );
        *data++ = 0xdf;
        *data = fdata;
        pkt->length++;
        break;
    case FUNCTION_29_36:
        fdata = ( train->functions[3] >> 5 ) |
                ( ( train->functions[4] << 3 ) & 0xf8 );
        *data++ = 0xd8;
        *data = fdata;
        pkt->length++;
        break;
    case FUNCTION_37_44:
        fdata = ( train->functions[4] >> 5 ) |
                ( ( train->functions[5] << 3 ) & 0xf8 );
        *data++ = 0xd9;
        *data = fdata;
        pkt->length++;
        break;
    case FUNCTION_45_52:
        fdata = ( train->functions[5] >> 5 ) |
                ( ( train->functions[6] << 3 ) & 0xf8 );
        *data++ = 0xda;
        *data = fdata;
        pkt->length++;
        break;
    case FUNCTION_53_60:
        fdata = ( train->functions[6] >> 5 ) |
                ( ( train->functions[7] << 3 ) & 0xf8 );
        *data++ = 0xdb;
        *data = fdata;
        pkt->length++;
        break;
    case FUNCTION_61_68:
        fdata = ( train->functions[7] >> 5 ) |
                ( ( train->functions[8] << 3 ) & 0xf8 );
        *data++ = 0xdc;
        *data = fdata;
        pkt->length++;
        break;
    }

    update_checksum( pkt );
}
