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

#ifndef BLINK_OBJECT_H
#define BLINK_OBJECT_H

/**
 * @defgroup blink_object blink_object
 * @ingroup ublink
 *
 * Object model interface (under development)
 *
 * @{
 * */

/* includes ***********************************************************/

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

/* enums **************************************************************/

/* structs ************************************************************/

struct blink_object;
struct blink_pool;
struct blink_stream;
struct blink_schema;

struct blink_decimal {
    int64_t mantissa;   
    int8_t exponent;
};

struct blink_string {
    const uint8_t *data;
    uint32_t len;
};

typedef struct blink_object * blink_object_t;
typedef struct blink_pool * blink_pool_t;
typedef struct blink_stream * blink_stream_t;
typedef struct blink_schema * blink_schema_t;

/* functions **********************************************************/

/** Create a new group model from a group definition
 *
 * @param[in] pool pool to allocate from
 * @param[in] group group definition
 *
 * @return group model
 *
 * @retval NULL could not create group model
 *
 * */
blink_object_t BLINK_Object_newGroup(blink_pool_t pool, blink_schema_t group);

/** Set a group field to NULL
 *
 * @param[in] group
 * @parma[in] fieldName
 *
 * @return true if successful
 *
 * */
bool BLINK_Object_setNull(blink_object_t group, const char *fieldName);

/** Write enum to field
 *
 * @param[in] group
 * @parma[in] fieldName
 * @param[in] value enum
 *
 * @return true if successful
 *
 * */
bool BLINK_Object_setEnum(blink_object_t group, const char *fieldName, const char *value);

/** Write boolean to field
 *
 * @param[in] group
 * @parma[in] fieldName
 * @param[in] value boolean
 *
 * @return true if successful
 *
 * */
bool BLINK_Object_setBool(blink_object_t group, const char *fieldName, bool value);

/** Write decimal to field
 *
 * @param[in] group
 * @parma[in] fieldName
 * @param[in] value decimal
 *
 * @return true if successful
 *
 * */
bool BLINK_Object_setDecimal(blink_object_t group, const char *fieldName, struct blink_decimal value);

/** Write u8 to field
 *
 * @param[in] group
 * @parma[in] fieldName
 * @param[in] value u8
 *
 * @return true if successful
 *
 * */
bool BLINK_Object_setU8(blink_object_t group, const char *fieldName, uint8_t value);

/** Write u16 to field
 *
 * @param[in] group
 * @parma[in] fieldName
 * @param[in] value u16
 *
 * @return true if successful
 *
 * */
bool BLINK_Object_setU16(blink_object_t group, const char *fieldName, uint16_t value);

/** Write u32 to field
 *
 * @param[in] group
 * @parma[in] fieldName
 * @param[in] value u32
 *
 * @return true if successful
 *
 * */
bool BLINK_Object_setU32(blink_object_t group, const char *fieldName, uint32_t value);

/** Write u64 to field
 *
 * @param[in] group
 * @parma[in] fieldName
 * @param[in] value u64
 *
 * @return true if successful
 *
 * */
bool BLINK_Object_setU64(blink_object_t group, const char *fieldName, uint64_t value);

/** Write i8 to field
 *
 * @param[in] group
 * @parma[in] fieldName
 * @param[in] value i8
 *
 * @return true if successful
 *
 * */
bool BLINK_Object_setI8(blink_object_t group, const char *fieldName, int8_t value);

/** Write i16 to field
 *
 * @param[in] group
 * @parma[in] fieldName
 * @param[in] value i16
 *
 * @return true if successful
 *
 * */
bool BLINK_Object_setI16(blink_object_t group, const char *fieldName, int16_t value);

/** Write i32 to field
 *
 * @param[in] group
 * @parma[in] fieldName
 * @param[in] value i32
 *
 * @return true if successful
 *
 * */
bool BLINK_Object_setI32(blink_object_t group, const char *fieldName, int32_t value);

/** Write i64 to field
 *
 * @param[in] group
 * @parma[in] fieldName
 * @param[in] value i64
 *
 * @return true if successful
 *
 * */
bool BLINK_Object_setI64(blink_object_t group, const char *fieldName, int64_t value);

/** Write f64 to field
 *
 * @param[in] group
 * @parma[in] fieldName
 * @param[in] value f64
 *
 * @return true if successful
 *
 * */
bool BLINK_Object_setF64(blink_object_t group, const char *fieldName, double value);

/** Write string to field
 *
 * @param[in] group
 * @parma[in] fieldName
 * @param[in] value string
 *
 * @return true if successful
 *
 * */
bool BLINK_Object_setString(blink_object_t group, const char *fieldName, struct blink_string value);

/** Write binary to field
 *
 * @param[in] group
 * @parma[in] fieldName
 * @param[in] value binary
 *
 * @return true if successful
 *
 * */
bool BLINK_Object_setBinary(blink_object_t group, const char *fieldName, struct blink_string value);

/** Write fixed to field
 *
 * @param[in] group
 * @parma[in] fieldName
 * @param[in] value fixed
 *
 * @return true if successful
 *
 * */
bool BLINK_Object_setFixed(blink_object_t group, const char *fieldName, struct blink_string value);

/** Write group to field
 *
 * @param[in] group
 * @parma[in] fieldName
 * @param[in] value group
 *
 * @return true if successful
 *
 * */
bool BLINK_Object_setGroup(blink_object_t group, const char *fieldName, blink_object_t value);

/** Test if field value is NULL
 *
 * @param[in] group
 * @parma[in] fieldName
 *
 * @return true if field value is NULL
 *
 * */
bool BLINK_Object_fieldIsNull(blink_object_t group, const char *fieldName);

/** Read enum from field
 *
 * @param[in] group
 * @parma[in] fieldName
 * @param[out] value length of enum 
 * @param[out] valueLen length of enum value
 *
 * @return true if value can be read
 *
 * */
bool BLINK_Object_getEnum(blink_object_t group, const char *fieldName, const char **value, size_t *valueLen);

/** Read boolean from field
 *
 * @param[in] group
 * @parma[in] fieldName
 * @param[out] value boolean
 *
 * @return true if value can be read
 *
 * */
bool BLINK_Object_getBool(blink_object_t group, const char *fieldName, bool *value);

/** Read decimal from field
 *
 * @param[in] group
 * @parma[in] fieldName
 * @param[out] value boolean
 *
 * @return true if value can be read
 *
 * */
bool BLINK_Object_getDecimal(blink_object_t group, const char *fieldName, struct blink_decimal *value);

/** Read u8 from field
 *
 * @param[in] group
 * @parma[in] fieldName
 * @param[out] value u8
 *
 * @return true if value can be read
 *
 * */
bool BLINK_Object_getU8(blink_object_t group, const char *fieldName, uint8_t *value);

/** Read u16 from field
 *
 * @param[in] group
 * @parma[in] fieldName
 * @param[out] value u16
 *
 * @return true if value can be read
 *
 * */
bool BLINK_Object_getU16(blink_object_t group, const char *fieldName, uint16_t *value);

/** Read u32 from field
 *
 * @param[in] group
 * @parma[in] fieldName
 * @param[out] value u32
 *
 * @return true if value can be read
 *
 * */
bool BLINK_Object_getU32(blink_object_t group, const char *fieldName, uint32_t *value);

/** Read u64 from field
 *
 * @param[in] group
 * @parma[in] fieldName
 * @param[out] value u64
 *
 * @return true if value can be read
 *
 * */
bool BLINK_Object_getU64(blink_object_t group, const char *fieldName, uint64_t *value);

/** Read i8 from field
 *
 * @param[in] group
 * @parma[in] fieldName
 * @param[out] value i8
 *
 * @return true if value can be read
 *
 * */
bool BLINK_Object_getI8(blink_object_t group, const char *fieldName, int8_t *value);

/** Read i16 from field
 *
 * @param[in] group
 * @parma[in] fieldName
 * @param[out] value i16
 *
 * @return true if value can be read
 *
 * */
bool BLINK_Object_getI16(blink_object_t group, const char *fieldName, int16_t *value);

/** Read i32 from field
 *
 * @param[in] group
 * @parma[in] fieldName
 * @param[out] value i32
 *
 * @return true if value can be read
 *
 * */
bool BLINK_Object_getI32(blink_object_t group, const char *fieldName, int32_t *value);

/** Read i64 from field
 *
 * @param[in] group
 * @parma[in] fieldName
 * @param[out] value i64
 *
 * @return true if value can be read
 *
 * */
bool BLINK_Object_getI64(blink_object_t group, const char *fieldName, int64_t *value);

/** Read f64 from field
 *
 * @param[in] group
 * @parma[in] fieldName
 * @param[out] value f64
 *
 * @return true if value can be read
 *
 * */
bool BLINK_Object_getF64(blink_object_t group, const char *fieldName, double *value);

/** Read string from field
 *
 * @param[in] group
 * @parma[in] fieldName
 * @param[out] value string
 *
 * @return true if value can be read
 *
 * */
bool BLINK_Object_getString(blink_object_t group, const char *fieldName, struct blink_string *value);

/** Read binary from field
 *
 * @param[in] group
 * @parma[in] fieldName
 * @param[out] value binary
 *
 * @return true if value can be read
 *
 * */
bool BLINK_Object_getBinary(blink_object_t group, const char *fieldName, struct blink_string *value);

/** Read fixed from field
 *
 * @param[in] group
 * @parma[in] fieldName
 * @param[out] value fixed
 *
 * @return true if value can be read
 *
 * */
bool BLINK_Object_getFixed(blink_object_t group, const char *fieldName, struct blink_string *value);

/** Read group from field
 *
 * @param[in] group
 * @parma[in] fieldName
 * @param[out] value group
 *
 * @return true if value can be read
 *
 * */
bool BLINK_Object_getGroup(blink_object_t group, const char *fieldName, blink_object_t *value);

/** Read field definition from field
 *
 * @param[in] group
 * @param[in] fieldName
 *
 * @return field definition
 *
 * */
blink_schema_t BLINK_Object_getFieldDefinition(blink_object_t group, cont char *fieldName);

/** Compact encode a group
 *
 * @param[in] group
 * @param[in] out output stream
 *
 * @return true if group was encoded 
 *
 * */
bool BLINK_Object_encodeCompact(blink_object_t group, blink_stream_t out);

/** Compact decode a group
 *
 * @param[in] pool
 * @param[in] schema
 * @param[in] in input stream
 *
 * @return group
 * @return NULL group could not be decoded
 *
 * */
blink_object_t BLINK_Object_decodeCompact(blink_pool_t pool, blink_schema_t schema, blink_stream_t in);

/** @} */

#endif
