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

#ifndef BLINK_COMPACT_H
#define BLINK_COMPACT_H

/**
 * @defgroup blink_compact blink_compact
 * @ingroup ublink
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

/* typedefs ***********************************************************/

struct blink_stream;

typedef struct blink_stream * blink_stream_t;

/* functions **********************************************************/

/**
 * Encode a VLC null
 *
 * @param[out] out output buffer
 * @param[in] outMax maximum byte length of `out`
 *
 * @return bytes written to `out`
 * @retval 0..1
 * 
 * */
bool BLINK_Compact_encodeNull(blink_stream_t out);

/**
 * Encode a present symbol
 *
 * @param[out] out output buffer
 * @param[in] outMax maximum byte length of `out`
 *
 * @return bytes written to `out`
 * @retval 0..1
 * 
 * */
bool BLINK_Compact_encodePresent(blink_stream_t out);

/**
 * Decode `bool`
 * 
 * @param[in] in input buffer
 * @param[in] inLen byte length of `in`
 * @param[out] out decoded value
 * @param[out] isNull set to `true` if `in` decodes to NULL
 *
 * @return number of bytes successfully read from `in`
 * @retval 0..1
 *
 * */
bool BLINK_Compact_decodeBool(blink_stream_t in, bool *out, bool *isNull);

/**
 * Decode `u8`
 * 
 * @param[in] in input buffer
 * @param[in] inLen byte length of `in`
 * @param[out] out decoded value
 * @param[out] isNull set to `true` if `in` decodes to NULL
 *
 * @return number of bytes successfully read from `in`
 * @retval 0..1
 *
 * */
bool BLINK_Compact_decodeU8(blink_stream_t in, uint8_t *out, bool *isNull);

/**
 * Decode `u16`
 * 
 * @param[in] in input buffer
 * @param[in] inLen byte length of `in`
 * @param[out] out decoded value
 * @param[out] isNull set to `true` if `in` decodes to NULL
 *
 * @return number of bytes successfully read from `in`
 * @retval 0..3
 *
 * */
bool BLINK_Compact_decodeU16(blink_stream_t in, uint16_t *out, bool *isNull);

/**
 * Decode `u32`
 * 
 * @param[in] in input buffer
 * @param[in] inLen byte length of `in`
 * @param[out] out decoded value
 * @param[out] isNull set to `true` if `in` decodes to NULL
 *
 * @return number of bytes successfully read from `in`
 * @retval 0..5
 *
 * */
bool BLINK_Compact_decodeU32(blink_stream_t in, uint32_t *out, bool *isNull);

/**
 * Decode `u64`
 * 
 * @param[in] in input buffer
 * @param[in] inLen byte length of `in`
 * @param[out] out decoded value
 * @param[out] isNull set to `true` if `in` decodes to NULL
 *
 * @return number of bytes successfully read from `in`
 * @retval 0..9
 *
 * */
bool BLINK_Compact_decodeU64(blink_stream_t in, uint64_t *out, bool *isNull);

/**
 * Decode `i8`
 * 
 * @param[in] in input buffer
 * @param[in] inLen byte length of `in`
 * @param[out] out decoded value
 * @param[out] isNull set to `true` if `in` decodes to NULL
 *
 * @return number of bytes successfully read from `in`
 * @retval 0..1
 *
 * */
bool BLINK_Compact_decodeI8(blink_stream_t in, int8_t *out, bool *isNull);

/**
 * Decode `i16`
 * 
 * @param[in] in input buffer
 * @param[in] inLen byte length of `in`
 * @param[out] out decoded value
 * @param[out] isNull set to `true` if `in` decodes to NULL
 *
 * @return number of bytes successfully read from `in`
 * @retval 0..3
 *
 * */
bool BLINK_Compact_decodeI16(blink_stream_t in, int16_t *out, bool *isNull);

/**
 * Decode `i32`
 * 
 * @param[in] in input buffer
 * @param[in] inLen byte length of `in`
 * @param[out] out decoded value
 * @param[out] isNull set to `true` if `in` decodes to NULL
 *
 * @return number of bytes successfully read from `in`
 * @retval 0..5
 *
 * */
bool BLINK_Compact_decodeI32(blink_stream_t in, int32_t *out, bool *isNull);

/**
 * Decode `i64`
 * 
 * @param[in] in input buffer
 * @param[in] inLen byte length of `in`
 * @param[out] out decoded value
 * @param[out] isNull set to `true` if `in` decodes to NULL
 *
 * @return number of bytes successfully read from `in`
 * @retval 0..9
 *
 * */
bool BLINK_Compact_decodeI64(blink_stream_t in, int64_t *out, bool *isNull);

/**
 * Decode `decimal`
 * 
 * @param[in] in input buffer
 * @param[in] inLen byte length of `in`
 * @param[out] mantissa decoded mantissa
 * @param[out] exponent decoded exponent
 * @param[out] isNull set to `true` if `in` decodes to NULL
 *
 * @return number of bytes successfully read from `in`
 * @retval 0..11
 *
 * */
bool BLINK_Compact_decodeDecimal(blink_stream_t in, int64_t *mantissa, int8_t *exponent, bool *isNull);

/**
 * Decode `f64`
 * 
 * @param[in] in input buffer
 * @param[in] inLen byte length of `in`
 * @param[out] out value
 * @param[out] isNull set to `true` if `in` decodes to NULL
 *
 * @return number of bytes successfully read from `in`
 * @retval 0..9
 *
 * */
bool BLINK_Compact_decodeF64(blink_stream_t in, double *out, bool *isNull);

/**
 * Decode a present field
 *
 * @param[in] in input buffer
 * @param[in] inLen byte length of `in`
 * @param[out] out set to `true` if present
 *
 * @return number of bytes successfully read from `in`
 * @retval 0..1
 * 
 * */
bool BLINK_Compact_decodePresent(blink_stream_t in, bool *out);

/**
 * Encode `bool`
 *
 * @param[in] in input value
 * @param[out] out output buffer
 * @param[in] outMax maximum byte length of `out`
 *
 * @return number of bytes successfully written to `out`
 * @retval 0..1
 *
 * */
bool BLINK_Compact_encodeBool(bool in, blink_stream_t out);

/**
 * Encode `u8`
 *
 * @param[in] in input value
 * @param[out] out output buffer
 * @param[in] outMax maximum byte length of `out`
 *
 * @return number of bytes successfully written to `out`
 * @retval 0..2
 *
 * */
bool BLINK_Compact_encodeU8(uint8_t in, blink_stream_t out);

/**
 * Encode `u16`
 *
 * @param[in] in input value
 * @param[out] out output buffer
 * @param[in] outMax maximum byte length of `out`
 *
 * @return number of bytes successfully written to `out`
 * @retval 0..3
 *
 * */
bool BLINK_Compact_encodeU16(uint16_t in, blink_stream_t out);

/**
 * Encode `u32`
 *
 * @param[in] in input value
 * @param[out] out output buffer
 * @param[in] outMax maximum byte length of `out`
 *
 * @return number of bytes successfully written to `out`
 * @retval 0..5
 *
 * */
bool BLINK_Compact_encodeU32(uint32_t in, blink_stream_t out);

/**
 * Encode `u64`
 *
 * @param[in] in input value
 * @param[out] out output buffer
 * @param[in] outMax maximum byte length of `out`
 *
 * @return number of bytes successfully written to `out`
 * @retval 0..9
 *
 * */
bool BLINK_Compact_encodeU64(uint64_t in, blink_stream_t out);

/**
 * Encode `i8`
 *
 * @param[in] in input value
 * @param[out] out output buffer
 * @param[in] outMax maximum byte length of `out`
 *
 * @return number of bytes successfully written to `out`
 * @retval 0..2
 *
 * */
bool BLINK_Compact_encodeI8(int8_t in, blink_stream_t out);

/**
 * Encode `i16`
 *
 * @param[in] in input value
 * @param[out] out output buffer
 * @param[in] outMax maximum byte length of `out`
 *
 * @return number of bytes successfully written to `out`
 * @retval 0..3
 *
 * */
bool BLINK_Compact_encodeI16(int16_t in, blink_stream_t out);

/**
 * Encode `i32`
 *
 * @param[in] in input value
 * @param[out] out output buffer
 * @param[in] outMax maximum byte length of `out`
 *
 * @return number of bytes successfully written to `out`
 * @retval 0..5
 *
 * */
bool BLINK_Compact_encodeI32(int32_t in, blink_stream_t out);

/**
 * Encode `i64`
 *
 * @param[in] in input value
 * @param[out] out output buffer
 * @param[in] outMax maximum byte length of `out`
 *
 * @return number of bytes successfully written to `out`
 * @retval 0..9
 *
 * */
bool BLINK_Compact_encodeI64(int64_t in, blink_stream_t out);

/**
 * Encode `f64`
 *
 * @param[in] in input value
 * @param[out] out output buffer
 * @param[in] outMax maximum byte length of `out`
 *
 * @return number of bytes successfully written to `out`
 * @retval 0..9
 *
 * */
bool BLINK_Compact_encodeF64(double in, blink_stream_t out);

/**
 * Encode `decimal`
 *
 * @param[in] mantissa
 * @param[in] exponent
 * @param[out] output buffer
 * @param[in] outMax maximum byte length of `out`
 *
 * @return number of bytes successfully written to `out`
 * @retval 0..11
 *
 * */
bool BLINK_Compact_encodeDecimal(int64_t mantissa, int8_t exponent, blink_stream_t out);
 
#ifdef __cplusplus
}
#endif

/** @} */
#endif