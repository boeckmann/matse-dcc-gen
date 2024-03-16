#ifndef TRAIN_H
#define TRAIN_H

enum {
    DCC_MODE_14,
    DCC_MODE_28,
    DCC_MODE_128
};

enum {
    TRAIN_BACKWARD,
    TRAIN_FORWARD
};

#include "bitstream.h"

typedef struct Train {
    uint8_t dcc_mode;
    uint16_t addr;
    uint8_t speed;
    uint8_t direction;
    uint8_t functions[9]; // Functions 0-68

    uint8_t active; // signal for train active
    uint8_t curr_bitstream;

    BitStream speed_stream;
    BitStream f00_04_stream;
    BitStream f05_08_stream;
    BitStream f09_12_stream;

    struct Train *next;
} Train;

Train *train_new( uint16_t addr );
Train *train_init( Train *train, uint16_t addr );

void train_activate( Train *train );
void train_deactivate( Train *train );

void train_set_dcc_mode( Train *train, uint8_t dcc_mode );
void train_set_speed_and_dir( Train *train, uint8_t speed, uint8_t dir );

void train_enable_function( Train *train, uint8_t f );
void train_disable_function( Train *train, uint8_t f );

Train *train_by_addr( uint16_t addr );

void trains_emergency_stop( int8_t stop );

BitStream *trains_schedule_next_bitstream();

#endif // TRAIN_H
