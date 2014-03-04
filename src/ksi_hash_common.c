#include <string.h>

#include "ksi_internal.h"
#include "ksi_hash.h"

#define HASH_ALGO(id, name, bitcount, trusted) {(id), (name), (bitcount), (trusted)}

struct KSI_hashAlgorithmInfo_st {
	int algo_id;
	char *name;
	int bitCount;
	int trusted;
};

static struct KSI_hashAlgorithmInfo_st KSI_hashAlgorithmInfo[] = {
		HASH_ALGO(KSI_HASHALG_SHA1,			"SHA1", 		160, 1),
		HASH_ALGO(KSI_HASHALG_SHA2_256,		"SHA-256", 		256, 1),
		HASH_ALGO(KSI_HASHALG_RIPEMD160,	"RIPEMD-160", 	160, 1),
		HASH_ALGO(KSI_HASHALG_SHA2_224,		"SHA2-224", 	224, 1),
		HASH_ALGO(KSI_HASHALG_SHA2_384,		"SHA2-384", 	384, 1),
		HASH_ALGO(KSI_HASHALG_SHA2_512,		"SHA2-512", 	512, 1),
		HASH_ALGO(KSI_HASHALG_RIPEMD_256,	"RIPEMD-256", 	256, 1),
		HASH_ALGO(KSI_HASHALG_SHA3_244,		"SHA3-224", 	224, 1),
		HASH_ALGO(KSI_HASHALG_SHA3_256,		"SHA3-256", 	256, 1),
		HASH_ALGO(KSI_HASHALG_SHA3_384,		"SHA3-384", 	384, 1),
		HASH_ALGO(KSI_HASHALG_SHA3_512,		"SHA3-512", 	512, 1),
		HASH_ALGO(KSI_HASHALG_SM3, 			"SM3", 			256, 1)
};

/**
 *
 */
void KSI_DataHasher_free(KSI_DataHasher *hasher) {
	if (hasher != NULL) {
		KSI_free(hasher->hashContext);
		KSI_free(hasher);
	}
}

void KSI_DataHash_free(KSI_DataHash *hash) {
	if (hash != NULL) {
		KSI_free(hash->digest);
		KSI_free(hash);
	}
}

/**
 *
 */
int KSI_fixHashAlgorithm(int hash_id) {
	if (hash_id == KSI_HASHALG_DEFAULT) {
		return KSI_HASHALG_SHA2_256;
	}
	return hash_id;
}

/**
 *
 */
int KSI_isTrusteddHashAlgorithm(int hash_id) {
	int id = KSI_fixHashAlgorithm(hash_id);
	if (id >= 0 && id < KSI_NUMBER_OF_KNOWN_HASHALGS) {
		return KSI_hashAlgorithmInfo[id].trusted;
	}
	return 0;
}

int KSI_getHashLength(int hash_id) {
	int id = KSI_fixHashAlgorithm(hash_id);
	if (id >= 0 && id < KSI_NUMBER_OF_KNOWN_HASHALGS) {
		return (KSI_hashAlgorithmInfo[id].bitCount) >> 3;
	}
	return -1;
}

/**
 *
 */
int KSI_DataHash_getData(KSI_DataHash *hash, int *algorithm, unsigned char **digest, int *digest_length) {
	KSI_ERR err;
	unsigned char *tmp_digest = NULL;

	KSI_PRE(&err, hash != NULL) goto cleanup;

	KSI_BEGIN(hash->ctx, &err);

	tmp_digest = KSI_calloc(hash->digest_length, 1);
	if (tmp_digest == NULL) {
		KSI_FAIL(&err, KSI_OUT_OF_MEMORY, NULL);
		goto cleanup;
	}

	memcpy(tmp_digest, hash->digest, hash->digest_length);

	if (digest_length != NULL) *digest_length = hash->digest_length;
	if (algorithm != NULL) *algorithm = hash->algorithm;
	if (digest != NULL) {
		*digest = tmp_digest;
		tmp_digest = NULL;
	}

	KSI_SUCCESS(&err);

cleanup:

	KSI_free(tmp_digest);

	return KSI_RETURN(&err);
}

/**
 *
 */
int KSI_DataHash_fromData(KSI_CTX *ctx, int algorithm, unsigned char *digest, int digest_length, KSI_DataHash **hash) {
	KSI_ERR err;
	KSI_DataHash *tmp_hash = NULL;
	int res;

	KSI_PRE(&err, ctx != NULL) goto cleanup;

	KSI_BEGIN(ctx, &err);

	tmp_hash = KSI_new(KSI_DataHash);
	if (tmp_hash == NULL) {
		KSI_FAIL(&err, KSI_OUT_OF_MEMORY, NULL);
		goto cleanup;
	}

	tmp_hash->digest = NULL;

	res = KSI_DataHash_fromData_ex(algorithm, digest, digest_length, tmp_hash);
	KSI_CATCH(&err, res) goto cleanup;

	*hash = tmp_hash;
	tmp_hash = NULL;

	KSI_SUCCESS(&err);

cleanup:

	KSI_DataHash_free(tmp_hash);

	return KSI_RETURN(&err);
}

/**
 *
 */
int KSI_DataHash_fromData_ex(int algorithm, unsigned char *digest, int digest_length, KSI_DataHash *hash) {
	KSI_ERR err;
	unsigned char *tmp_digest = NULL;

	KSI_PRE(&err, hash != NULL) goto cleanup;
	KSI_PRE(&err, digest != NULL) goto cleanup;

	KSI_BEGIN(hash->ctx, &err);
	if (KSI_getHashLength(algorithm) != digest_length) {
		KSI_FAIL(&err, KSI_INVALID_FORMAT, "Digest length does not match with algorithm.");
		goto cleanup;
	}

	tmp_digest = KSI_calloc(digest_length, 1);
	if (tmp_digest == NULL) {
		KSI_FAIL(&err, KSI_OUT_OF_MEMORY, NULL);
		goto cleanup;
	}

	memcpy(tmp_digest, digest, digest_length);

	KSI_free(hash->digest);

	hash->algorithm = algorithm;
	hash->digest = tmp_digest;
	hash->digest_length = digest_length;

	tmp_digest = NULL;

	KSI_SUCCESS(&err);

cleanup:

	KSI_free(tmp_digest);

	return KSI_RETURN(&err);
}

/**
 *
 */
int KSI_DataHash_getImprint(KSI_DataHash *hash, unsigned char **imprint, int *imprint_length) {
	KSI_ERR err;
	unsigned char *tmp_imprint = NULL;

	KSI_PRE(&err, hash != NULL) goto cleanup;

	KSI_BEGIN(hash->ctx, &err);

	tmp_imprint = KSI_calloc(hash->digest_length + 1, 1);
	if (tmp_imprint == NULL) {
		KSI_FAIL(&err, KSI_OUT_OF_MEMORY, NULL);
		goto cleanup;
	}

	*tmp_imprint = (unsigned char) hash->algorithm;
	memcpy(tmp_imprint + 1, hash->digest, hash->digest_length);

	*imprint_length = hash->digest_length + 1;
	*imprint = tmp_imprint;
	tmp_imprint = NULL;

	KSI_SUCCESS(&err);

cleanup:

	KSI_free(tmp_imprint);

	return KSI_RETURN(&err);
}

/**
 *
 */
int KSI_DataHash_fromImprint(KSI_CTX *ctx, unsigned char *imprint, int imprint_length, KSI_DataHash **hash) {
	return KSI_DataHash_fromData(ctx, *imprint, imprint + 1, imprint_length - 1, hash);
}

/**
 *
 */
int KSI_DataHash_fromImprint_ex(unsigned char *imprint, int imprint_length, KSI_DataHash *hash) {
	KSI_ERR err;
	int res;
	KSI_PRE(&err, hash != NULL) goto cleanup;
	KSI_PRE(&err, imprint != NULL) goto cleanup;

	KSI_BEGIN(hash->ctx, &err);

	res = KSI_DataHash_fromData_ex(*imprint, imprint + 1, imprint_length - 1, hash);
	KSI_CATCH(&err, res) goto cleanup;

	KSI_SUCCESS(&err);

cleanup:

	return KSI_RETURN(&err);

}
