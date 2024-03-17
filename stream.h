#ifndef STREAM_H
#define STREAM_H

#include "train.h"
#include <stdint.h>

enum {
    STREAM_NONE,
    STREAM_RESET,
    STREAM_IDLE,
    STREAM_ESTOP,
    STREAM_ESTOP128,
    STREAM_TRAIN,
};

typedef struct {
    uint8_t length;
    uint8_t data[8];
} Stream;

extern volatile uint8_t num_addresses_active;

Stream *schedule_next_stream();
void activate_emergency_stop( int8_t stop );

#endif // STREAM_H
