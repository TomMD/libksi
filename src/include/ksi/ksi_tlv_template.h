#include <stdlib.h>
#include "ksi_common.h"

#ifndef KSI_TLV_TEMPLATE_H_
#define KSI_TLV_TEMPLATE_H_

#ifdef __cplusplus
extern "C" {
#endif
	/**
	 * \addtogroup tlvtemplate TLV Templates
	 * The TLV templates are used to convert plain c structs into #KSI_TLV object trees and back.
	 * @{
	 */

	/**
	 * Template type.
	 */
	typedef struct KSI_TlvTemplate_st KSI_TlvTemplate;

	/**
	 * Generic value getter function type.
	 */
	typedef int (*getter_t)(const void *, void **);

	/**
	 * Generic value setter function type.
	 */
	typedef int (*setter_t)(void *, void *);

	/**
	 * Generic decode function type.
	 */
	typedef int (*cb_decode_t)(KSI_CTX *ctx, const KSI_TLV *, void *, const KSI_TlvTemplate *);

	/**
	 * Generic encode function type.
	 */
	typedef int (*cb_encode_t)(KSI_CTX *ctx, KSI_TLV *, const void *, const KSI_TlvTemplate *);

	/**
	 * TLV template strcuture.
	 */
	struct KSI_TlvTemplate_st {
		/**
		 * Template internal type.
		 */
		int type;

		/**
		 * TLV tag value.
		 */
		int tag;

		/**
		 * Is this TLV non-critical - should it produce an error when the parser does not know this tag.
		 */
		int isNonCritical;

		/**
		 * If the current TLV is unknown, should it be forwarded or dropped.
		 */
		int isForward;

		/**
		 * Getter function for the object value.
		 */
		getter_t getValue;

		/**
		 * Setter function for the object value.
		 */
		setter_t setValue;

		/**
		 * Object value basic constructor.
		 */
		int (*construct)(KSI_CTX *, void **);

		/**
		 * Object value destructor.
		 */
		void (*destruct)(void *);

		/**
		 * If the current tag is a composite TLV (i.e not a primitive type), the composite element is parsed
		 * using this sub-template.
		 */
		const KSI_TlvTemplate *subTemplate;

		/**
		 * If the object is a list, this function is used to add the element to it.
		 */
		int (*listAppend)(void *, void *);

		/**
		 * Can this element be added multiple times? If this value is 0, an error is thrown when more than
		 * one TLV of this tag is encountered in a single composite element.
		 */
		int multiple;

		/**
		 * If the object is a list, this function is used to initialize the list object.
		 */
		int (*listNew)(KSI_CTX *, void **);

		/**
		 * If the object is a list, this function is used to free the memory of it when an
		 * error occurs.
		 */
		void (*listFree)(void *);

		/**
		 * If the object is a list, this function is used to get the length of it.
		 */
		int (*listLength)(const void *);

		/**
		 * If the object is a list, this function is used for random access of its elements.
		 */
		int (*listElementAt)(const void *, int, void **);

		/**
		 * If the object can not be encoded as a TLV using templates, a callback function may be used.
		 */
		cb_encode_t callbackEncode;

		/**
		 * If the TLV can not be decoded into a object using templates, a callback function may be used.
		 */
		cb_decode_t callbackDecode;

		/**
		 * Simple function for converting a TLV into an object.
		 */
		int (*fromTlv)(KSI_TLV *tlv, void **);

		/**
		 * Simple function for converting an object into a TLV.
		 */
		int (*toTlv)(void *, int, int, int, KSI_TLV **tlv);
	};


	/**
	 * Derive template actual object name.
	 * \param[in]	name		Template name.
	 *
	 * \return Actual template object name (i.e the name concatenated with postfix "_template").
	 */
	#define KSI_TLV_TEMPLATE(name) name##_template

	/**
	 * This macro is used to import predefined templates.
	 */
	#define KSI_IMPORT_TLV_TEMPLATE(name) extern const KSI_TlvTemplate KSI_TLV_TEMPLATE(name)[];

	/**
	 * Generic #KSI_TlvTemplate type.
	 */
	#define KSI_TLV_TEMPLATE_OBJECT					1

	/**
	 * Composite (a nested TLV) #KSI_TlvTemplate type.
	 */
	#define KSI_TLV_TEMPLATE_COMPOSITE				2

	/**
	 * List #KSI_TlvTemplate type.
	 */
	#define KSI_TLV_TEMPLATE_LIST					3

	/**
	 * Callback #KSI_TlvTemplate type.
	 */
	#define KSI_TLV_TEMPLATE_CALLBACK				4

	/**
	 * Native unsigned 64-bit integer #KSI_TlvTemplate type.
	 */
	#define KSI_TLV_TEMPLATE_NATIVE_INT				5

	/**
	 * A special #KSI_TlvTemplate type for storing the absolute offset of the nested TLV object.
	 */
	#define KSI_TLV_TEMPLATE_SEEK_POS				6

	/**
	 * A helper macro for defining a single template with all parameters.
	 * \param[in]	typ				Template internal type.
	 * \param[in]	tg				TLV tag value.
	 * \param[in]	nc				Is the TLV non-critical?
	 * \param[in]	fw				Should the TLV be forwarded if unknown?
	 * \param[in]	gttr			Getter function.
	 * \param[in]	sttr			Setter function.
	 * \param[in]	constr			Constructor function.
	 * \param[in]	destr			Destructor functionn.
	 * \param[in]	subTmpl			Sub-template.
	 * \param[in]	list_append		List append function.
	 * \param[in]	mul				Are multiple values allowed inside a single TLV?
	 * \param[in]	list_new		List object constructor function.
	 * \param[in]	list_free		List object destructor function.
	 * \param[in]	list_len		List length function.
	 * \param[in]	list_elAt		List elements random access function.
	 * \param[in]	cbEnc			Object to TLV encode function.
	 * \param[in]	cbDec			TLV to Object decode function.
	 * \param[in]	fromTlv			Create object from TLV function.
	 * \param[in]	toTlv			Create TLV from object function.
	 */
	#define KSI_TLV_FULL_TEMPLATE_DEF(typ, tg, nc, fw, gttr, sttr, constr, destr, subTmpl, list_append, mul, list_new, list_free, list_len, list_elAt, cbEnc, cbDec, fromTlv, toTlv) 								\
				{ typ, tg, nc, fw, (getter_t)gttr, (setter_t)sttr, 																																					\
				(int (*)(KSI_CTX *, void **)) constr, (void (*)(void *)) destr, subTmpl, 																															\
				(int (*)(void *, void *))list_append, mul, (int (*)(KSI_CTX *, void **)) list_new, (void (*)(void *)) list_free, (int (*)(const void *)) list_len, (int (*)(const void *, int, void **))list_elAt, 	\
				(cb_encode_t)cbEnc, (cb_decode_t)cbDec, 																																							\
				(int (*)(KSI_TLV *, void **)) fromTlv, (int (*)(void *, int, int, int, KSI_TLV **))toTlv},																											\

	/**
	 * A helper macro for defining primitive templates.
	 * \param[in]	typ				Template internal type.
	 * \param[in]	tg				TLV tag value.
	 * \param[in]	nc				Is the TLV non-critical?
	 * \param[in]	fw				Should the TLV be forwarded if unknown?
	 * \param[in]	gttr			Getter function.
	 * \param[in]	sttr			Setter function.
	 */
	#define KSI_TLV_PRIMITIVE_TEMPLATE_DEF(typ, tg, nc, fw, gttr, sttr) KSI_TLV_FULL_TEMPLATE_DEF(typ, tg, nc, fw, gttr, sttr, NULL, NULL, NULL, NULL, 0, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL)

	/**
	 * This macro starts a #KSI_TlvTemplate definition. The definition is ended with #KSI_END_TLV_TEMPLATE .
	 * \param[in]	name		Template name - recomended to use the object type name.
	 */
	#define KSI_DEFINE_TLV_TEMPLATE(name)	const KSI_TlvTemplate name##_template[] = {

	/**
	 * Generic TLV template for objects for which \c fromTlv and \c toTlv functions are defined.
	 * \param[in]	tg				TLV tag value.
	 * \param[in]	nc				Is the TLV non-critical?
	 * \param[in]	fw				Should the TLV be forwarded if unknown?
	 * \param[in]	gttr			Getter function.
	 * \param[in]	sttr			Setter function.
	 * \param[in]	fromTlv			Function to create the object from TLV.
	 * \param[in]	toTlv			Function to create a TLV from the object.
	 */
	#define KSI_TLV_OBJECT(tg, nc, fw, gttr, sttr, fromTlv, toTlv) KSI_TLV_FULL_TEMPLATE_DEF(KSI_TLV_TEMPLATE_OBJECT, tg, nc, fw, gttr, sttr, NULL, NULL, NULL, NULL, 0, NULL, NULL, NULL, NULL, NULL, NULL, fromTlv, toTlv)

	/**
	 * TLV template for #KSI_Utf8String type.
	 * \param[in]	tg				TLV tag value.
	 * \param[in]	nc				Is the TLV non-critical?
	 * \param[in]	fw				Should the TLV be forwarded if unknown?
	 * \param[in]	gttr			Getter function.
	 * \param[in]	sttr			Setter function.
	 */
	#define KSI_TLV_UTF8_STRING(tg, nc, fw, gttr, sttr) KSI_TLV_OBJECT(tg, nc, fw, gttr, sttr, KSI_Utf8String_fromTlv, KSI_Utf8String_toTlv)

	/**
	 * TLV template for #KSI_Integer type.
	 * \param[in]	tg				TLV tag value.
	 * \param[in]	nc				Is the TLV non-critical?
	 * \param[in]	fw				Should the TLV be forwarded if unknown?
	 * \param[in]	gttr			Getter function.
	 * \param[in]	sttr			Setter function.
	 */
	#define KSI_TLV_INTEGER(tg, nc, fw, gttr, sttr) KSI_TLV_OBJECT(tg, nc, fw, gttr, sttr, KSI_Integer_fromTlv, KSI_Integer_toTlv)

	/**
	 * TLV template for #KSI_OctetString type.
	 * \param[in]	tg				TLV tag value.
	 * \param[in]	nc				Is the TLV non-critical?
	 * \param[in]	fw				Should the TLV be forwarded if unknown?
	 * \param[in]	gttr			Getter function.
	 * \param[in]	sttr			Setter function.
	 */
	#define KSI_TLV_OCTET_STRING(tg, nc, fw, gttr, sttr) KSI_TLV_OBJECT(tg, nc, fw, gttr, sttr, KSI_OctetString_fromTlv, KSI_OctetString_toTlv)

	/**
	 * TLV template for #KSI_DataHash type.
	 * \param[in]	tg				TLV tag value.
	 * \param[in]	nc				Is the TLV non-critical?
	 * \param[in]	fw				Should the TLV be forwarded if unknown?
	 * \param[in]	gttr			Getter function.
	 * \param[in]	sttr			Setter function.
	 */
	#define KSI_TLV_IMPRINT(tg, nc, fw, gttr, sttr) KSI_TLV_OBJECT(tg, nc, fw, gttr, sttr, KSI_DataHash_fromTlv, KSI_DataHash_toTlv)

	/**
	 * Native unsigned integer template.
	 * \param[in]	tg				TLV tag value.
	 * \param[in]	nc				Is the TLV non-critical?
	 * \param[in]	fw				Should the TLV be forwarded if unknown?
	 * \param[in]	gttr			Getter function.
	 * \param[in]	sttr			Setter function.
	 */
	#define KSI_TLV_NATIVE_INT(tg, nc, fw, gttr, sttr) KSI_TLV_PRIMITIVE_TEMPLATE_DEF(KSI_TLV_TEMPLATE_NATIVE_INT, tg, nc, fw, gttr, sttr)

	/**
	 * Generic object list template. The \c obj parameter may be only a type
	 * for which there is a list type defined (see #KSI_DEFINE_LIST and #KSI_IMPLEMENT_LIST).
	 * \param[in]	tg				TLV tag value.
	 * \param[in]	nc				Is the TLV non-critical?
	 * \param[in]	fw				Should the TLV be forwarded if unknown?
	 * \param[in]	gttr			Getter function.
	 * \param[in]	sttr			Setter function.
	 * \param[in]	obj				Type of object stored in the list.
	 */
	#define KSI_TLV_OBJECT_LIST(tg, nc, fw, gttr, sttr, obj) KSI_TLV_FULL_TEMPLATE_DEF(KSI_TLV_TEMPLATE_OBJECT, tg, nc, fw, gttr, sttr, NULL, NULL, NULL, obj##List_append, 1, obj##List_new, KSI_OctetStringList_free, obj##List_length, obj##List_elementAt, NULL, NULL, obj##_fromTlv, obj##_toTlv)

	/**
	 * TLV template for list of #KSI_OctetString types.
	 * \param[in]	tg				TLV tag value.
	 * \param[in]	nc				Is the TLV non-critical?
	 * \param[in]	fw				Should the TLV be forwarded if unknown?
	 * \param[in]	gttr			Getter function.
	 * \param[in]	sttr			Setter function.
	 */
	#define KSI_TLV_OCTET_STRING_LIST(tg, nc, fw, gttr, sttr) KSI_TLV_OBJECT_LIST(tg, nc, fw, gttr, sttr, KSI_OctetString)

	/**
	 * TLV template for list of #KSI_Utf8String types.
	 * \param[in]	tg				TLV tag value.
	 * \param[in]	nc				Is the TLV non-critical?
	 * \param[in]	fw				Should the TLV be forwarded if unknown?
	 * \param[in]	gttr			Getter function.
	 * \param[in]	sttr			Setter function.
	 */
	#define KSI_TLV_UTF8_STRING_LIST(tg, nc, fw, gttr, sttr) KSI_TLV_OBJECT_LIST(tg, nc, fw, gttr, sttr, KSI_Utf8String)

	/**
	 * TLV template for composite objects.
	 * \param[in]	tg				TLV tag value.
	 * \param[in]	nc				Is the TLV non-critical?
	 * \param[in]	fw				Should the TLV be forwarded if unknown?
	 * \param[in]	gttr			Getter function.
	 * \param[in]	sttr			Setter function.
	 * \param[in]	sub				Composite element template.
	 */
	#define KSI_TLV_COMPOSITE(tg, nc, fw, gttr, sttr, sub) KSI_TLV_FULL_TEMPLATE_DEF(KSI_TLV_TEMPLATE_COMPOSITE, tg, nc, fw, gttr, sttr, sub##_new, sub##_free, sub##_template, NULL, 0,  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL)

	/**
	 * TLV template for list of composite objects.
	 * \param[in]	tg				TLV tag value.
	 * \param[in]	nc				Is the TLV non-critical?
	 * \param[in]	fw				Should the TLV be forwarded if unknown?
	 * \param[in]	gttr			Getter function.
	 * \param[in]	sttr			Setter function.
	 * \param[in]	sub				Composite element template.
	 */
	#define KSI_TLV_COMPOSITE_LIST(tg, nc, fw, gttr, sttr, sub) KSI_TLV_FULL_TEMPLATE_DEF(KSI_TLV_TEMPLATE_COMPOSITE, tg, nc, fw, gttr, sttr, sub##_new, sub##_free, sub##_template, sub##List_append, 1, sub##List_new, sub##List_free, sub##List_length, sub##List_elementAt, NULL, NULL, NULL, NULL)

	/**
	 * A special TLV template to retreive the absolute position of the TLV.
	 * \param[in]	tg				TLV tag value.
	 * \param[in]	sttr			Setter function for int value.
	 */
	#define KSI_TLV_SEEK_POS(tg, sttr) KSI_TLV_PRIMITIVE_TEMPLATE_DEF(KSI_TLV_TEMPLATE_SEEK_POS, tg, 0, 0, NULL, sttr)

	/**
	 * TLV template to encode and decode using callback functions.
	 * \param[in]	tg				TLV tag value.
	 * \param[in]	nc				Is the TLV non-critical?
	 * \param[in]	fw				Should the TLV be forwarded if unknown?
	 * \param[in]	gttr			Getter function.
	 * \param[in]	sttr			Setter function.
	 * \param[in]	encode			Encode (object to TLV) function.
	 * \param[in]	decode			Decode (TLV to object) function.
	 */
	#define KSI_TLV_CALLBACK(tg, nc, fw, gttr, sttr, encode, decode) KSI_TLV_FULL_TEMPLATE_DEF(KSI_TLV_TEMPLATE_CALLBACK, tg, nc, fw, gttr, sttr, NULL, NULL, NULL, NULL, 1, NULL, NULL, NULL, NULL, encode, decode, NULL, NULL)

	/**
	 * This macro ends the #KSI_TlvTemplate definition started by #KSI_TLV_TEMPLATE.
	 */
	#define KSI_END_TLV_TEMPLATE { -1, 0, 0, 0, NULL, NULL, NULL, NULL, NULL, NULL, 0, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL}};

	/**
	 * Given a TLV object, template and a initialized target payload, this function evaluates the payload objects
	 * with the data from the TLV.
	 * \param[in]		ctx			KSI context.
	 * \param[in]		payload		Preinitialized empty object to be evaluated with the TLV values.
	 * \param[in]		tlv			TLV value which has the structure represented in \c template.
	 * \param[in]		template	Template of the TLV expected structure.
	 * \param[in]		reminder	List of TLV's that did not match the template on the first level. Can be NULL, in which case
	 * 								an error code is returned if an unknown non-critical TLV is encountered.
	 * \return status code (\c KSI_OK, when operation succeeded, otherwise an
	 * error code).
	 */
	int KSI_TlvTemplate_extract(KSI_CTX *ctx, void *payload, KSI_TLV *tlv, const KSI_TlvTemplate *template, KSI_LIST(KSI_TLV) *reminder);

	/**
	 * This function acts similary as #KSI_TlvTemplate_extract but allows the caller to specify how the top level
	 * TLV's are retrieved (e.g. read from a file).
	 * \param[in]		ctx				KSI context.
	 * \param[in]		payload			Preinitialized empty object to be evaluated with the TLV values.
	 * \param[in]		generatorCtx	Context for the generator.
	 * \param[in]		tlv				TLV value which has the structure represented in \c template.
	 * \param[in]		template		Template of the TLV expected structure.
	 * \param[in]		reminder		List of TLV's that did not match the template on the first level. Can be NULL, in which case
	 * 									an error code is returned if an unknown non-critical TLV is encountered.
	 * \param[in]		generator		Generator function. The \c generatorCtx is passed as the first parameter and a #KSI_TLV object
	 * 									is expected to be returned by the second parameter - a NULL value is interpreted as end of input.
	 * 									The function is expected to return #KSI_OK on success.
	 */
	int KSI_TlvTemplate_extractGenerator(KSI_CTX *ctx, void *payload, void *generatorCtx, const KSI_TlvTemplate *template, KSI_LIST(KSI_TLV) *reminder, int (*generator)(void *, KSI_TLV **));

	/**
	 * Given a payload object, template and a initialized target TLV, this function constructs a TLV using the
	 * template and the values from the payload.
	 * \param[in]		ctx			KSI context.
	 * \param[in]		payload		Evaluated payload.
	 * \param[in]		tlv			An empty target TLV.
	 * \param[in]		template	Template of the TLV expected structure.
	 *
	 * \return status code (\c KSI_OK, when operation succeeded, otherwise an
	 * error code).
	 */
	int KSI_TlvTemplate_construct(KSI_CTX *ctx, KSI_TLV *tlv, const void *payload, const KSI_TlvTemplate *template);

	/**
	 * Deepcopy an object using TLV templates. The object is first transformed internally into a #KSI_TLV tree and
	 * the process is reversed and the result is stoed, thus all values are copied.
	 *
	 * \return status code (\c KSI_OK, when operation succeeded, otherwise an
	 * error code).
	 *
	 * \note If the TLV template is incomplete and discards by endoding or decoding some values, the result is not an
	 * exact copy of the original.
	 */
	int KSI_TlvTemplate_deepCopy(KSI_CTX *ctx, const void *from, const KSI_TlvTemplate *baseTemplate, void *to);

	/**
	 * @}
	 */

#ifdef __cplusplus
}
#endif

#endif /* KSI_TLV_TEMPLATE_H_ */
