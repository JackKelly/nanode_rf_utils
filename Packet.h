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
class Packet {
public:
	Packet();
	virtual ~Packet();
	void set_packet_length(const byte& _packet_length);
	void append(const byte& value);
	void append(const byte bytes[], const byte& length);

	/**
	 * Print contents of packet to Serial port.
	 * TODO: is this still needed?
	 */
	void print_bytes() const;

	/**
	 * Reset the byte_index to point to the first byte in this packet.
	 */
	virtual void reset();

	/**
	 * Returns true if we've reached the end of the packet.
	 * TODO: can this be made private?
	 */
	bool done() const;

	// TODO: can this be made private?
	const volatile byte& get_byte_index() const;

#ifdef TESTING
	const volatile byte& get_length() const;
	const volatile byte* get_packet() const;
#endif

protected:
	/* Longest packet = 6B preamble + 2B sync + 12B EDF IAM + 2B tail
	 *  If you want to mimick CC_TX packets then this needs to be set to 21!  */
	const static byte MAX_PACKET_LENGTH = 22;

	/****************************************************
	 * Member variables used within ISR and outside ISR *
	 ****************************************************/
	volatile byte length; // number of bytes in this packet
	volatile byte byte_index;    // index of next byte to write/read
	// we can't use new() on the
	// arduino (not easily, anyway) so let's just have a statically declared
	// array of length MAX_PACKET_LENGTH.
	volatile byte packet[MAX_PACKET_LENGTH];

	/********************************************
	 * Private methods                          *
	 ********************************************/
	/**
	 * @returns the modular sum (the checksum algorithm used in the
	 *           EDF EcoManager protocol) given the payload.
	 */
	static byte modular_sum(
			const volatile byte payload[],
			const byte& length);

};

class RXPacket : public Packet
{
public:
	RXPacket();

	void append(const byte& value); // override
	bool is_ok();
    volatile const uint32_t& get_timecode() const;

	void reset();

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

	virtual void handle_first_byte(const byte& first_byte);

    /**
     * Run this after packet has been received fully to
     * demanchesterise (if from TX), set health, watts and id.
     */
    virtual void post_process();

	/**
	 * @ return true if checksum in packet matches calculated checksum
	 */
	Health verify_checksum() const;

};

class TXPacket : public Packet
{
public:

	/**
	 * Assemble a packet from the following components (in order):
	 *   1. preamble
	 *   2. sync word
	 *   3. payload
	 *   4. (optional) checksum
	 *   5. tail
	 */
	void assemble(const byte payload[], const byte& payload_length,
			const bool add_checksum = false);

	byte get_next_byte();

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
