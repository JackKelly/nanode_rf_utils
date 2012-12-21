/*
 * Packet_test.cpp
 *
 *  Created on: 21 Oct 2012
 *      Author: jack
 */

#include <iostream>
#include "FakeArduino.h"

#include "../Packet.h"
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE PacketTest
#include <boost/test/unit_test.hpp>

void append_array(RXPacket<>& rx_packet,
        const byte data[], const index_t length)
{
    for (index_t i=0; i<length; i++){
        rx_packet.append(data[i]);
    }
}

BOOST_AUTO_TEST_CASE(trxPacket1)
{
    RXPacket<> rx_packet;

    const index_t LENGTH = 12;
    const byte data[] = {
            0x46, 0x55, 0x10, 0x00, 0x01, 0x00, 0x50, 0x53, 0x00, 0x00, 0x4F, 0x9E  };

    append_array(rx_packet, data, LENGTH);
    rx_packet.set_packet_length(LENGTH);

    BOOST_CHECK(rx_packet.is_ok());
}

BOOST_AUTO_TEST_CASE(trxPacket2)
{
    RXPacket<> rx_packet;

    const index_t LENGTH = 12;
    const byte data[] = {
            0x52, 0x55, 0x10, 0x00, 0x01, 0x00, 0x41, 0x4B, 0x3E, 0x00, 0x53, 0xD5  };

    append_array(rx_packet, data, LENGTH);
    rx_packet.set_packet_length(LENGTH);

    BOOST_CHECK(rx_packet.is_ok());
}

BOOST_AUTO_TEST_CASE(trxPacket3_chk_fail)
{
    RXPacket<> rx_packet;

    const index_t LENGTH = 12;
    const byte data[] = {
            0x52, 0x55, 0x10, 0x00, 0x01, 0x00, 0x41, 0x4B, 0x3E, 0x00, 0x53, 0xD6  };

    append_array(rx_packet, data, LENGTH);
    rx_packet.set_packet_length(LENGTH);

    BOOST_CHECK(!rx_packet.is_ok());
}

