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
 *
 */

#ifndef RFM12B_H_
#define RFM12B_H_

#include "Packet.h"
#include "spi.h"
#include "Logger.h"
#include "utils.h"
#include "utilsconsts.h"

enum State {RX, TX};

template <class T> /* T = type of packet for the packet buffer */
class Rfm12b {
public:
	static void enable_rx()
	{
	    // 3. Power Management Command 0x8201
	    // 1 0 0 0   0 0 1 0  er ebb et es ex eb ew dc
	    //                     0   0  0  0  0  0  0  1
	    // er : enable whole receiver chain (automatically turns on crystal,
	    //      synth, baseband and RF front end)
	    // ebb: enable RX baseband circuit (missing on RFM01)
	    // et : enable TX (PLL & PA)
	    // es : enable synthesiser (must be on to enable baseband circuits)
	    // ex : enable crystal oscillator
	    // eb : enable low batt detector
	    // ew : enable wake-up timer
	    // dc : disable clock output of CLK pin
	    //                          eeeeeeed
	    //                          rbtsxbwc
	    state = RX;
	    spi::transfer_word(0x82D9); // enable RX (er=1), disable TX (et=0)
	    spi::transfer_word(0x8058); // disable TX register (el=0), enable RX FIFO register (ef=1)
	    reset_fifo();
	}


	/**
	 * Will wait a short while before TXing if we're already RXing
	 */
	static void enable_tx()
	{
	    if (currently_receiving) {
	        log(DEBUG, PSTR("CRX"));
	        /* If we're currently receiving then wait up to 100ms */
	        const uint32_t deadline = millis() + 100;
	        while (currently_receiving && utils::in_future(deadline))
	            ;
	    }

	    state = TX;
	    spi::transfer_word(0x9830); // 60kHz FSK deviation for first few bytes
	    spi::transfer_word(0x8098); // enable TX register (el=1), disable RX FIFO register (ef=0)
	    spi::transfer_word(0x8279); // disable RX (er=0), enable TX (et=1), leave base band block running
	}


	static void transmit(
	        const byte payload[],
	        const byte& payload_length,
            const bool add_checksum = false)
	{
	    tx_packet.assemble(payload, payload_length, add_checksum);
	    enable_tx();

	    while (!tx_packet.done()); // Wait until we finish transmitting
	}


	/**
	 * Init the RFM12b using commands sniffed from an EcoManager.
	 */
	static void init ()
	{
	    currently_receiving = false;
	    spi::init();

	    spi::transfer_word(0x0000);

	    delay(2000); // give RFM time to start up

	    /***************************
	     * BEGIN RFM12b COMMANDS...
	     ***************************/

	    // 2. configuration setting command 80D8
	    // 1 0 0 0 0 0 0 0 el ef b1 b0 x3 x2 x1 x0
	    //                  1  1  0  1  1  0  0  0
	    // el: enable TX register (same as CC RFM01)
	    // ef: enable RX FIFO register (same as CC RFM01)
	    // b:  select band. 01 = 433MHz (same as CC RFM01)
	    // x:  load capacitor.
	    //      0010 (0x2)=9.5pF  (from EnviR RFM01)
	    //      0111 (0x7)=12.0pF (from jeelib)
	    //      1000 (0x8)=12.5pF (from EDF EcoManager)
	    spi::transfer_word(0x80D8);

	    // 3. Power Management Command 0x8201
	    // 1 0 0 0   0 0 1 0  er ebb et es ex eb ew dc
	    //                     0   0  0  0  0  0  0  1
	    // er : enable whole receiver chain (automatically turns on crystal,
	    //      synth, baseband and RF front end)
	    // ebb: enable RX baseband circuit (missing on RFM01)
	    // et : enable TX (PLL & PA)
	    // es : enable synthesiser (must be on to enable baseband circuits)
	    // ex : enable crystal oscillator
	    // eb : enable low batt detector
	    // ew : enable wake-up timer
	    // dc : disable clock output of CLK pin
	    spi::transfer_word(0x8201);

	    // 4. Frequency setting command
	    // 1 0 1 0 F
	    // F  = 1588 decimal = 0x634 (EnviR RFM01 has 1560 decimal giving command A618)
	    // Fc = 10 x 1 x (43 + F/4000) MHz = 433.97 MHz (EnviR RFM01 uses 433.9 MHz)
	    //spi::transfer_word(0xA634); // EcoManager default
	    //spi::transfer_word(0xA62C); // <- works well for TXs (433.95MHz, F=1580 dec)
	    spi::transfer_word(0xA62F); // 433.9575 F=1583 <-- spot-on for TRXs

	    // 5. Data Rate command
	    // 1 1 0 0   0 1 1 0   cs  r...
	    // r  = 0b1010111 = 87 decimal
	    // cs = 0
	    // BitRate = 10000 / 29 / (R+1) = 3.918 kbps (same as CC RFM01)
	    spi::transfer_word(0xC657);

	    // 6 Receiver control command 94C0
	    // 1 0 0 1 0 P16 d1 d0 i2 i1 i0 g1 g0 r2 r1 r0
	    //             1  0  0  1  1  0  0  0  0  0  0
	    // p16: function of pin16. 1 = VDI output (same as CC RFM01)
	    // d: VDI response time. 00=fast, 01=med, 10=slow, 11=always on (same as CC RFM01)
	    // i: baseband bandwidth. 110=67kHz (same as CC RFM01)
	    // g: LNA gain. 00=0dB. (same as CC RFM01)
	    // r: RSSI detector threshold. 000 = -103 dBm (same as CC RFM01)
	    spi::transfer_word(0x94C0);

	    // 7. Digital filter command C22C
	    // 1 1 0 0 0 0 1 0 al ml 1 s 1 f2 f1 f0
	    //                  0  0 1 0 1  1  0  0
	    // al: clock recovery (CR) auto lock control (same as CC RFM01)
	    //     1=auto, 0=manual (set by ml).
	    //     CCRFM01=0.
	    // ml: enable clock recovery fast mode. CCRFM01=1 (diff to CC RFM01)
	    // s :  data filter. 0=digital filter. (default & CCRFM01)=0 (same as CC RFM01)
	    // f : DQD threshold. CCRFM01=2; but RFM12b manual recommends >4 (diff to CC RFM01)
	    //spi::transfer_word(0xC22C); // EDF EcoManager default
	    //spi::transfer_word(0xC22A); // DQD = 2
	    //spi::transfer_word(0xC26A); // DQD = 2, ML=1 (enabling fast clock recovery gives a bit improvement for receiving CC TXs)
	    spi::transfer_word(0xC2EA); // DQD = 2, AL=1 (auto clock recovery, ML has no effect)

	    // 8. FIFO and Reset Mode Command (CA81)
	    // 1 1 0 0 1 0 1 0 f3 f2 f1 f0 sp al ff dr
	    //                  1  0  0  0  0  0  0  1
	    // f: FIFO interrupt level = 8 (RFM01 & default)
	    // sp: length of synchron pattern (not on RFM01!!!)
	    //     0 = 2 bytes (first = 2D, second is configurable defaulting to D4)
	    // al: FIFO fill start condition. Default = sync-word.
	    //     0=synchron pattern
	    //     1=always fill
	    // ff: enable FIFO fill
	    // dr: disable hi sensitivity reset mode
	    spi::transfer_word(0xCA81); // from EDF config

	    // 9. Synchron Pattern Command CED4
	    // D4 = default synchron pattern
	    spi::transfer_word(0xCED4);

	    // 11. AFC Command C4F7
	    // 1 1 0 0 0 1 0 0 a1 a0 rl1 rl0 st fi oe en
	    // 1 1 0 0 0 1 0 0  1  1   1   1  0  1  1  1
	    // a:  AFC auto-mode selector (different to CC RFM01)
	    //     10 = keep offset when VDI hi (RFM01 and default)
	    //     11 = Keep the f_offset value independently from the state of the VDI signal (EDF EcoManager)
	    // rl: range limit (different to CC RFM01)
	    //     01 = +15 to -16 (433band: 2.5kHz) (CC RFM01)
	    //     11 =  +3 to  -4 (EDF EcoManager)
	    // st: (different to CC RFM01)
	    //     Strobe edge, when st goes to high, the actual latest
	    //     calculated frequency error is stored into the offset
	    //     register of the AFC block.
	    // fi: Enable AFC hi accuracy mode (same as CC RFM01)
	    //     Switches the circuit to high accuracy (fine) mode.
	    //     In this case, the processing time is about twice as
	    //     long, but the measurement uncertainty is about half.
	    // oe: Enables the frequency offset register. (same as CC RFM01)
	    //     It allows the addition of the offset register
	    //     to the frequency control word of the PLL.
	    // en: Enable AFC function. (Same as CC RFM01)
	    //     Enables the calculation of the
	    //     offset frequency by the AFC circuit.
	    //spi::transfer_word(0xC4F7); // EcoManager default
	    //spi::transfer_word(0xC4B7); // a=10, otherwise EcoManger default
	    //spi::transfer_word(0xC4B7);
	    //spi::transfer_word(0xC497); // a=10, rl=01, en=1
	    spi::transfer_word(0xC4A7); // a=10, rl=+7..-8, en=1

	    // 12. TX Configuration Control Command 0x9820
	    // 1 0 0 1 1 0 0 mp m3 m2 m1 m0 0 p2 p1 p0
	    // 1 0 0 1 1 0 0  0  0  0  1  0 0  0  0  0
	    // mp: FSK modulation parameters (inferred using jeelabs RFM12b calc)
	    //     frequency shift = pos
	    //     deviation       = 45 kHz
	    // p : Output power. 0x000=0dB
	    spi::transfer_word(0x9820);


	    // 13. PLL Setting Command 0xCC66
	    // 1 1 0 0 1 1 0 0 0 ob1 ob0 1 dly ddit 1 bw0
	    // 1 1 0 0 1 1 0 0 0   1   1 0   0    1 1   0
	    // ob: Microcontroller output clock buffer rise and
	    //     fall time control. The ob1-ob0 bits are changing
	    //     the output drive current of the CLK pin. Higher
	    //     current provides faster rise and fall times but
	    //     can cause interference.
	    //     11 = 5 or 10MHz (recommended)
	    // dly: switches on the delay in the phase detector
	    // ddit: disables dithering in the PLL loop
	    // bw0: PLL bandwidth set fo optimal TX RM performance
	    //      0 = Max bit rate = 86.2kbps
	    spi::transfer_word(0xCC66);

	    // 15. Wake-up timer command
	    // R=0, M=0 (i.e. Twakeup = 0ms, the minimum)
	    spi::transfer_word(0xE000);

	    // 16. Low Duty-Cycle   disabled
	    spi::transfer_word(0xC800);

	    // 17. Low Batt Detector and Microcontroller Clock Div
	    // Low batt threshold = 2.2V (lowest poss)
	    // Clock pin freq = 1.0Mhz (lowest poss)
	    spi::transfer_word(0xC000);

	    log(INFO, PSTR("attaching interrupt"));
	    delay(500);
	    attachInterrupt(0, interrupt_handler, LOW);
	    return;
	}

    static PacketBuffer<T> rx_packet_buffer;

#if 0
	/**
	 * Poll a CurrentCost transceiver (TRX), e.g. an EDF Wireless Transmitter Plug,
	 * to ask for the latest wattage reading.
	 */
	static void poll_cc_trx(const id_t& id);

	/**
	 * Send acknowledgement to complete pairing.
	 */
	static void ack_cc_trx(const id_t& id);

	/**
	 * Turn TRX on or off.
	 */
	static void change_trx_state(const id_t& id, const bool state);

	/**
	 * Blocks until finished TX.
	 */
    static void send_command_to_trx(const byte& cmd1, const byte& cmd2, const id_t& id);
#endif

private:
	static State state; // state RFM12b is in
	static volatile bool currently_receiving;
	static TXPacket<> tx_packet; // the packet about to be sent

	/**
	 * Get the next byte in Rfm12b::tx_packet
	 * and send it to the RFM12b for transmission.
	 */
	static void tx_next_byte()
	{
	    if (tx_packet.get_byte_index() == 2) {
	        spi::transfer_word(0x9820); // 45kHz FSK deviation for rest of bytes
	    }

	    const byte out = tx_packet.get_next_byte();
	    spi::transfer_word(0xB800 | out);

	    if (tx_packet.done()) {
	        // we've finished transmitting the packet
	        enable_rx();
	    }
	}


	/**
	 * Reset the RFM12b's FIFO buffer. This is necessary
	 * at the end of each packet to tells the RFM12b to
	 * only fill the buffer again if a sync word is received.
	 */
	static void reset_fifo()
	{
	    // To restart synchron pattern recognition, bit ff should be cleared and set
	    // i.e. packet will start with preable, then synchron pattern, then packet
	    // data, then turn FIFO off and on (I think)

	    // 8. FIFO and Reset Mode Command (CA81)
	    // 1 1 0 0 1 0 1 0 f3 f2 f1 f0 sp al ff dr
	    //                  1  0  0  0  0  0  0  1
	    // f: FIFO interrupt level = 8 (RFM01 & default)
	    // sp: length of synchron pattern (not on RFM01!!!)
	    //     0 = 2 bytes (first = 2D, second is configurable defaulting to D4)
	    // al: FIFO fill start condition. Default = sync-word.
	    //     0=synchron pattern
	    //     1=always fill
	    // ff: enable FIFO fill
	    // dr: disable hi sensitivity reset mode

	    spi::transfer_word(0xCA81); // from EDF config
	    spi::transfer_word(0xCA83); // my attempt to enable FIFO fill
	}


	/**
	 * Called every time the RFM12b fires an interrupt request.
	 * This must handle interrupts associated with both RX and TX.
	 */
	static void interrupt_handler()
	{
	    spi::select(true);
	    const byte status_MSB = spi::transfer_byte(0x00); // get status word MSB
	#ifdef TUNING
	    const byte status_LSB = spi::transfer_byte(0x00); // get status word LSB
	#else
	    spi::transfer_byte(0x00); // get status word LSB
	#endif // TUNING

	    if (state == RX) {
	        bool full = false; // is the buffer full after receiving the byte waiting for us?
	        if ((status_MSB & 0x20) != 0) { // FIFO overflow
	            full  = rx_packet_buffer.append(spi::transfer_byte(0x00)); // get 1st byte of data
	            full |= rx_packet_buffer.append(spi::transfer_byte(0x00));
	        } else  if ((status_MSB & 0x80) != 0) { // FIFO has 8 bits ready
	            full = rx_packet_buffer.append(spi::transfer_byte(0x00)); // get 1st byte of data
	        }
	        spi::select(false);

	        if (full) { // Reached end of packet
	            reset_fifo();
	            currently_receiving = false;
	#ifdef TUNING
	            tuning(status_LSB);
	#endif // TUNING
	        } else {
	            currently_receiving = true;
	        }
	    } else { // state == TX
	        if ((status_MSB & 0x80) != 0) { // TX register ready
	            tx_next_byte();
	        }
	        spi::select(false);
	    }
	}


#ifdef TUNING
	/**
	 * Print tuning offset.
	 */
    static void tuning(const byte& status_LSB)
    {
        int8_t tuning_offset = status_LSB & 0xF;

        // decode two's compliment
        if (status_LSB & 0x10) { // if sign bit == 1 then this is a negative number
            tuning_offset = tuning_offset - 0x10;
        }

        log(INFO, PSTR("Tuning = %d"), tuning_offset);
    }
#endif // TUNING
};

/* The syntax used below was adapted from http://stackoverflow.com/a/5828158/732596 */

template<typename T>
State Rfm12b<T>::state;

template<typename T>
volatile bool Rfm12b<T>::currently_receiving;

template<typename T>
TXPacket<> Rfm12b<T>::tx_packet;

template<typename T>
PacketBuffer<T> Rfm12b<T>::rx_packet_buffer;

#endif /* RFM12B_H_ */
