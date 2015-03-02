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

#ifndef PUBLICATIONSFILE_IMPL_H_
#define PUBLICATIONSFILE_IMPL_H_

#ifdef __cplusplus
extern "C" {
#endif

	struct KSI_PublicationsFile_st {
		KSI_CTX *ctx;
		unsigned char *raw;
		unsigned raw_len;
		KSI_PublicationsHeader *header;
		KSI_LIST(KSI_CertificateRecord) *certificates;
		KSI_LIST(KSI_PublicationRecord) *publications;
		size_t signatureOffset;
		KSI_PKISignature *signature;
	};

	struct KSI_PublicationData_st {
		KSI_CTX *ctx;
		KSI_Integer *time;
		KSI_DataHash *imprint;
	};

	struct KSI_PublicationRecord_st {
		KSI_CTX *ctx;
		KSI_PublicationData *publishedData;
		KSI_LIST(KSI_Utf8String) *publicationRef;
		KSI_LIST(KSI_Utf8String) *repositoryUriList;
	};


#ifdef __cplusplus
}
#endif

#endif /* PUBLICATIONSFILE_IMPL_H_ */
