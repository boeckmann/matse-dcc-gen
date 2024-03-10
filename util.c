#include "util.h"
#include <ctype.h>
#include <stdint.h>

uint8_t str_to_uint8( const char **digits )
{
    return (uint8_t)str_to_uint16( digits );
}

uint16_t str_to_uint16( const char **digits )
{
    uint16_t v = 0;
    const char *d = *digits;

    while ( isdigit( *d ) ) {
        v = v * 10 + *( d++ ) - '0';
    }
    *digits = d;

    return v;
}
