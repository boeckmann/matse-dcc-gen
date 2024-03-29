#ifndef STREAM_H
#define STREAM_H

#include "train.h"
#include <stdint.h>

// Anzahl an Paketwiederholungen wenn ich Zugeigenschaften ändern. Mehrmaliges
// sofortiges Senden erhöht die Empfangswahrscheinlichkeit.
#define DEFAULT_STREAM_REPITITIONS 3

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

extern volatile uint8_t repeated_stream_count;
extern uint8_t repeated_stream_type;
extern Train *repeated_stream_train;

void plan_next_stream( void );
Stream *build_next_stream( void );
void activate_emergency_stop( int8_t stop );

#endif // STREAM_H
