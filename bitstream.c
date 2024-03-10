#include "bitstream.h"

void bitstream_update_checksum( BitStream *stream )
{
    uint8_t i = 0;
    uint8_t checksum = 0;

    for ( ; i < stream->length - 1; i++ ) {
        checksum ^= stream->data[i];
    }

    stream->data[stream->length - 1] = checksum;
}
