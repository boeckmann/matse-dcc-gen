//
// Created by bernd on 27.05.18.
//

#ifndef RAIL03_BITSTREAM_H
#define RAIL03_BITSTREAM_H

#include <stdint.h>

typedef struct {
    uint8_t length;
    uint8_t data[8];
    volatile uint8_t active;
} BitStream;

void bitstream_update_checksum(BitStream *stream);

#endif //RAIL03_BITSTREAM_H
