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

#ifndef PACKET_H_
#define PACKET_H_

#ifdef TESTING
#include "tests/FakeArduino.h"
#else
#include <Arduino.h>
#endif

#include "Logger.h"
#include "utilsconsts.h"

/**
 * Simple base class for representing a packet of consecutive bytes.
 */
template<uint8_t MAX_PACKET_LENGTH=22>
class Packet {
public:
    /**********************************************
     * Packet
     **********************************************/
    Packet(): length(MAX_PACKET_LENGTH), byte_index(0) {}

	virtual ~Packet() {};

	void set_packet_length(const byte& _packet_length)
	{
	    length = _packet_length;
	}

	void append(const byte& value)
	{
	    if (!done()) {
	        packet[byte_index++] = value;
	    }
	}

	void append(const byte bytes[], const byte& length)
	{
	    for (int i=0; i<length; i++) {
	        Packet::append(bytes[i]);
	    }
	}

	/**
	 * Print contents of packet to Serial port.
	 * TODO: is this still needed?
	 */
	void print_bytes() const
	{

	    for (int i=0; i<length; i++) {
	        Serial.print(packet[i], HEX);
	        Serial.print(F(" "));
	    }

	    Serial.println(F(""));
	}

	/**
	 * Reset the byte_index to point to the first byte in this packet.
	 */
	virtual void reset() { byte_index = 0; }

	/**
	 * Returns true if we've reached the end of the packet.
	 * TODO: can this be made private?
	 */
	bool done() const
	{
	    return byte_index >= length;
	}

	// TODO: can this be made private?
	const volatile byte& get_byte_index() const	{ return byte_index; }


#ifdef TESTING
    const volatile index_t& get_length() const { return length; }
    const volatile byte* get_packet() const { return packet; }
#endif


protected:
	/* Longest EDF IAM packet = 6B preamble + 2B sync + 12B EDF IAM + 2B tail
	 *  If you want to mimick CC_TX packets then this needs to be set to 21!  */

	/****************************************************
	 * Member variables used within ISR and outside ISR *
	 ****************************************************/
	volatile byte length; // number of bytes in this packet
	volatile byte byte_index;    // index of next byte to write/read

	/* we can't use new() on the
	 * arduino (not easily, anyway) so let's just have a statically declared
	 * array of length MAX_PACKET_LENGTH. */
	volatile byte packet[MAX_PACKET_LENGTH];

	/********************************************
	 * Private methods                          *
	 ********************************************/

	/**
	 * @returns the modular sum (the checksum algorithm used in the
	 *          EDF EcoManager protocol) given the payload.
	 */
	static byte modular_sum(
			const volatile byte payload[],
			const byte& length)
	{
	    byte acc = 0;
	    for (index_t i=0; i<length; i++) {
	        acc += payload[i]; // deliberately overflow
	    }
	    return acc;
	}


};

/**********************************************
 * RxPacket
 **********************************************/

template<uint8_t MAX_PACKET_LENGTH=22>
class RxPacket : public Packet<MAX_PACKET_LENGTH>
{
public:
    using Packet<MAX_PACKET_LENGTH>::byte_index;
    using Packet<MAX_PACKET_LENGTH>::done;
    using Packet<MAX_PACKET_LENGTH>::packet;
    using Packet<MAX_PACKET_LENGTH>::length;
    using Packet<MAX_PACKET_LENGTH>::modular_sum;

	RxPacket(): Packet<MAX_PACKET_LENGTH>(), timecode(0), health(NOT_CHECKED) {}

	virtual ~RxPacket() {}

	void append(const byte& value) // override
	{
	    if (!done()) {
	        if (byte_index==0) { // first byte
	            timecode = millis(); // record timecode that first byte received
	            handle_first_byte(value);
	        }
	        packet[byte_index++] = value;
	    }
	}

	bool is_ok()
	{
	    if (health == NOT_CHECKED) {
	        post_process();
	    }

	    return (health == OK);
	}

    volatile const uint32_t& get_timecode() const { return timecode; }


	void reset()
	{
	    byte_index = 0;
	    health = NOT_CHECKED;
	}

protected:
	/****************************************************
	 * Member variables used within ISR and outside ISR *
	 ****************************************************/
	volatile uint32_t timecode;

	/******************************************
	 * Member variables never used within ISR *
	 ******************************************/
	enum Health {NOT_CHECKED, OK, BAD} health; // does the checksum or de-manchesterisation check out?

	/********************************************
	 * Private methods                          *
	 ********************************************/

	virtual void handle_first_byte(const byte& first_byte) {}

    /**
     * Run this after packet has been received fully to
     * demanchesterise (if from TX), set health, watts and id.
     */
    virtual void post_process()
    {
        health = verify_checksum();
    }

	/**
	 * @ return true if checksum in packet matches calculated checksum
	 */
	Health verify_checksum() const
	{
	    const byte calculated_checksum = modular_sum(packet, length-1);
	    return (calculated_checksum == packet[length-1]) ? OK : BAD;
	}


};

/**********************************************
 * TXPacket
 **********************************************/

template<uint8_t MAX_PACKET_LENGTH=22>
class TxPacket : public Packet<MAX_PACKET_LENGTH>
{
public:
    using Packet<MAX_PACKET_LENGTH>::reset;
    using Packet<MAX_PACKET_LENGTH>::append;
    using Packet<MAX_PACKET_LENGTH>::modular_sum;
    using Packet<MAX_PACKET_LENGTH>::set_packet_length;
    using Packet<MAX_PACKET_LENGTH>::byte_index;
    using Packet<MAX_PACKET_LENGTH>::done;
    using Packet<MAX_PACKET_LENGTH>::packet;

	/**
	 * Assemble a packet from the following components (in order):
	 *   1. preamble
	 *   2. sync word
	 *   3. payload
	 *   4. (optional) checksum
	 *   5. tail
	 */
	void assemble(const byte payload[], const byte& payload_length,
			const bool add_checksum = false)
	{
	    // We can get away with a shorter header of just 0x55, 0x2D, 0xD4
	    // but using a long preamble seems to improve reliability.
	    const byte HEADER[] = {
	            0xCC, // Low effective bitrate to help RX lock on
	            0xCC,
	            0xCC,
	            0xAA, // Preamble (to allow RX to lock on).
	            0xAA,
	            0xAA,
	            0x2D, // Synchron byte 0
	            0xD4  // Synchron byte 1
	    };
	    const byte TAIL[] = {0x40, 0x00}; // Reducing this to a single byte produces very unreliable comms

	    const byte HEADER_LENGTH = sizeof(HEADER);
	    const byte TAIL_LENGTH   = sizeof(TAIL);

	    reset();

	    append(HEADER, HEADER_LENGTH);

	    append(payload, payload_length);
	    if (add_checksum) {
	        append(modular_sum(payload, payload_length));
	    }
	    append(TAIL, TAIL_LENGTH);

	    set_packet_length(HEADER_LENGTH + payload_length + add_checksum + TAIL_LENGTH);

	    byte_index = 0;
	}

	byte get_next_byte()
	{
	    if (done()) {
	        return 0;
	    } else {
	        return packet[byte_index++];
	    }
	}

};


/**
 * Class for storing multiple packets.  We need this because
 * multiple packets might arrive before we have a chance to
 * read these packets over the FTDI serial port.
 */
template <class T>
class PacketBuffer {
public:

	PacketBuffer() : current_packet(0) {}

	/****************************************
	 * FUNCTIONS WHICH MAY BE CALLED FROM AN
	 * INTERRUPT HANDLER
	 * **************************************/

	/**
	 * @returns true if packet is complete AFTER appending value to it.
	 */
	bool append(const byte& value)
	{
	    packets[current_packet].append(value);

	    if (packets[current_packet].done()) {
	        bool successfully_found_empty_slot = false;
	        for (byte i=0; i<PACKET_BUF_LENGTH; i++) { // find empty slot
	            if (!packets[i].done()) {
	                current_packet = i;
	                successfully_found_empty_slot = true;
	                break;
	            }
	        }
	        if (!successfully_found_empty_slot) {
	            log(ERROR, PSTR("NO MORE BUFFERS!"));
	        }

	        return true;
	    } else {
	        return false;
	    }
	}


	byte current_packet;
	T packets[PACKET_BUF_LENGTH];
};


#endif /* PACKET_H_ */
