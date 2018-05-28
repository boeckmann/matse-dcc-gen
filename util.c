//
// Created by bernd on 28.05.18.
//

#include <stdint.h>
#include "util.h"

uint8_t two_digits_to_num(const char *digits)
{
    return (uint8_t)((digits[0] - '0') * 10 + (digits[1] - '0'));
}

uint8_t three_digits_to_num(const char *digits)
{
    return (uint8_t)((digits[0] - '0') * 100 + (digits[1] - '0') * 10 + (digits[2] - '0'));
}

int8_t has_two_chars(const char *s)
{
    return (*s != 0) && (*(s + 1) != 0);
}

int8_t has_three_chars(const char *s)
{
    return (*s != 0) && (*(s + 1) != 0) && (*(s + 2) != 0);
}

