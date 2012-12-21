/*
*      Author: Jack Kelly
 *
 * THERE IS NO WARRANTY FOR THE PROGRAM, TO THE EXTENT PERMITTED BY APPLICABLE
 * LAW. EXCEPT WHEN OTHERWISE STATED IN WRITING THE COPYRIGHT HOLDERS AND/OR OTHER
 * PARTIES PROVIDE THE PROGRAM “AS IS” WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESSED OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. THE ENTIRE RISK AS TO THE
 * QUALITY AND PERFORMANCE OF THE PROGRAM IS WITH YOU. SHOULD THE PROGRAM PROVE
 * DEFECTIVE, YOU ASSUME THE COST OF ALL NECESSARY SERVICING, REPAIR OR CORRECTION.
 */

#ifndef UTILS_H
#define UTILS_H

#ifdef TESTING
#include <stdint.h>
typedef uint8_t byte;
#endif

#include "utilsconsts.h"

namespace utils {

const uint32_t KEYPRESS_TIMEOUT = 5000; /* milliseconds */

/**
 * Reads C string from serial port.
 * Ends if length-1 chars are read or if carriage return is received.
 * Always adds sentinel char (so make sure str[] is long enough
 * for your string plus sentinel char).
 * Blocks until '\r' (carriage return) is received.
 */
void read_cstring_from_serial(char* str, const byte& length);

uint32_t read_uint32_from_serial();

/**
 * @returns true if deadline is in the future.
 * Should handle roll-over of millis() correctly.
 */
bool in_future(const uint32_t& deadline);

template<class T>
bool roughly_equal(const T& a, const T& b, const T delta)
{
    return ( b > (a-delta) && b < (a+delta) );
}

void uint_to_bytes(const uint16_t& input, byte* output);

void uint_to_bytes(const uint32_t& input, byte* output);

uint16_t bytes_to_uint16(const byte* input);

uint32_t bytes_to_uint32(const volatile byte* input);

}; // utils namespace

#endif // CC_UTILS_H
