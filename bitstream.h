#ifndef BITSTREAM_H
#define BITSTREAM_H

#include <stdint.h>

typedef struct {
    uint8_t length;
    uint8_t data[8];
    volatile uint8_t lock;
} BitStream;

void bitstream_update_checksum( BitStream *stream );

#endif // BITSTREAM_H
