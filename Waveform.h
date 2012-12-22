/*
 * Waveform.h
 *
 *  Created on: 22 Dec 2012
 *      Author: jack
 */

#ifndef WAVEFORM_H_
#define WAVEFORM_H_

#include <stdint.h>

/**
 * A class which abstracts the functionality required for sampling and processing
 * current and voltage waveforms.
 */
class Waveform
{
public:
    Waveform(float _calibration, uint8_t _pin, float _phasecal=1);

    /**
     * Call this if we haven't taken a sample for a while.
     * Prime the LTAD algorithm.
     */
    void prime();

    void take_sample();

    void process();

    const float& get_filtered();

    float get_phase_shifted();

    float get_rms_and_reset();

    /* Reset counters and accumulators. */
    void reset();

    const uint16_t& get_num_samples();

    const float& get_calibration();

    void print_samples();

    const float& get_zero_offset();

private:
    float calibration, filtered, last_filtered, zero_offset, sum, phasecal;
    bool in_positive_lobe;
    int sample, last_sample;
    uint16_t num_samples, num_consecutive_equal;
    uint32_t zero_crossing_times[3]; /* [0] is two zero crossings ago, [1] is prev zero crossing */
    uint8_t pin;
    static const uint16_t MAX_NUM_SAMPLES = 50; /* Just used for printing raw samples */
    float samples[MAX_NUM_SAMPLES]; /* Just used for printing raw samples */

    /**
     * Calculate the zero offset by averaging over 1 second of raw samples.
     */
    float calc_zero_offset();

    /**
     * Return true if we've just crossed zero_offset.
     * Also sets in_positive_lobe according to the lobe
     * we're just entering.
     */
    bool just_crossed_zero();

    /**
     * Call this just after crossing zero to update zero_offset.
     */
    void update_zero_offset();
};


#endif /* WAVEFORM_H_ */
