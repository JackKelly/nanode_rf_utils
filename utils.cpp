#ifdef TESTING
#include "tests/FakeArduino.h"
#include <cstdlib> // for strtoul
#else
#include <Arduino.h>
#endif // TESTING

#include "utils.h"
#include "Logger.h"

void utils::read_cstring_from_serial(char* str, const byte& length)
{
    byte i = 0;
    str[0] = '\0';

    char ch = '\0';

    const uint32_t end_time = millis() + KEYPRESS_TIMEOUT;

    do {
        if (Serial.available()) {
            ch = Serial.read();
            if (ch == 0x7F) { /* backspace ASCII char */
                if (i>0) {
                    i--;
                    Serial.write(8); /* backspace */
                    Serial.print(F(" ")); /* replace character with a space */
                    Serial.write(8); /* backspace */
                }
            } else {
                str[i++] = ch;
                Serial.print(str[i-1]); // echo
                if (i == length-1) break;
            }
        }
    } while (ch != '\r' && in_future(end_time));

    Serial.println(F(""));
    Serial.flush();
    str[i] = '\0';
}


uint32_t utils::read_uint32_from_serial()
{
    const byte BUFF_LENGTH = 15; /* 10 chars + 1 sentinel char + extras for whitespace.
                                     * The max value a uint32 can store is
                                     * 4 billion (10 chars decimal). */
    char buff[BUFF_LENGTH];
    read_cstring_from_serial(buff, BUFF_LENGTH);

    if (buff[0] == '\0') {
        log(INFO, PSTR("timeout"));
        return UINT32_INVALID;
    }

    return strtoul(buff, NULL, 0);
}


bool utils::in_future(const uint32_t& deadline)
{
    const uint32_t PUSH_FORWARD = 100000;

    if (millis() < deadline)
        return true;
    else if ((millis() + PUSH_FORWARD) < (deadline + PUSH_FORWARD))
        /* Try pushing both millis and deadline forward so they both roll over */
        return true;
    else
        return false;
}


void utils::uint_to_bytes(const uint16_t& input, byte* output)
{
    output[0] = input >> 8; // MSB
    output[1] = input & 0x00FF; // LSB
}


void utils::uint_to_bytes(const uint32_t& input, byte* output)
{
    uint_to_bytes(uint16_t(input >> 16), output);
    uint_to_bytes(uint16_t(input & 0x0000FFFF), output+2);
}

uint16_t utils::bytes_to_uint16(const byte* input)
{
    uint16_t output;
    output = input[0];
    output <<= 8;
    output |= input[1];
    return output;
}

uint32_t utils::bytes_to_uint32(const volatile byte* input)
{
    uint32_t output = input[0];
    for (uint8_t i=1; i<4; i++) {
        output <<= 8;
        output |= input[i];
    }
    return output;
}
