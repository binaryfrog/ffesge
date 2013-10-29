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
 * Decompresses groups of {0, N} in SRC to N + 1 zeros in DEST.
 * Returns the number of bytes written to DEST.
 */
int expand_zeros(char *dest, const char *src, int length)
{
	const char *src_ptr = src;

	while(length-- > 0) {
		*dest++ = *src_ptr++;

		if(dest[-1] == 0) {
			unsigned char chr = *src_ptr++;
			chr = (chr > length ? length : chr);
			length -= chr;
			while(chr-- > 0)
				*dest++ = 0;
		}
	}

	return src_ptr - src;
}

/*
 * Decodes SIZE bytes starting at DATA using iterative XOR.
 * Returns CRC value.
 */
uint32_t unscramble(int16_t *data, int size)
{
	uint32_t magic = SCRAMBLE_MAGIC;

	size /= 2;

	while(size-- > 0) {
		int32_t tmp;
		*data ^= magic & -0x1;
		tmp = *data;
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
	void *data_start = NULL, *data;
	int32_t data_size = 0;
	int8_t padding = 0;

	printf("unmangle %s\n", VERSION);

	if(argc != 3) {
		fprintf(stderr, "Usage: unmangle FILE1 FILE2\n\n");
		fprintf(stderr, "Unmangles file FILE1, writing to FILE2\n");
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

	block5_size = -1;

	while(!feof(input)) {
		uint32_t scrambled_header, crc;

		fread(&scrambled_header, 4, 1, input);
		
		if(scrambled_header == SCRAMBLED_HEADER) {
			fseek(input, -4, SEEK_CUR);
			block5_size = ftell(input);

			data_size = input_size - block5_size;
			data_start = malloc(data_size);
			fread(data_start, data_size, 1, input);

			data_size -= 4;
			crc = *(uint32_t *) (data_start + data_size);

			if(unscramble((int16_t *) data_start, data_size) != crc) {
				free(data_start);
				data_start = NULL;
				block5_size = -1;
			} else
				break;
		} else
			fseek(input, -3, SEEK_CUR);
	}

	if(data_start == NULL) {
		fprintf(stderr, "Failed to find scrambled header in `%s'.\n"
			"Is this really a saved game?\n", argv[1]);
		fclose(input);
		return 1;
	}

	block5 = malloc(block5_size);
	fseek(input, 0, SEEK_SET);
	fread(block5, block5_size, 1, input);
	fclose(input);

	data = data_start + 4;

	data += expand_zeros(block0, (char *) data, BLOCK0_SIZE);

	memcpy(block1, data, BLOCK1_SIZE);
	data += BLOCK1_SIZE;

	memcpy(block2, data, BLOCK2_SIZE);
	data += BLOCK2_SIZE;
	
	data += expand_zeros(block3, (char *) data, BLOCK3_SIZE);

	data += expand_zeros(block4, (char *) data, BLOCK4_SIZE);

	if(data - data_start != data_size)
		padding = *(char *) data++;

	if(data - data_start != data_size) {
		fprintf(stderr, "Eeek. %ld unused bytes in scrambled section "
			"of `%s'\n", (long) data_size - (data - data_start),
			argv[1]);
		free(block5);
		return 1;
	}

#ifdef DJGPP
	/* Why doesn't "b" on fopen work? */
	_fmode = O_BINARY;
#endif

	if((output = fopen(argv[2], "wb")) == NULL) {
		fprintf(stderr, "Failed to open `%s' for writing: %s\n",
			argv[2], strerror(errno));
		free(block5);
		return 1;
	}

	fwrite(BLOCK0_HEADER, strlen(BLOCK0_HEADER), 1, output);
	fwrite(block0, BLOCK0_SIZE, 1, output);

	fwrite(BLOCK1_HEADER, strlen(BLOCK1_HEADER), 1, output);
	fwrite(block1, BLOCK1_SIZE, 1, output);

	fwrite(BLOCK2_HEADER, strlen(BLOCK2_HEADER), 1, output);
	fwrite(block2, BLOCK2_SIZE, 1, output);

	fwrite(BLOCK3_HEADER, strlen(BLOCK3_HEADER), 1, output);
	fwrite(block3, BLOCK3_SIZE, 1, output);

	fwrite(BLOCK4_HEADER, strlen(BLOCK4_HEADER), 1, output);
	fwrite(block4, BLOCK4_SIZE, 1, output);

	fwrite(&padding, 1, 1, output);

	fwrite(BLOCK5_HEADER, strlen(BLOCK5_HEADER), 1, output);
	fwrite(block5, block5_size, 1, output);

	fclose(output);

	free(block5);

	return 0;
}

