#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>

#include "common.h"

char block0[BLOCK0_SIZE];
char block1[BLOCK1_SIZE];
char block2[BLOCK2_SIZE];
char block3[BLOCK3_SIZE];
char block4[BLOCK4_SIZE];
char *block5;

/*
 * Compresses groups of N zeros in SRC to {0, N-1} in DEST.
 * Returns the number of bytes written to DEST.
 */
int compress_zeros(char *dest, const char *src, int length)
{
	int written = 0;

	while(length-- > 0) {
		dest[written++] = *src++;
		
		if(dest[written - 1] == 0) {
			const char *p;
			unsigned char chr = 0;

			for(p = src; p < src + 0xff && p < src + length; p++)
				if(*p != 0)
					break;

			chr = p - src;
			dest[written++] = chr;
			src += chr;
			length -= chr;
		}
	}

	return written;
}


/*
 * Encodes SIZE bytes starting at DATA, using iterative XOR.
 * Returns CRC value.
 */
int32_t scramble(int16_t *data, int size)
{
	int32_t magic = SCRAMBLE_MAGIC;

	size /= 2;

	while(size-- > 0) {
		int16_t tmp;
		tmp = *data;
		*data ^= magic & -0x1;
		magic += magic + tmp * 2;
		data++;
	}

	return magic;
}

int main(int argc, char **argv)
{
	struct stat statbuf;
	FILE *input, *output;
	int32_t input_size, block5_size;
	void *data_start, *data;
	int8_t padding = 0;
	uint32_t crc;

	printf("mangle %s\n", VERSION);

	if(argc != 3) {
		fprintf(stderr, "Usage: mangle FILE1 FILE2\n\n");
		fprintf(stderr, "Mangles FILE1, writing to FILE2\n");
		return 1;
	}

	if(stat(argv[1], &statbuf) < 0) {
		fprintf(stderr, "Failed to stat `%s': %s\n",
			argv[1], strerror(errno));
		return 1;
	}

	input_size = statbuf.st_size;

	if((input = fopen(argv[1], "rb")) == NULL) {
		fprintf(stderr, "Failed to open `%s' for reading: %s\n",
			argv[1], strerror(errno));
		return 1;
	}

	fseek(input, strlen(BLOCK0_HEADER), SEEK_CUR);
	fread(block0, BLOCK0_SIZE, 1, input);

	fseek(input, strlen(BLOCK1_HEADER), SEEK_CUR);
	fread(block1, BLOCK1_SIZE, 1, input);

	fseek(input, strlen(BLOCK2_HEADER), SEEK_CUR);
	fread(block2, BLOCK2_SIZE, 1, input);

	fseek(input, strlen(BLOCK3_HEADER), SEEK_CUR);
	fread(block3, BLOCK3_SIZE, 1, input);

	fseek(input, strlen(BLOCK4_HEADER), SEEK_CUR);
	fread(block4, BLOCK4_SIZE, 1, input);

	fread(&padding, 1, 1, input);

	fseek(input, strlen(BLOCK5_HEADER), SEEK_CUR);
	block5_size = input_size - ftell(input);
	block5 = malloc(block5_size);
	fread(block5, block5_size, 1, input);

	fclose(input);

	data_start = malloc(4 + BLOCK0_SIZE + BLOCK1_SIZE + BLOCK2_SIZE +
			    BLOCK3_SIZE + BLOCK4_SIZE + 4 + 1);

	data = data_start;

	*(uint32_t *) data = UNSCRAMBLED_HEADER;
	data += 4;

	data += compress_zeros((char *) data, block0, BLOCK0_SIZE);

	memcpy(data, block1, BLOCK1_SIZE);
	data += BLOCK1_SIZE;

	memcpy(data, block2, BLOCK2_SIZE);
	data += BLOCK2_SIZE;

	data += compress_zeros((char *) data, block3, BLOCK3_SIZE);

	data += compress_zeros((char *) data, block4, BLOCK4_SIZE);

	if((data - data_start) & 1)
		*(char *) data++ = padding;

	crc = scramble(data_start, data - data_start);
	*(uint32_t *) data = crc;
	data += 4;

#ifdef DJGPP
	/* Why doesn't "b" on fopen work? */
	_fmode = O_BINARY;
#endif

	if((output = fopen(argv[2], "wb")) == NULL) {
		fprintf(stderr, "Failed to open `%s' for writing: %s\n",
			argv[2], strerror(errno));
		return 1;
	}

	fwrite(block5, block5_size, 1, output);
	fwrite(data_start, data - data_start, 1, output);

	fclose(output);

	return 0;
}

