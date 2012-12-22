/*
 * vcc.cpp
 *
 *  Created on: 22 Dec 2012
 *      Author: jack
 *
 * Functions for measuring the voltage supplied to the ATmega
 */

#include <Arduino.h>
#include "vcc.h"

#ifdef TESTING
/* Dummy function for use when unit testing on a PC. */
float get_vcc_ratio() { return 3.3 / 1024; }
#else // NOT TESTING

/*
 * Adapted from EmonLib EnergyMonitor::readVcc()
 *
 * Thanks to http://hacking.majenko.co.uk/making-accurate-adc-readings-on-arduino
 * and Jérôme who alerted us to
 * http://provideyourown.com/2012/secret-arduino-voltmeter-measure-battery-voltage/
 */
float get_vcc_ratio() {
    // Read 1.1V reference against AVcc
    ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
    delay(2); // Wait for Vref to settle
    ADCSRA |= _BV(ADSC); // Convert
    while (bit_is_set(ADCSRA,ADSC))
        ; // measuring...

    uint16_t result = ADCL; // read least significant byte
    result |= ADCH << 8; // read most significant byte

    // 1.1V reference
    return 1.1 / result;
}
#endif // TESTING

float get_vcc() {
    // 1024 ADC steps http://openenergymonitor.org/emon/node/1186
    return get_vcc_ratio() * 1024;
}
