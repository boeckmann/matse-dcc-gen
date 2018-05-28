//
// Created by bernd on 27.05.18.
//

#ifndef RAIL03_TRAIN_H
#define RAIL03_TRAIN_H

#define DCC_MODE_14 0
#define DCC_MODE_28 1

#define TRAIN_FORWARD 1
#define TRAIN_BACKWARD 0


#include "bitstream.h"

typedef struct Train {
    uint8_t dcc_mode;
    uint8_t addr;
    uint8_t speed;
    uint8_t direction;
    uint8_t f0;

    uint8_t active;     // signal for train active
    uint8_t curr_bitstream;

    BitStream speed_stream;
    BitStream f00_04_stream;
    BitStream f05_08_stream;
    BitStream f09_12_stream;

    struct Train *next;
} Train;

Train* train_new(uint8_t addr);
Train* train_init(Train *train, uint8_t addr);

void train_activate(Train *train);
void train_deactivate(Train *train);

void train_set_dcc_mode(Train *train, uint8_t dcc_mode);
void train_set_speed_and_dir(Train *train, uint8_t speed, uint8_t dir);

void train_enable_function(Train *train, uint8_t f);
void train_disable_function(Train *train, uint8_t f);


Train* train_by_addr(uint8_t addr);

void trains_emergency_stop(int8_t stop);

BitStream* trains_schedule_next_bitstream();



#endif //RAIL03_TRAIN_H
