
#ifdef TESTING
#include "tests/FakeArduino.h"
#else
#include <Arduino.h>
#endif

#include "Logger.h"
#include "Packet.h"
#include "utilsconsts.h"

/**********************************************
 * Packet
 **********************************************/

Packet::Packet()
: length(MAX_PACKET_LENGTH), byte_index(0) {}


Packet::~Packet() {}


void Packet::set_packet_length(const index_t& _packet_length)
{
	length = _packet_length;
}


void Packet::append(const byte bytes[], const index_t& length)
{
	for (int i=0; i<length; i++) {
		Packet::append(bytes[i]);
	}
}


void Packet::append(const byte& value)
{
	if (!done()) {
		packet[byte_index++] = value;
	}
}


void Packet::print_bytes() const
{

	for (int i=0; i<length; i++) {
		Serial.print(packet[i], HEX);
		Serial.print(F(" "));
	}

	Serial.println(F(""));
}


bool Packet::done() const
{
	return byte_index >= length;
}


void Packet::reset() {
	byte_index = 0;
}


byte Packet::modular_sum(
		const volatile byte payload[],
		const byte& length
		)
{
    byte acc = 0;
    for (index_t i=0; i<length; i++) {
    	acc += payload[i]; // deliberately overflow
    }
    return acc;
}


const volatile index_t& Packet::get_byte_index() const
{
	return byte_index;
}


#ifdef TESTING
    const volatile index_t& Packet::get_length() const
    {
        return length;
    }

    const volatile byte* Packet::get_packet() const
    {
        return packet;
    }
#endif


/**********************************************
 * TXPacket
 **********************************************/

byte TXPacket::get_next_byte()
{
	if (done()) {
	    return 0;
	} else {
		return packet[byte_index++];
	}
}


void TXPacket::assemble(
        const byte payload[],
        const byte& payload_length,
		const bool add_checksum)
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

/**********************************************
 * RXPacket
 **********************************************/

RXPacket::RXPacket()
:Packet(), timecode(0), health(NOT_CHECKED) {}


void RXPacket::append(const byte& value)
{
	if (!done()) {
		if (byte_index==0) { // first byte
			timecode = millis(); // record timecode that first byte received
			handle_first_byte(value);
		}
		packet[byte_index++] = value;
	}
}

void RXPacket::handle_first_byte(const byte& first_byte)
{
}

RXPacket::Health RXPacket::verify_checksum() const
{
	const byte calculated_checksum = modular_sum(packet, length-1);
	return (calculated_checksum == packet[length-1]) ? OK : BAD;
}


void RXPacket::reset()
{
    byte_index = 0;
    health = NOT_CHECKED;
}


bool RXPacket::is_ok()
{
    if (health == NOT_CHECKED) {
        post_process();
    }

    return (health == OK);
}


void RXPacket::post_process()
{
    health = verify_checksum();
}


volatile const uint32_t& RXPacket::get_timecode() const
{
	return timecode;
}
