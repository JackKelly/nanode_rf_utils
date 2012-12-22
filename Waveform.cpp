/*
 * Waveform.cpp
 *
 *  Created on: 22 Dec 2012
 *      Author: jack
 */

#ifdef TESTING
#include <tests/FakeArduino.h>
#else
#include <Arduino.h>
#endif // TESTING

#include "vcc.h"
#include "Waveform.h"
#include "utils.h"

Waveform::Waveform(float _calibration, byte _pin, float _phasecal)
: calibration(_calibration), sum(0), phasecal(_phasecal), num_samples(0),
  num_consecutive_equal(0), pin(_pin)
{
    for (uint8_t i=0; i<3; i++) {
        zero_crossing_times[i] = 0;
    }
    // prime();
}


void Waveform::prime()
{
    Serial.print(F("priming LTAD..."));
    sum = 0;
    num_samples = 0;

    /* Use average of 1 second worth of samples to "boot strap" the LTAD algorithm */
    zero_offset = calc_zero_offset();

    /* Figure out which lobe we're in at the moment */
    sample = analogRead(pin);
    in_positive_lobe = last_sample > zero_offset;

    /* "Dry run" LTAD for half a second to let it stabilise */
    const uint32_t deadline = millis() + 500;
    while (utils::in_future(deadline)) {
        take_sample();
        process();
    }

    /* Reset counters and accumulators after dry run */
    reset();

    Serial.println(F(" done priming."));
}


void Waveform::take_sample()
{
    last_sample = sample;
    sample = analogRead(pin);
    num_samples++;
}


void Waveform::process()
{
    if (just_crossed_zero()) {
        update_zero_offset();
    }

    /* If we're consuming no current then the CT clamp's
     * zero offset might drift but, because we're consuming no
     * current, we never cross zero so update_zero_offset()
     * will not be called to correct zero_offset.  So we need
     * to explicitly check if we're consuming no current by
     * checking if multiple consecutive raw samples have the
     * same value.  If they do then adjust zero_offset so
     * we correctly report a filtered value of zero. */
    if (sample == last_sample) {
        num_consecutive_equal++;

        if (num_consecutive_equal > 15 &&
            !utils::roughly_equal<float>(zero_offset, (float)sample, 0.01)) {

            zero_offset += (sample - zero_offset);
            num_consecutive_equal = 0;
        }
    } else {
        num_consecutive_equal = 0;
    }

    /* Remove zero offset */
    last_filtered = filtered;
    filtered = sample - zero_offset;

    /* Save samples for printing to serial port later */
    if (num_samples < MAX_NUM_SAMPLES) {
        samples[num_samples] = sample;
    }

    /* Update variables used in RMS calculation */
    sum += (filtered * filtered);

    /* TODO: I should experiment with integer maths
     * http://openenergymonitor.org/emon/node/1629
     * and look into calypso_rae's other ideas:
     * http://openenergymonitor.org/emon/node/841 */
}

const float& Waveform::get_filtered() { return filtered; }

float Waveform::get_phase_shifted()
{
    return last_filtered + phasecal * (filtered - last_filtered);
}

float Waveform::get_rms_and_reset()
{
    const float ratio = calibration * get_vcc_ratio();
    const float rms = ratio * sqrt(sum / num_samples);
    reset();
    return rms;
}

/* Reset counters and accumulators. */
void Waveform::reset()
{
    sum = 0;
    num_samples = 0;
    num_consecutive_equal = 0;
}

const uint16_t& Waveform::get_num_samples() { return num_samples; }

const float& Waveform::get_calibration() { return calibration; }

void Waveform::print_samples()
{
    for (uint16_t i=0; i<MAX_NUM_SAMPLES; i++) {
        Serial.println(samples[i]);
    }
}

const float& Waveform::get_zero_offset() { return zero_offset; }

/**
 * Calculate the zero offset by averaging over 1 second of raw samples.
 */
float Waveform::calc_zero_offset()
{
    const uint8_t SECONDS_TO_SAMPLE = 1;
    const uint32_t deadline = millis() + (1000 * SECONDS_TO_SAMPLE);
    uint32_t accumulator = 0;
    uint16_t num_samples = 0;

    while (utils::in_future(deadline)) {
        num_samples++;
        accumulator += analogRead(pin);
    }

    return float(accumulator) / num_samples;
}

/**
 * Return true if we've just crossed zero_offset.
 * Also sets in_positive_lobe according to the lobe
 * we're just entering.
 */
bool Waveform::just_crossed_zero()
{
    bool just_crossed_zero = false;

    if (last_sample <= zero_offset && sample > zero_offset) {
        just_crossed_zero = true;
        in_positive_lobe = true;
    } else if (last_sample >= zero_offset && sample < zero_offset) {
        just_crossed_zero = true;
        in_positive_lobe = false;
    }

    return just_crossed_zero;
}

/**
 * Call this just after crossing zero to update zero_offset.
 */
void Waveform::update_zero_offset()
{
    zero_crossing_times[2] = micros();

    if (zero_crossing_times[0] && zero_crossing_times[1]) { // check these aren't zero
        uint32_t lobe_durations[2]; // TODO: don't need to recalc penultimate lobe duration every time
        lobe_durations[0] = zero_crossing_times[1] - zero_crossing_times[0]; // penultimate lobe
        lobe_durations[1] = zero_crossing_times[2] - zero_crossing_times[1]; // previous lobe

        if (!utils::roughly_equal<uint32_t>(lobe_durations[0], lobe_durations[1], 10)) {
            if (in_positive_lobe) { // just entered positive lobe
                if (lobe_durations[0] > lobe_durations[1]) { // [0] is +ve, [1] is -ve
                    zero_offset+= 0.1;
                } else {
                    zero_offset-= 0.1;
                }
            } else { // just entered negative lobe
                if (lobe_durations[0] > lobe_durations[1]) { // [0] is -ve, [1] is +ve
                    zero_offset-= 0.1;
                } else {
                    zero_offset+= 0.1;
                }

            }
        }
    }

    /* Shuffle zero_crossing_times back 1 */
    for (uint8_t i=0; i<2; i++) {
        zero_crossing_times[i] = zero_crossing_times[i+1];
    }
}
