#ifndef PACKET_H
#define PACKET_H

#include "train.h"
#include <stdint.h>

enum {
    PACKET_NONE,
    PACKET_RESET,
    PACKET_IDLE,
    PACKET_ESTOP,
    PACKET_ESTOP128,
    PACKET_TRAIN,
};

typedef struct {
    uint8_t length;
    uint8_t data[8];
} Packet;

extern volatile uint8_t num_addresses_active;

void packet_request( uint8_t packet, uint8_t repeat, Train *train );

Packet *schedule_next_packet();
void activate_emergency_stop( int8_t stop );

#endif // PACKET_H
