#ifndef BITSTREAM_H
#define BITSTREAM_H

#include <stdint.h>

enum {
    BITSTREAM_F0_4,
    BITSTREAM_F5_8,
    BITSTREAM_F9_12,
    BITSTREAM_F13_16,
    BITSTREAM_F17_20,
    BITSTREAM_F21_24,
    BITSTREAM_F25_28,
    BITSTREAM_SPEED,
    BITSTREAM_IDLE,
    BITSTREAM_ESTOP
};

typedef struct {
    uint8_t length;
    uint8_t data[8];
    volatile uint8_t lock;
} BitStream;

void bitstream_update_checksum( BitStream *stream );

#endif // BITSTREAM_H
