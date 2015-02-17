/**************************************************************************
 *
 * GUARDTIME CONFIDENTIAL
 *
 * Copyright (C) [2015] Guardtime, Inc
 * All Rights Reserved
 *
 * NOTICE:  All information contained herein is, and remains, the
 * property of Guardtime Inc and its suppliers, if any.
 * The intellectual and technical concepts contained herein are
 * proprietary to Guardtime Inc and its suppliers and may be
 * covered by U.S. and Foreign Patents and patents in process,
 * and are protected by trade secret or copyright law.
 * Dissemination of this information or reproduction of this
 * material is strictly forbidden unless prior written permission
 * is obtained from Guardtime Inc.
 * "Guardtime" and "KSI" are trademarks or registered trademarks of
 * Guardtime Inc.
 */

#include "internal.h"

#include <string.h>
#include <assert.h>
#include <ctype.h>

static const char base32EncodeTable[33] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ234567";

static const unsigned char base32NumDecTable[10] = {
	-1, -1, 26, 27, 28, 29, 30, 31, -1, -1
};

static int makeMask(int bit_count)
{
	int i;
	int ret = 0;

	for (i = 0; i < bit_count; i++) {
		ret <<= 1;
		ret |= 1;
	}

	return ret;
}

static void addBits(unsigned char *buf, int *bits_decoded, int bits)
{
	int bits_to_first_byte;
	int shift_count;
	int buf_idx;
	int selected_bits;

	if (bits < 0) {
		return;
	}

	bits_to_first_byte = 8 - *bits_decoded % 8;
	if (bits_to_first_byte > 5) {
		bits_to_first_byte = 5;
	}

	shift_count = 8 - bits_to_first_byte - *bits_decoded % 8;
	buf_idx = *bits_decoded / 8;
	selected_bits = (bits & (makeMask(bits_to_first_byte) <<
				(5 - bits_to_first_byte))) >> (5 - bits_to_first_byte);

	buf[buf_idx] |= selected_bits << shift_count;
	*bits_decoded += bits_to_first_byte;

	if (bits_to_first_byte < 5) {
		int bits_to_second_byte = 5 - bits_to_first_byte;

		shift_count = 8 - bits_to_second_byte;
		buf_idx++;
		selected_bits = bits & ((makeMask(bits_to_second_byte) <<
					(5 - bits_to_second_byte)) >> (5 - bits_to_second_byte));

		buf[buf_idx] |= selected_bits << shift_count;
		*bits_decoded += bits_to_second_byte;
	}
}

int KSI_base32Decode(const char *base32, unsigned char **data, size_t *data_len) {
	int res = KSI_UNKNOWN_ERROR;

	int bits_decoded = 0;
	char c;
	size_t i;
	unsigned char *tmp = NULL;
	size_t base32_len;

	if (base32 == NULL || data == NULL || data_len == NULL) {
		res = KSI_INVALID_ARGUMENT;
		goto cleanup;
	}

	base32_len = strlen(base32);

	tmp = KSI_calloc(base32_len * 5 / 8 + 2, 1);
	if (tmp == NULL) {
		res = KSI_OUT_OF_MEMORY;
		goto cleanup;
	}

	for (i = 0; i < base32_len; i++) {
		c = (char)toupper(base32[i]);

		if (c == '=') {
			break;
		}

		/* The '-' is used for grouping of the base64 encoded string and we may ignore it. */
		if (c == '-') {
			continue;
		}

		if (isdigit(c)) {
			addBits(tmp, &bits_decoded, base32NumDecTable[c - '0']);
			continue;
		}
		/* It is not safe to use isalpha() here: it is locale dependent and
		 * can return true for characters that are invalid for base32 encoding.
		 */
		if (c >= 'A' && c <= 'Z') {
			addBits(tmp, &bits_decoded, c - 'A');
			continue;
		}

		res = KSI_INVALID_FORMAT;
		goto cleanup;
	}

	/* We ignore padding errors. */

	/* This operation also truncates extra bits from the end (when input
	 * bit count was not divisible by 5). */
	*data_len = bits_decoded / 8;
	*data = tmp;
	tmp = NULL;

	res = KSI_OK;

cleanup:

	KSI_free(tmp);
	return res;
}

/* Returns -1 when EOF is encountered. */
static int readNextBits(const unsigned char *data, size_t data_len,
		int bits_read)
{
	int ret = 0;
	size_t first_byte_bits;
	size_t byte_to_read;
	int shift_count;

	byte_to_read = bits_read / 8;

	if (byte_to_read >= data_len) {
		return -1;
	}

	first_byte_bits = 8 - (bits_read - byte_to_read * 8);
	if (first_byte_bits > 5) {
		first_byte_bits = 5;
	}
	shift_count = 8 - bits_read % 8 - (int)first_byte_bits;
	ret = (data[byte_to_read] & (makeMask((int)first_byte_bits) << shift_count)) >>
			shift_count;

	byte_to_read++;
	if (first_byte_bits < 5) {
		int second_byte_bits = 5 - (int)first_byte_bits;
		ret <<= second_byte_bits;

		if (byte_to_read < data_len) {
			shift_count = 8 - second_byte_bits;
			ret |= (data[byte_to_read] & (makeMask(second_byte_bits) <<
						shift_count)) >> shift_count;
		}
	}

	return ret;
}

int KSI_base32Encode(const unsigned char *data, size_t data_len, size_t group_len, char **encoded) {
	int res = KSI_UNKNOWN_ERROR;

	char *tmp = NULL;
	int next_bits;
	size_t bits_read;
	size_t buf_len;
	size_t ret_len = 0;

	if (data == NULL || data_len == 0 || encoded == NULL) {
		res = KSI_INVALID_ARGUMENT;
		goto cleanup;
	}

	buf_len = (data_len * 8 + 39) / 40 * 8;
	if (group_len > 0) {
		buf_len += (buf_len - 1) / group_len;
	}
	++buf_len;

	tmp = KSI_calloc(buf_len,1);
	if (tmp == NULL) {
		res = KSI_OUT_OF_MEMORY;
		goto cleanup;
	}

	for (bits_read = 0;	(next_bits = readNextBits(data, data_len, bits_read)) != -1; bits_read += 5) {
		tmp[ret_len++] = base32EncodeTable[next_bits];

		if (group_len > 0 && ret_len % (group_len + 1) == group_len && bits_read + 5 < data_len * 8) {
			tmp[ret_len++] = '-';
		}
	}

	/* Pad output. */
	while (bits_read % 40 != 0) {
		tmp[ret_len++] = '=';
		if (ret_len % (group_len + 1) == group_len && bits_read % 40 != 35) {
			tmp[ret_len++] = '-';
		}
		bits_read += 5;
	}

	tmp[ret_len++] = '\0';

	*encoded = tmp;
	tmp = NULL;

	res = KSI_OK;

cleanup:

	KSI_free(tmp);
	return res;
}
