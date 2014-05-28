#ifndef KSI_NET_H_
#define KSI_NET_H_

#ifdef __cplusplus
extern "C" {
#endif

	/**
	 *
	 */
	void KSI_NetHandle_free(KSI_NetHandle *heandle);

	/**
	 *
	 */
	void KSI_NetProvider_free(KSI_NetProvider *provider);

	/**
	 * TODO!
	 */
	int KSI_NetProvider_sendSignRequest(KSI_NetProvider *provider, KSI_NetHandle *handle);
	int KSI_NetProvider_sendExtendRequest(KSI_NetProvider *provider, KSI_NetHandle *handle);
	int KSI_NetProvider_sendPublicationsFileRequest(KSI_NetProvider *provider, KSI_NetHandle *handle);

	int KSI_NetHandle_receive(KSI_NetHandle *handle);

	int KSI_NetHandle_setResponse(KSI_NetHandle *handle, const unsigned char *response, int response_len);
	int KSI_NetHandle_setNetContext(KSI_NetHandle *handle, void *, void (*netCtx_free)(void *));
	void *KSI_NetHandle_getNetContext(KSI_NetHandle *handle);
	int KSI_NetHandle_getRequest(KSI_NetHandle *handle, const unsigned char **response, int *response_len);
	int KSI_NetHandle_getResponse(KSI_NetHandle *handle, const unsigned char **response, int *response_len);
	void *KSI_NetHandle_getNetContext(KSI_NetHandle *handle);
	int KSI_NetHandle_new(KSI_CTX *ctx, const unsigned char *request, int request_length, KSI_NetHandle **handle);
	int KSI_NetHandle_setReadResponseFn(KSI_NetHandle *handle, int fn(KSI_NetHandle *));

	int KSI_NetProvider_new(KSI_CTX *ctx, KSI_NetProvider **provider);
	int KSI_NetProvider_setNetCtx(KSI_NetProvider *provider, void *netCtx, void (*netCtx_free)(void *));
	int KSI_NetProvider_setSendSignRequestFn(KSI_NetProvider *provider, int (*fn)(KSI_NetProvider *, KSI_NetHandle *));
	int KSI_NetProvider_setSendExtendRequestFn(KSI_NetProvider *provider, int (*fn)(KSI_NetProvider *, KSI_NetHandle *));
	int KSI_NetProvider_setSendPublicationRequestFn(KSI_NetProvider *provider, int (*fn)(KSI_NetProvider *, KSI_NetHandle *));
	void *KSI_NetProvider_getNetContext(KSI_NetProvider *provider);


#ifdef __cplusplus
}
#endif

#endif /* KSI_NET_H_ */
