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
}
