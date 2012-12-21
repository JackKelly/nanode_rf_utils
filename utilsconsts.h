/*
 * utilsconsts.h
 *
 *  Created on: 12 Dec 2012
 *      Author: jack
 */

#ifndef UTILSCONSTS_H_
#define UTILSCONSTS_H_

#include <stdint.h>

const uint32_t UINT32_INVALID = 0xFFFFFFFF;
typedef uint32_t millis_t; /* type for storing times in milliseconds */
typedef uint8_t  index_t;  /* type for storing indices */

#define INDEX_MAX 255

/* Values denoting invalid status */

const index_t INVALID_INDEX   = 0xFF;

const uint8_t PACKET_BUF_LENGTH = 5;

#endif /* UTILSCONSTS_H_ */
