/* Copyright (c) 2016 Cameron Harper
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * 
 * */

#ifndef BLINK_COMPACT_FORM_H
#define BLINK_COMPACT_FORM_H

/**
 * @defgroup blink_compact_form Compact form
 *
 * Compact form primitives.
 * 
 * @{
 * */

#ifdef __cplusplus
extern "C" {
#endif

/* includes ***********************************************************/

#include <stdint.h>
#include <stdbool.h>

/* functions **********************************************************/

/**
 * Encode a VLC null
 *
 * @param[out] out output buffer
 * @param[in] outMax maximum byte length of `out`
 *
 * @return uint8_t
 * @retval 0..1 (bytes successfully written to `out`)
 * 
 * */
uint8_t BLINK_EncodeVLCNull(uint8_t *out, uint32_t outMax);

/**
 * Return minimum number of bytes required to VLC encode unsigned integer value
 *
 * @param[in] value value to encode as VLC
 *
 * @return uint8_t
 * @retval 1..9 (minimum number of bytes to VLC encode unsigned `value`)
 * 
 * */
uint8_t BLINK_GetVLCSizeUnsigned(uint64_t value);

/**
 * Return minimum number of bytes required to VLC encode signed integer value
 *
 * @param[in] value value to encode as VLC
 *
 * @return uint8_t
 * @retval 1..9 (minimum number of bytes to VLC encode signed `value`)
 * 
 * */
uint8_t BLINK_GetVLCSizeSigned(int64_t value);

/**
 * Encode signed/unsigned integer as VLC
 *
 * @param[in] in value to encode
 * @param[in] isSigned if true `value` will be interpreted as a signed integer
 * @param[out] out output buffer
 * @param[in] outMax byte length of `out`
 *
 * @return uint8_t
 * 
 * @retval 0..9 (bytes successfully written to `out`)
 *
 * */
uint8_t BLINK_EncodeVLC(uint64_t in, bool isSigned, uint8_t *out, uint32_t outMax);

/**
 * Decode signed/unsigned integer as VLC
 *
 * @param[in] in input buffer
 * @param[in] inLen byte length of `in`
 * @param[in] isSigned integer shall be intepreted as 2s complement
 * @param[out] out output integer buffer
 * @param[out] isNull set to `true` if `in` decodes to NULL
 *
 * @return uint8_t
 * @retval 0..9 (bytes successfully read from `in`)
 *
 * */
uint8_t BLINK_DecodeVLC(const uint8_t *in, uint32_t inLen, bool isSigned, uint64_t *out, bool *isNull);

/**
 * Decode `bool`
 * 
 * @param[in] in input buffer
 * @param[in] inLen byte length of `in`
 * @param[out] out decoded value
 * @param[out] isNull set to `true` if `in` decodes to NULL
 *
 * @return uint8_t
 * 
 * @retval 0..1 (bytes successfully read from `in`)
 *
 * */
uint8_t BLINK_DecodeBool(const uint8_t *in, uint32_t inLen, bool *out, bool *isNull);

/**
 * Decode `u8`
 * 
 * @param[in] in input buffer
 * @param[in] inLen byte length of `in`
 * @param[out] out decoded value
 * @param[out] isNull set to `true` if `in` decodes to NULL
 *
 * @return uint8_t
 * 
 * @retval 0..1 (bytes successfully read from `in`)
 *
 * */
uint8_t BLINK_DecodeU8(const uint8_t *in, uint32_t inLen, uint8_t *out, bool *isNull);

/**
 * Decode `u16`
 * 
 * @param[in] in input buffer
 * @param[in] inLen byte length of `in`
 * @param[out] out decoded value
 * @param[out] isNull set to `true` if `in` decodes to NULL
 *
 * @return uint8_t
 * 
 * @retval 0..3 (bytes successfully read from `in`)
 *
 * */
uint8_t BLINK_DecodeU16(const uint8_t *in, uint32_t inLen, uint16_t *out, bool *isNull);

/**
 * Decode `u32`
 * 
 * @param[in] in input buffer
 * @param[in] inLen byte length of `in`
 * @param[out] out decoded value
 * @param[out] isNull set to `true` if `in` decodes to NULL
 *
 * @return uint8_t
 * 
 * @retval 0..5 (bytes successfully read from `in`)
 *
 * */
uint8_t BLINK_DecodeU32(const uint8_t *in, uint32_t inLen, uint32_t *out, bool *isNull);

/**
 * Decode `u64`
 * 
 * @param[in] in input buffer
 * @param[in] inLen byte length of `in`
 * @param[out] out decoded value
 * @param[out] isNull set to `true` if `in` decodes to NULL
 *
 * @return uint8_t
 * 
 * @retval 0..9 (bytes successfully read from `in`)
 *
 * */
uint8_t BLINK_DecodeU64(const uint8_t *in, uint32_t inLen, uint64_t *out, bool *isNull);

/**
 * Decode `i8`
 * 
 * @param[in] in input buffer
 * @param[in] inLen byte length of `in`
 * @param[out] out decoded value
 * @param[out] isNull set to `true` if `in` decodes to NULL
 *
 * @return uint8_t
 * 
 * @retval 0..1 (bytes successfully read from `in`)
 *
 * */
uint8_t BLINK_DecodeI8(const uint8_t *in, uint32_t inLen, int8_t *out, bool *isNull);

/**
 * Decode `i16`
 * 
 * @param[in] in input buffer
 * @param[in] inLen byte length of `in`
 * @param[out] out decoded value
 * @param[out] isNull set to `true` if `in` decodes to NULL
 *
 * @return uint8_t
 * 
 * @retval 0..3 (bytes successfully read from `in`)
 *
 * */
uint8_t BLINK_DecodeI16(const uint8_t *in, uint32_t inLen, int16_t *out, bool *isNull);

/**
 * Decode `i32`
 * 
 * @param[in] in input buffer
 * @param[in] inLen byte length of `in`
 * @param[out] out decoded value
 * @param[out] isNull set to `true` if `in` decodes to NULL
 *
 * @return uint8_t
 * 
 * @retval 0..5 (bytes successfully read from `in`)
 *
 * */
uint8_t BLINK_DecodeI32(const uint8_t *in, uint32_t inLen, int32_t *out, bool *isNull);

/**
 * Decode `i64`
 * 
 * @param[in] in input buffer
 * @param[in] inLen byte length of `in`
 * @param[out] out decoded value
 * @param[out] isNull set to `true` if `in` decodes to NULL
 *
 * @return uint8_t
 * 
 * @retval 0..9 (bytes successfully read from `in`)
 *
 * */
uint8_t BLINK_DecodeI64(const uint8_t *in, uint32_t inLen, int64_t *out, bool *isNull);

/**
 * Decode `binary`
 * 
 * @param[in] in input buffer
 * @param[in] inLen byte length of `in`
 * @param[out] out decoded value
 * @param[out] outLen byte length of `out`
 * @param[out] isNull set to `true` if `in` decodes to NULL
 *
 * @return uint8_t
 * 
 * @retval 0..0xffffffff (bytes successfully read from `in`)
 *
 * */
uint32_t BLINK_DecodeBinary(const uint8_t *in, uint32_t inLen, const uint8_t **out, uint32_t *outLen, bool *isNull);

/**
 * Decode `string`
 * 
 * @param[in] in input buffer
 * @param[in] inLen byte length of `in`
 * @param[out] out decoded value
 * @param[out] outLen byte length of `out`
 * @param[out] isNull set to `true` if `in` decodes to NULL
 *
 * @return uint8_t
 * 
 * @retval 0..0xffffffff (bytes successfully read from `in`)
 *
 * */
uint32_t BLINK_DecodeString(const uint8_t *in, uint32_t inLen, const uint8_t **out, uint32_t *outLen, bool *isNull);

/**
 * Decode `fixed`
 * 
 * @param[in] in input buffer
 * @param[in] inLen byte length of `in`
 * @param[out] out decoded value
 * @param[in] size byte size of `fixed` field
 *
 * @return uint8_t
 * 
 * @retval 0..0xffffffff (bytes successfully read from `in`)
 *
 * */
uint32_t BLINK_DecodeFixed(const uint8_t *in, uint32_t inLen, const uint8_t **out, uint32_t size);

/**
 * Decode an optional `fixed`
 * 
 * @param[in] in input buffer
 * @param[in] inLen byte length of `in`
 * @param[out] out decoded value
 * @param[out] size byte size of `fixed` field
 * @param[out] isNull set to `true` if `in` decodes to NULL
 *
 * @return uint8_t
 * 
 * @retval 0..0xffffffff (bytes successfully read from `in`)
 *
 * */
uint32_t BLINK_DecodeOptionalFixed(const uint8_t *in, uint32_t inLen, const uint8_t **out, uint32_t size, bool *isNull);

/**
 * Decode `decimal`
 * 
 * @param[in] in input buffer
 * @param[in] inLen byte length of `in`
 * @param[out] mantissa decoded mantissa
 * @param[out] exponent decoded exponent
 * @param[out] isNull set to `true` if `in` decodes to NULL
 *
 * @return uint8_t
 * 
 * @retval 0..11 (bytes successfully read from `in`)
 *
 * */
uint8_t BLINK_DecodeDecimal(const uint8_t *in, uint32_t inLen, int64_t *mantissa, int8_t *exponent, bool *isNull);

/**
 * Decode `f64`
 * 
 * @param[in] in input buffer
 * @param[in] inLen byte length of `in`
 * @param[out] out value
 * @param[out] isNull set to `true` if `in` decodes to NULL
 *
 * @return uint8_t
 * 
 * @retval 0..9 (bytes successfully read from `in`)
 *
 * */
uint8_t BLINK_DecodeF64(const uint8_t *in, uint32_t inLen, double *out, bool *isNull);
 
#ifdef __cplusplus
}
#endif

/** @} */
#endif
