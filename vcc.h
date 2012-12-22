/*
 * vcc.h
 *
 *  Created on: 22 Dec 2012
 *      Author: jack
 *
 * Functions for measuring the voltage supplied to the ATmega
 */

#ifndef VCC_H_
#define VCC_H_

/**
 * @return Calibration parameter based on VCC
 */
float get_vcc_ratio();

/**
 * @return VCC in volts
 */
float get_vcc();

#endif /* VCC_H_ */
