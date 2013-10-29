#define VERSION			"0.1"

#ifdef DJGPP
typedef long int32_t;
typedef short int16_t;
typedef char int8_t;
typedef unsigned long uint32_t;
typedef unsigned short uint16_t;
typedef unsigned char uint8_t;
#else
#include <stdint.h>
#endif

/* Initial value for the iterative XOR scrambler/descrambler */
#define SCRAMBLE_MAGIC		(0x12345678 + (0xef0a & -0x2))

/* First dword of the descrambled block should be this */
#define UNSCRAMBLED_HEADER	((uint32_t) 0x11)

/* This is the result of scrambling UNSCRAMBLED_HEADER,
 * the scrambled block should begin with this */
#define SCRAMBLED_HEADER	((uint32_t) 0x8B264593)

#define BLOCK0_HEADER		"==== BLOCK 0 ===="
#define BLOCK1_HEADER		"==== BLOCK 1 ===="
#define BLOCK2_HEADER		"==== BLOCK 2 ===="
#define BLOCK3_HEADER		"==== BLOCK 3 ===="
#define BLOCK4_HEADER		"==== BLOCK 4 ===="
#define BLOCK5_HEADER		"==== BLOCK 5 ===="

#define BLOCK0_SIZE		0x984a
#define BLOCK1_SIZE		0x100	/* XXX: is this really constant? */
#define BLOCK2_SIZE		0x40
#define BLOCK3_SIZE		0x556a
#define BLOCK4_SIZE		0x51


