//
// Created by bernd on 27.05.18.
//

#include <stddef.h>
#include <util/atomic.h>
#include <stdlib.h>

#include "train.h"
#include "bitstream.h"

Train *first_train = NULL;
Train *last_train = NULL;
Train *curr_train = NULL;

static uint8_t num_trains_active = 0;
int8_t trains_estop = 0;

static BitStream estop_bitstream = { .length = 3, .data = { 0x00, 0x41, 0x41 } };
static BitStream idle_bitstream = { .length = 3, .data = { 0xff, 0x00, 0xff } };



static void train_update_bitstream(Train *train);

Train* train_new(uint8_t addr)
{
    Train *train = malloc(sizeof(Train));
    if (train == NULL) return NULL;

    train_init(train, addr);

    if (last_train) {
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
            train->next = first_train;
            last_train->next = train;
            last_train = train;
        }
    } else {
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
            train->next = train;
            first_train = last_train = train;
            curr_train = first_train;
        }
    }

    return train;
}

void train_init_function_bitstream(Train *train, BitStream *stream, uint8_t data);

Train* train_init(Train *train, uint8_t addr)
{
    train->dcc_mode = DCC_MODE_28;

    train->addr = addr;
    train->f0 = 0;
    train->speed = 0;
    train->direction = TRAIN_FORWARD;
    train->active = 0;

    train->curr_bitstream = 0;
    train->speed_stream.lock = 0;

    train_update_bitstream(train);

    train_init_function_bitstream(train, &train->f00_04_stream, 0x80);
    train_init_function_bitstream(train, &train->f05_08_stream, 0xb0);
    train_init_function_bitstream(train, &train->f09_12_stream, 0xa0);

    return train;
}


void train_init_function_bitstream(Train *train, BitStream *stream, uint8_t data)
{
    stream->length = 3;
    stream->data[0] = train->addr;
    stream->data[1] = data;
    stream->data[2] = train->addr ^ data;
    stream->lock = 0;
}

void train_activate(Train *train)
{
    train->active = 1;
    num_trains_active++;
}


void train_deactivate(Train *train)
{
    num_trains_active--;
    train->active = 0;
}


void train_enable_function(Train *train, uint8_t f)
{
    if (f == 0) {
        train->f0 = 1;
        if (train->dcc_mode == DCC_MODE_14) {
            train_update_bitstream(train);
        }

        train->f00_04_stream.data[1] |= 1 << 4;
        bitstream_update_checksum(&train->f00_04_stream);
    }
    else if (f < 5) {
        train->f00_04_stream.data[1] |= 1 << (f-1);
        bitstream_update_checksum(&train->f00_04_stream);
    }
    else if (f < 9) {
        train->f05_08_stream.data[1] |= 1 << (f-5);
        bitstream_update_checksum(&train->f05_08_stream);
    }
    else if (f < 13) {
        train->f09_12_stream.data[1] |= 1 << (f-9);
        bitstream_update_checksum(&train->f09_12_stream);
    }
}


void train_disable_function(Train *train, uint8_t f)
{
    if (f == 0) {
        train->f0 = 0;
        if (train->dcc_mode == DCC_MODE_14) {
            train_update_bitstream(train);
        }

        train->f00_04_stream.data[1] &= ~(1 << 4);
        bitstream_update_checksum(&train->f00_04_stream);
    }
    else if (f < 5) {
        train->f00_04_stream.data[1] &= ~(1 << (f-1));
        bitstream_update_checksum(&train->f00_04_stream);
    }
    else if (f < 9) {
        train->f05_08_stream.data[1] &= ~(1 << (f-5));
        bitstream_update_checksum(&train->f05_08_stream);
    }
    else if (f < 13) {
        train->f09_12_stream.data[1] &= ~(1 << (f-9));
        bitstream_update_checksum(&train->f09_12_stream);
    }
}


void train_set_dcc_mode(Train *train, uint8_t dcc_mode)
{
    train->dcc_mode = dcc_mode;
    train_set_speed_and_dir(train, train->speed, train->direction);
}


void train_set_speed_and_dir(Train *train, uint8_t speed, uint8_t dir)
{
    if (train->dcc_mode == DCC_MODE_14) {
        if (speed > 14) speed = 14;
    } else {
        if (speed > 28) speed = 28;
    }

    train->speed = speed;
    train->direction = dir;

    train_update_bitstream(train);
}


Train* train_by_addr(uint8_t addr)
{
    Train *train = first_train;

    if (train == NULL) return train_new(addr);

    do {
        if (train->addr == addr) return train;
        train = train->next;

    } while (train != first_train);

    return train_new(addr);
}



void trains_emergency_stop(int8_t stop)
{
    trains_estop = stop;
}


// Läuft im Kontext der ISR
BitStream* train_schedule_next_bitstream(Train *train)
{
    BitStream *next_stream;

try_next:
    switch (train->curr_bitstream) {
        case 0:
            train->curr_bitstream++;
            next_stream = &train->speed_stream;
            break;
        case 1:
            train->curr_bitstream++;
            next_stream = &train->f00_04_stream;
            break;
        case 2:
            train->curr_bitstream++;
            next_stream = &train->f05_08_stream;
            break;
        case 3:
            train->curr_bitstream = 0;
            next_stream = &train->f09_12_stream;
            break;
        default:
            train->curr_bitstream = 0;
            next_stream = &train->speed_stream;
    }
    // Wird Bitstream gerade aktualisiert? Dann einen anderen wählen!
    if (next_stream->lock) goto try_next;

    return next_stream;
}

// Läuft im Kontext der ISR
BitStream* trains_schedule_next_bitstream()
{
    static int odd = 0;
    
    odd ^= 1;   // markiert jedes zweite Datenpaket

    if (trains_estop) return &estop_bitstream;

    // Idle Kommando senden wenn keine Züge aktiv ist. Weiterhin jedes zweite
    // Paket als Idle Kommando senden wenn nur ein Zug aktiv ist, um 5ms
    // Zeitintervall zwischen den Paketen an die gleiche Adresse einzuhalten.
    if (!num_trains_active ||
        (num_trains_active == 1 && odd)) return &idle_bitstream;

    do {
        curr_train = curr_train->next;
    } while (!curr_train->active);

    return train_schedule_next_bitstream(curr_train);
}


static void train_update_bitstream(Train *train)
{
    BitStream *stream = &train->speed_stream;

    stream->length = 3;
    stream->data[0] = train->addr;

    uint8_t speed_and_dir = 0x40;
    if (train->direction) speed_and_dir |= 0x20;

    if (train->dcc_mode == DCC_MODE_14) {
        if (train->speed > 0) {
            speed_and_dir = speed_and_dir |  (train->speed + (uint8_t)1);
        }
        if (train->f0) speed_and_dir |= 0x10;
    } else {
        if (train->speed > 0) {
            uint8_t spd = train->speed + (uint8_t)3;
            speed_and_dir = speed_and_dir | (uint8_t)((spd & 0x1e) >> 1) | (uint8_t)((spd & 1) << 4);
        }
    }

    stream->lock = 1;
    stream->data[1] = speed_and_dir;
    bitstream_update_checksum(stream);
    stream->lock = 0;
}

