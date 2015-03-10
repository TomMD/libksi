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

#include <assert.h>
#include <string.h>

#include "internal.h"
#include "tlv.h"

struct KSI_OctetString_st {
	KSI_CTX *ctx;
	size_t refCount;
	unsigned char *data;
	unsigned int data_len;
};

struct KSI_Integer_st {
	int staticAlloc;
	size_t refCount;
	KSI_uint64_t value;
};

struct KSI_Utf8String_st {
	KSI_CTX *ctx;
	size_t refCount;
	char *value;
	size_t len;
};

/**
 *  A static pool for immutable #KSI_Integer object values in range 0..f
 */
static KSI_Integer integerPool[] = {
		{1, 0, 0x00}, {1, 0, 0x01}, {1, 0, 0x02}, {1, 0, 0x03},
		{1, 0, 0x04}, {1, 0, 0x05}, {1, 0, 0x06}, {1, 0, 0x07},
		{1, 0, 0x08}, {1, 0, 0x09}, {1, 0, 0x0a}, {1, 0, 0x0b},
		{1, 0, 0x0c}, {1, 0, 0x0d}, {1, 0, 0x0e}, {1, 0, 0x0f}
};


KSI_IMPLEMENT_LIST(KSI_Integer, KSI_Integer_free);
KSI_IMPLEMENT_LIST(KSI_Utf8String, KSI_Utf8String_free);
KSI_IMPLEMENT_LIST(KSI_Utf8StringNZ, KSI_Utf8String_free);
KSI_IMPLEMENT_LIST(KSI_OctetString, KSI_OctetString_free);

/**
 * KSI_OctetString
 */
void KSI_OctetString_free(KSI_OctetString *o) {
	if (o != NULL && --o->refCount == 0) {
		KSI_free(o->data);
		KSI_free(o);
	}
}

int KSI_OctetString_new(KSI_CTX *ctx, const unsigned char *data, unsigned int data_len, KSI_OctetString **o) {
	int res = KSI_UNKNOWN_ERROR;
	KSI_OctetString *tmp = NULL;

	tmp = KSI_new(KSI_OctetString);
	if (tmp == NULL) {
		res = KSI_OUT_OF_MEMORY;
		goto cleanup;
	}

	tmp->ctx = ctx;
	tmp->data = NULL;
	tmp->data_len = data_len;
	tmp->refCount = 1;

	tmp->data = KSI_calloc(data_len, 1);
	if (tmp->data == NULL) {
		res = KSI_OUT_OF_MEMORY;
		goto cleanup;
	}

	memcpy(tmp->data, data, data_len);

	*o = tmp;
	tmp = NULL;
	res = KSI_OK;

cleanup:

	KSI_OctetString_free(tmp);
	return res;
}

int KSI_OctetString_ref(KSI_OctetString *o) {
	if (o != NULL) {
		++o->refCount;
	}
	return KSI_OK;
}

int KSI_OctetString_extract(const KSI_OctetString *o, const unsigned char **data, unsigned int *data_len) {
	int res = KSI_UNKNOWN_ERROR;

	if (o == NULL || data == NULL) {
		res = KSI_INVALID_ARGUMENT;
		goto cleanup;
	}

	*data = o->data;
	*data_len = o->data_len;

	res = KSI_OK;

cleanup:

	 return res;
}

int KSI_OctetString_equals(const KSI_OctetString *left, const KSI_OctetString *right) {
	return left != NULL && right != NULL &&
			((left == right) || (left->data_len == right->data_len && !memcmp(left->data, right->data, left->data_len)));
}

int KSI_OctetString_fromTlv(KSI_TLV *tlv, KSI_OctetString **o) {
	KSI_ERR err;
	KSI_CTX *ctx = NULL;
	int res;
	const unsigned char *raw = NULL;
	unsigned int raw_len = 0;
	KSI_OctetString *tmp = NULL;

	KSI_PRE(&err, tlv != NULL) goto cleanup;
	KSI_PRE(&err, o != NULL) goto cleanup;

	ctx = KSI_TLV_getCtx(tlv);
	KSI_BEGIN(ctx, &err);

	res = KSI_TLV_cast(tlv, KSI_TLV_PAYLOAD_RAW);
	KSI_CATCH(&err, res) goto cleanup;

	res = KSI_TLV_getRawValue(tlv, &raw, &raw_len);
	KSI_CATCH(&err, res) goto cleanup;

	res = KSI_OctetString_new(ctx, raw, raw_len, &tmp);
	KSI_CATCH(&err, res) goto cleanup;

	*o = tmp;
	tmp = NULL;

	KSI_SUCCESS(&err);

cleanup:

	KSI_nofree(ctx);
	KSI_nofree(raw);
	KSI_OctetString_free(tmp);

	return KSI_RETURN(&err);
}

int KSI_OctetString_toTlv(KSI_CTX *ctx, KSI_OctetString *o, unsigned tag, int isNonCritical, int isForward, KSI_TLV **tlv) {
	KSI_ERR err;
	int res;
	KSI_TLV *tmp = NULL;

	KSI_PRE(&err, o != NULL) goto cleanup;
	KSI_PRE(&err, tlv != NULL) goto cleanup;
	KSI_BEGIN(ctx, &err);

	res = KSI_TLV_new(ctx, KSI_TLV_PAYLOAD_RAW, tag, isNonCritical, isForward, &tmp);
	KSI_CATCH(&err, res) goto cleanup;

	res = KSI_TLV_setRawValue(tmp, o->data, o->data_len);
	KSI_CATCH(&err, res) goto cleanup;

	*tlv = tmp;
	tmp = NULL;

	KSI_SUCCESS(&err);

cleanup:

	KSI_TLV_free(tmp);

	return KSI_RETURN(&err);
}

static int verifyUtf8(const unsigned char *str, unsigned len) {
	int res = KSI_UNKNOWN_ERROR;
    size_t i = 0;
    size_t charContinuationLen = 0;

    while (i < len) {
        if (i + 1 != len && str[i] == 0) {
        	/* The string contains a '\0' byte where not allowed. */
        	res = KSI_INVALID_FORMAT;
        	goto cleanup;
        } else if (str[i] <= 0x7f)
            charContinuationLen = 0;
        else if (str[i] >= 0xc0 /*11000000*/ && str[i] <= 0xdf /*11011111*/)
            charContinuationLen = 1;
        else if (str[i] >= 0xe0 /*11100000*/ && str[i] <= 0xef /*11101111*/)
            charContinuationLen = 2;
        else if (str[i] >= 0xf0 /*11110000*/ && str[i] <= 0xf4 /* Cause of RFC 3629 */)
            charContinuationLen = 3;
        else {
        	res = KSI_INVALID_FORMAT;
        	goto cleanup;
        }
        if (i + charContinuationLen >= len) {
        	res = KSI_BUFFER_OVERFLOW;
        	goto cleanup;
        }

        ++i;

        while (i < len && charContinuationLen > 0
               && str[i] >= 0x80 /*10000000*/ && str[i] <= 0xbf /*10111111*/) {
            ++i;
            --charContinuationLen;
        }
        if (charContinuationLen != 0) {
        	res = KSI_INVALID_FORMAT;
        	goto cleanup;
        }
    }

    res = KSI_OK;

cleanup:

    return res;
}
/**
 * Utf8String
 */
void KSI_Utf8String_free(KSI_Utf8String *o) {
	if (o != NULL && --o->refCount == 0) {
		KSI_free(o->value);
		KSI_free(o);
	}
}

int KSI_Utf8String_new(KSI_CTX *ctx, const char *str, unsigned len, KSI_Utf8String **o) {
	KSI_ERR err;
	int res;
	KSI_Utf8String *tmp = NULL;
	char *val = NULL;

	KSI_PRE(&err, ctx != NULL) goto cleanup;
	KSI_PRE(&err, str != NULL) goto cleanup;
	KSI_PRE(&err, o != NULL) goto cleanup;
	KSI_BEGIN(ctx, &err);

	tmp = KSI_new(KSI_Utf8String);
	if (tmp == NULL) {
		KSI_FAIL(&err, KSI_OUT_OF_MEMORY, NULL);
		goto cleanup;
	}

	tmp->ctx = ctx;
	tmp->value = NULL;
	tmp->refCount = 1;
	
	/* Verify that it is a null-terminated string. */
	if (len == 0 || str[len - 1] != '\0') {
		KSI_FAIL(&err, KSI_INVALID_FORMAT, "String value is not null-terminated.");
		goto cleanup;
	}

	/* Verify correctness of utf-8 */
	res = verifyUtf8((const unsigned char *)str, len);
	KSI_CATCH(&err, res) goto cleanup;

	val = KSI_malloc(len);
	memcpy(val, str, len);

	tmp->value = val;
	tmp->len = len;

	val = NULL;

	*o = tmp;
	tmp = NULL;

	KSI_SUCCESS(&err);

cleanup:

	KSI_free(val);
	KSI_Utf8String_free(tmp);

	return KSI_RETURN(&err);
}

int KSI_Utf8String_ref(KSI_Utf8String *o) {
	if (o != NULL) {
		++o->refCount;
	}
	return KSI_OK;
}

size_t KSI_Utf8String_size(const KSI_Utf8String *o) {
	return o != NULL ? o->len : 0;
}

const char *KSI_Utf8String_cstr(const KSI_Utf8String *o) {
	return o == NULL ? NULL : o->value;
}

int KSI_Utf8String_fromTlv(KSI_TLV *tlv, KSI_Utf8String **o) {
	KSI_ERR err;
	KSI_CTX *ctx = NULL;
	int res;
	const char *cstr = NULL;
	KSI_Utf8String *tmp = NULL;
	unsigned len;

	KSI_PRE(&err, tlv != NULL) goto cleanup;
	KSI_PRE(&err, o != NULL) goto cleanup;

	ctx = KSI_TLV_getCtx(tlv);
	KSI_BEGIN(ctx, &err);

	res = KSI_TLV_getRawValue(tlv, (const unsigned char **)&cstr, &len);
	KSI_CATCH(&err, res) goto cleanup;

	res = KSI_Utf8String_new(ctx, cstr, len, &tmp);
	KSI_CATCH(&err, res) goto cleanup;

	*o = tmp;
	tmp = NULL;

	KSI_SUCCESS(&err);

cleanup:

	KSI_nofree(ctx);
	KSI_nofree(cstr);
	KSI_Utf8String_free(tmp);

	return KSI_RETURN(&err);
}

int KSI_Utf8String_toTlv(KSI_CTX *ctx, KSI_Utf8String *o, unsigned tag, int isNonCritical, int isForward, KSI_TLV **tlv) {
	KSI_ERR err;
	int res;
	KSI_TLV *tmp = NULL;

	KSI_PRE(&err, o != NULL) goto cleanup;
	KSI_PRE(&err, tlv != NULL) goto cleanup;
	KSI_BEGIN(ctx, &err);

	res = KSI_TLV_new(ctx, KSI_TLV_PAYLOAD_RAW, tag, isNonCritical, isForward, &tmp);
	KSI_CATCH(&err, res) goto cleanup;

	if (o->len > 0xffff){
		KSI_FAIL(&err, KSI_INVALID_ARGUMENT, "UTF8 string too long for TLV conversion.");
		goto cleanup;
	}
	
	res = KSI_TLV_setRawValue(tmp, o->value, (unsigned)o->len);
	KSI_CATCH(&err, res) goto cleanup;

	*tlv = tmp;
	tmp = NULL;

	KSI_SUCCESS(&err);

cleanup:

	KSI_TLV_free(tmp);

	return KSI_RETURN(&err);
}

int KSI_Utf8StringNZ_fromTlv(KSI_TLV *tlv, KSI_Utf8String **o) {
	KSI_ERR err;
	KSI_CTX *ctx = NULL;
	int res;
	const unsigned char *cstr = NULL;
	KSI_Utf8String *tmp = NULL;

	KSI_PRE(&err, tlv != NULL) goto cleanup;
	KSI_PRE(&err, o != NULL) goto cleanup;

	ctx = KSI_TLV_getCtx(tlv);
	KSI_BEGIN(ctx, &err);

	res = KSI_Utf8String_fromTlv(tlv, &tmp);
	KSI_CATCH(&err, res) goto cleanup;

	if (tmp->len == 0 || (tmp->len == 1 && tmp->value[0] == 0)) {
		KSI_FAIL(&err, KSI_INVALID_FORMAT, "Empty string value not allowed.");
		goto cleanup;
	}

	*o = tmp;
	tmp = NULL;

	KSI_SUCCESS(&err);

cleanup:

	KSI_nofree(ctx);
	KSI_nofree(cstr);
	KSI_Utf8String_free(tmp);

	return KSI_RETURN(&err);
}

int KSI_Utf8StringNZ_toTlv(KSI_CTX *ctx, KSI_Utf8String *o, unsigned tag, int isNonCritical, int isForward, KSI_TLV **tlv) {
	KSI_ERR err;
	int res;
	KSI_TLV *tmp = NULL;

	KSI_PRE(&err, o != NULL) goto cleanup;
	KSI_PRE(&err, tlv != NULL) goto cleanup;
	KSI_BEGIN(ctx, &err);

	if (o->len == 0 || (o->len == 1 && o->value[0] == 0)) {
		KSI_FAIL(&err, KSI_INVALID_FORMAT, "Empty string value not allowed.");
		goto cleanup;
	}

	res = KSI_Utf8String_toTlv(ctx, o, tag, isNonCritical, isForward, &tmp);
	KSI_CATCH(&err, res) goto cleanup;

	*tlv = tmp;
	tmp = NULL;

	KSI_SUCCESS(&err);

cleanup:

	KSI_TLV_free(tmp);

	return KSI_RETURN(&err);
}

void KSI_Integer_free(KSI_Integer *o) {
	if (o != NULL && !o->staticAlloc && --o->refCount == 0) {
		KSI_free(o);
	}
}

int KSI_Integer_ref(KSI_Integer *o) {
	if (o != NULL && !o->staticAlloc) {
		++o->refCount;
	}
	return KSI_OK;
}

char *KSI_Integer_toDateString(const KSI_Integer *o, char *buf, unsigned buf_len) {
	char *ret = NULL;
	time_t pubTm;
	struct tm tm;

	pubTm = (time_t)o->value;

	gmtime_r(&pubTm, &tm);

	strftime(buf, buf_len, "%Y-%m-%d %H:%M:%S UTC", &tm);

	ret = buf;

	return ret;
}

KSI_uint64_t KSI_Integer_getUInt64(const KSI_Integer *o) {
	return o != NULL ? o->value : 0;
}

int KSI_Integer_equals(const KSI_Integer *a, const KSI_Integer *b) {
	return a != NULL && b != NULL && (a == b || a->value == b->value);
}

int KSI_Integer_equalsUInt(const KSI_Integer *o, KSI_uint64_t i) {
	return o != NULL && o->value == i;
}

int KSI_Integer_compare(const KSI_Integer *a, const KSI_Integer *b) {
	if (a == b || (a == NULL && b == NULL)) return 0;
	if (a == NULL && b != NULL) return -1;
	if (a != NULL && b == NULL) return 1;
	if (a->value > b->value)
		return 1;
	else if (a->value < b->value)
		return -1;
	else
		return 0;
}

int KSI_Integer_new(KSI_CTX *ctx, KSI_uint64_t value, KSI_Integer **o) {
	KSI_ERR err;
	KSI_Integer *tmp = NULL;
	static size_t poolSize = sizeof(integerPool) / sizeof(KSI_Integer);

	KSI_PRE(&err, ctx != NULL) goto cleanup;
	KSI_PRE(&err, o != NULL) goto cleanup;
	KSI_BEGIN(ctx, &err);

	if (value < poolSize) {
		tmp = integerPool + value;
	} else {
		tmp = KSI_new(KSI_Integer);
		if (tmp == NULL) {
			KSI_FAIL(&err, KSI_OUT_OF_MEMORY, NULL);
			goto cleanup;
		}

		tmp->staticAlloc = 0;
		tmp->value = value;
		tmp->refCount = 1;
	}

	*o = tmp;
	tmp = NULL;

	KSI_SUCCESS(&err);

cleanup:

	KSI_Integer_free(tmp);

	return KSI_RETURN(&err);
}

int KSI_Integer_fromTlv(KSI_TLV *tlv, KSI_Integer **o) {
	KSI_ERR err;
	KSI_CTX *ctx = NULL;
	int res;
	KSI_Integer *tmp = NULL;
	const unsigned char *raw = NULL;
	unsigned len;
	unsigned i;
	KSI_uint64_t val = 0;

	KSI_PRE(&err, tlv != NULL) goto cleanup;
	KSI_PRE(&err, o != NULL) goto cleanup;

	ctx = KSI_TLV_getCtx(tlv);
	KSI_BEGIN(ctx, &err);

	res = KSI_TLV_getRawValue(tlv, &raw, &len);
	KSI_CATCH(&err, res) goto cleanup;

	if (len > 8) {
		KSI_FAIL(&err, KSI_INVALID_FORMAT, "Integer larger than 64bit");
		goto cleanup;
	}

	for (i = 0; i < len; i++) {
		val = val << 8 | raw[i];
	}

	if (len != KSI_UINT64_MINSIZE(val)) {
		KSI_FAIL(&err, KSI_INVALID_FORMAT, "Integer not properly formated.");
		goto cleanup;
	}

	res = KSI_Integer_new(ctx, val, &tmp);
	KSI_CATCH(&err, res) goto cleanup;

 	*o = tmp;
	tmp = NULL;

	KSI_SUCCESS(&err);

cleanup:

	KSI_nofree(ctx);
	KSI_Integer_free(tmp);

	return KSI_RETURN(&err);
}

int KSI_Integer_toTlv(KSI_CTX *ctx, KSI_Integer *o, unsigned tag, int isNonCritical, int isForward, KSI_TLV **tlv) {
	KSI_ERR err;
	int res;
	KSI_TLV *tmp = NULL;
	unsigned char raw[8];
	unsigned len = 0;
	KSI_uint64_t val = o->value;

	KSI_PRE(&err, o != NULL) goto cleanup;
	KSI_PRE(&err, tlv != NULL) goto cleanup;
	KSI_BEGIN(ctx, &err);

	res = KSI_TLV_new(ctx, KSI_TLV_PAYLOAD_RAW, tag, isNonCritical, isForward, &tmp);
	KSI_CATCH(&err, res) goto cleanup;

	while (val != 0) {
		raw[7 - len++] = val & 0xff;
		val >>= 8;
	}

	if (len > 0) {
		res = KSI_TLV_setRawValue(tmp, raw + 8 - len, len);
		KSI_CATCH(&err, res) goto cleanup;
	}

	*tlv = tmp;
	tmp = NULL;

	KSI_SUCCESS(&err);

cleanup:

	KSI_TLV_free(tmp);

	return KSI_RETURN(&err);
}
