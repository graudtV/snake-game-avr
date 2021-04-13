/* Some type and macro declarations */

#ifndef DECLS_H_
#define DECLS_H_

#include <avr/io.h>

typedef uint8_t byte_t;
typedef int8_t bool_t;

#define true 1
#define false 0
#define NULL ((void *) 0)

#define BIT_SET(val, bitno) ((val) |= (1UL << (bitno)))
#define BIT_CLEAR(val, bitno) ((val) &= ~(1UL << (bitno)))
#define BIT_SET_TO(val, bitno, bitval) ((val) = ((val) & ~(1UL << (bitno))) | (!!(bitval) << (bitno)))

/* Don't use with functions! Will calculates argument twice. */
#define ABS(x) (((x) > 0) ? (x) : -(x))

#define ARR_SZ(arr) (sizeof arr / sizeof arr[0])

#endif // DECLS_H_