/*
 * Utils_test.cpp
 *
 *  Created on: 21 Dec 2012
 *      Author: jack
 */

#include "../utils.h"
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE UtilsTest
#include <boost/test/unit_test.hpp>

using namespace utils;

BOOST_AUTO_TEST_CASE(uint16Test)
{
    byte output[2];

    uint_to_bytes(uint16_t(0xFFFF), output);
    BOOST_CHECK_EQUAL(output[0], 0xFF);
    BOOST_CHECK_EQUAL(output[1], 0xFF);

    uint_to_bytes(uint16_t(0xABCD), output);
    BOOST_CHECK_EQUAL(output[0], 0xAB);
    BOOST_CHECK_EQUAL(output[1], 0xCD);

    uint_to_bytes(uint16_t(0x0000), output);
    BOOST_CHECK_EQUAL(output[0], 0x00);
    BOOST_CHECK_EQUAL(output[1], 0x00);

    uint_to_bytes(uint16_t(0xFF00), output);
    BOOST_CHECK_EQUAL(output[0], 0xFF);
    BOOST_CHECK_EQUAL(output[1], 0x00);

    uint_to_bytes(uint16_t(0x00FF), output);
    BOOST_CHECK_EQUAL(output[0], 0x00);
    BOOST_CHECK_EQUAL(output[1], 0xFF);
}

BOOST_AUTO_TEST_CASE(uint32Test)
{
    byte output[4];

    uint_to_bytes(uint32_t(0xFFFFFFFF), output);
    BOOST_CHECK_EQUAL(output[0], 0xFF);
    BOOST_CHECK_EQUAL(output[1], 0xFF);
    BOOST_CHECK_EQUAL(output[2], 0xFF);
    BOOST_CHECK_EQUAL(output[3], 0xFF);

    uint_to_bytes(uint32_t(0xABCDEF01), output);
    BOOST_CHECK_EQUAL(output[0], 0xAB);
    BOOST_CHECK_EQUAL(output[1], 0xCD);
    BOOST_CHECK_EQUAL(output[2], 0xEF);
    BOOST_CHECK_EQUAL(output[3], 0x01);

    uint_to_bytes(uint32_t(0x00000000), output);
    BOOST_CHECK_EQUAL(output[0], 0x00);
    BOOST_CHECK_EQUAL(output[1], 0x00);
    BOOST_CHECK_EQUAL(output[2], 0x00);
    BOOST_CHECK_EQUAL(output[3], 0x00);

    uint_to_bytes(uint32_t(0xFF0000AB), output);
    BOOST_CHECK_EQUAL(output[0], 0xFF);
    BOOST_CHECK_EQUAL(output[1], 0x00);
    BOOST_CHECK_EQUAL(output[2], 0x00);
    BOOST_CHECK_EQUAL(output[3], 0xAB);

    uint_to_bytes(uint32_t(0x00010000), output);
    BOOST_CHECK_EQUAL(output[0], 0x00);
    BOOST_CHECK_EQUAL(output[1], 0x01);
    BOOST_CHECK_EQUAL(output[2], 0x00);
    BOOST_CHECK_EQUAL(output[3], 0x00);
}

BOOST_AUTO_TEST_CASE(bytesToUint16Test)
{
    const byte byte_string1[] = {0x01, 0x02};
    BOOST_CHECK_EQUAL(bytes_to_uint16(byte_string1), 0x0102);

    const byte byte_string2[] = {0x00, 0x00};
    BOOST_CHECK_EQUAL(bytes_to_uint16(byte_string2), 0x0000);

    const byte byte_string3[] = {0xFF, 0x00};
    BOOST_CHECK_EQUAL(bytes_to_uint16(byte_string3), 0xFF00);

    const byte byte_string4[] = {0x00, 0xEF};
    BOOST_CHECK_EQUAL(bytes_to_uint16(byte_string4), 0x00EF);
}

BOOST_AUTO_TEST_CASE(bytesToUint32Test)
{
    const byte byte_string1[] = {0x01, 0x02, 0x03, 0x04};
    BOOST_CHECK_EQUAL(bytes_to_uint32(byte_string1), 0x01020304);

    const byte byte_string2[] = {0x00, 0x00, 0x00, 0x00};
    BOOST_CHECK_EQUAL(bytes_to_uint32(byte_string2), 0x00000000);

    const byte byte_string3[] = {0xFF, 0x00, 0xFF, 0x00};
    BOOST_CHECK_EQUAL(bytes_to_uint32(byte_string3), 0xFF00FF00);

    const byte byte_string4[] = {0x00, 0xEF, 0x00, 0x01};
    BOOST_CHECK_EQUAL(bytes_to_uint32(byte_string4), 0x00EF0001);
}

BOOST_AUTO_TEST_CASE(roughlyEqual)
{
    BOOST_CHECK(roughly_equal<float>(0.1, 0.1, 0.001));
    BOOST_CHECK(roughly_equal<float>(0.1, 0.2, 0.11));
    BOOST_CHECK(roughly_equal<float>(0.2, 0.1, 0.11));
    BOOST_CHECK(!roughly_equal<float>(0.2, 0.1, 0.09));
    BOOST_CHECK(roughly_equal<uint8_t>(10, 11, 1));
    BOOST_CHECK(!roughly_equal<uint8_t>(10, 11, 0));
}
