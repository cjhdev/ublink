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

#include "blink_alloc.h"

/* types **************************************************************/

struct blink_object;
struct blink_stream;
struct blink_schema;

typedef struct blink_object * blink_object_t;
typedef struct blink_stream * blink_stream_t;
typedef struct blink_schema * blink_schema_t;

union blink_object_value {
    bool boolean;   
    uint64_t u64;   /**< unsigned integer */
    int64_t i64;    /**< signed integer */
    double f64;
    struct blink_string {
        const uint8_t *data;
        uint32_t len;
    } string;       /**< binary, string, and fixed */
    const char *enumeration;
    struct blink_decimal {
        int64_t mantissa;   
        int8_t exponent;
    }decimal;
    blink_object_t group;   /**< static and dynamic groups */
};

/* functions **********************************************************/

/** Create a new group model from a group definition
 *
 * @param[in] allocator
 * @param[in] group group definition
 *
 * @return group model
 *
 * @retval NULL could not create group model
 *
 * */
blink_object_t BLINK_Object_newGroup(const struct blink_allocator *alloc, blink_schema_t group);

/** Clear a field (i.e. set to NULL)
 *
 * @param[in] group
 * @param[in] fieldName null terminated field name string
 * 
 * @return true if successful
 *
 * */
bool BLINK_Object_clear(blink_object_t group, const char *fieldName);

/** Set a field value
 *
 * @note not type safe
 * @note if type safety is required, use one of the specialised set functions
 *
 * @param[in] group
 * @param[in] fieldName null terminated field name string
 * @param[in] value
 *
 * @return true if successful
 *
 * */
bool BLINK_Object_set(blink_object_t group, const char *fieldName, const union blink_object_value *value);

/** Get a field value
 *
 * @note not type safe
 * @note if type safety is required, use one of the specialised get functions
 *
 * @param[in] group
 * @param[in] fieldName null terminated field name string
 * 
 * @return value
 *
 * @note if field is NULL it will still return a default value equivalent to zero or NULL
 *
 * */
union blink_object_value BLINK_Object_get(blink_object_t group, const char *fieldName);

/** Test if a field value is NULL
 *
 * @param[in] group
 * @param[in] fieldName null terminated field name string
 *
 * @return true if field value is NULL
 *
 * */
bool BLINK_Object_fieldIsNull(blink_object_t group, const char *fieldName);

/** Write enum to field
 *
 * @param[in] group
 * @param[in] fieldName null terminated field name string
 * @param[in] value enum
 *
 * @return true if successful
 *
 * */
bool BLINK_Object_setEnum(blink_object_t group, const char *fieldName, const char *value);

/** Write boolean to field
 *
 * @param[in] group
 * @param[in] fieldName null terminated field name string
 * @param[in] value boolean
 *
 * @return true if successful
 *
 * */
bool BLINK_Object_setBool(blink_object_t group, const char *fieldName, bool value);

/** Write decimal to field
 *
 * @param[in] group
 * @param[in] fieldName null terminated field name string
 * @param[in] value decimal
 *
 * @return true if successful
 *
 * */
bool BLINK_Object_setDecimal(blink_object_t group, const char *fieldName, struct blink_decimal *value);

/** Write an unsigned integer to field
 *
 * @param[in] group
 * @param[in] fieldName null terminated field name string
 * @param[in] value
 *
 * @return true if successful
 *
 * */
bool BLINK_Object_setUint(blink_object_t group, const char *fieldName, uint64_t value);

/** Write a signed integer to field
 *
 * @param[in] group
 * @param[in] fieldName null terminated field name string
 * @param[in] value
 *
 * @return true if successful
 *
 * */
bool BLINK_Object_setInt(blink_object_t group, const char *fieldName, int64_t value);

/** Write f64 to field
 *
 * @param[in] group
 * @param[in] fieldName null terminated field name string
 * @param[in] value f64
 *
 * @return true if successful
 *
 * */
bool BLINK_Object_setF64(blink_object_t group, const char *fieldName, double value);

/** Write string to field
 *
 * @param[in] group
 * @param[in] fieldName null terminated field name string
 * @param[in] value string
 *
 * @return true if successful
 *
 * */
bool BLINK_Object_setString(blink_object_t group, const char *fieldName, struct blink_string *value);

/** Write binary to field
 *
 * @param[in] group
 * @param[in] fieldName null terminated field name string
 * @param[in] value binary
 *
 * @return true if successful
 *
 * */
bool BLINK_Object_setBinary(blink_object_t group, const char *fieldName, struct blink_string *value);

/** Write fixed to field
 *
 * @param[in] group
 * @param[in] fieldName null terminated field name string
 * @param[in] value fixed
 *
 * @return true if successful
 *
 * */
bool BLINK_Object_setFixed(blink_object_t group, const char *fieldName, struct blink_string *value);

/** Write group to field
 *
 * @param[in] group
 * @param[in] fieldName null terminated field name string
 * @param[in] value group
 *
 * @return true if successful
 *
 * */
bool BLINK_Object_setGroup(blink_object_t group, const char *fieldName, blink_object_t value);

/** Test if field value is NULL
 *
 * @param[in] group
 * @param[in] fieldName null terminated field name string
 *
 * @return true if field value is NULL
 *
 * */
bool BLINK_Object_fieldIsNull(blink_object_t group, const char *fieldName);

/** Read enum from field
 *
 * @param[in] group
 * @param[in] fieldName null terminated field name string
 * @param[out] value length of enum 
 * @param[out] valueLen length of enum value
 *
 * @return true if value can be read
 *
 * */
struct blink_string BLINK_Object_getEnum(blink_object_t group, const char *fieldName);

/** Read boolean from field
 *
 * @param[in] group
 * @param[in] fieldName null terminated field name string
 * @param[out] value boolean
 *
 * @return true if value can be read
 *
 * */
bool BLINK_Object_getBool(blink_object_t group, const char *fieldName);

/** Read decimal from field
 *
 * @param[in] group
 * @param[in] fieldName null terminated field name string
 * @param[out] value boolean
 *
 * @return true if value can be read
 *
 * */
struct blink_decimal BLINK_Object_getDecimal(blink_object_t group, const char *fieldName);

/** Read Uint from field
 *
 * @param[in] group
 * @param[in] fieldName null terminated field name string
 * @param[out] value u8
 *
 * @return true if value can be read
 *
 * */
uint64_t BLINK_Object_getUint(blink_object_t group, const char *fieldName);

/** Read Int from field
 *
 * @param[in] group
 * @param[in] fieldName null terminated field name string
 * @param[out] value i8
 *
 * @return true if value can be read
 *
 * */
int64_t BLINK_Object_getInt(blink_object_t group, const char *fieldName);

/** Read f64 from field
 *
 * @param[in] group
 * @param[in] fieldName null terminated field name string
 * @param[out] value f64
 *
 * @return true if value can be read
 *
 * */
double BLINK_Object_getF64(blink_object_t group, const char *fieldName);

/** Read string from field
 *
 * @param[in] group
 * @param[in] fieldName null terminated field name string
 * @param[out] value string
 *
 * @return true if value can be read
 *
 * */
struct blink_string BLINK_Object_getString(blink_object_t group, const char *fieldName);

/** Read binary from field
 *
 * @param[in] group
 * @param[in] fieldName null terminated field name string
 * @param[out] value binary
 *
 * @return true if value can be read
 *
 * */
struct blink_string  BLINK_Object_getBinary(blink_object_t group, const char *fieldName);

/** Read fixed from field
 *
 * @param[in] group
 * @param[in] fieldName null terminated field name string
 * @param[out] value fixed
 *
 * @return true if value can be read
 *
 * */
struct blink_string  BLINK_Object_getFixed(blink_object_t group, const char *fieldName);

/** Read group from field
 *
 * @param[in] group
 * @param[in] fieldName null terminated field name string
 * @param[out] value group
 *
 * @return true if value can be read
 *
 * */
blink_object_t BLINK_Object_getGroup(blink_object_t group, const char *fieldName);

/** Read field definition from field
 *
 * @param[in] group
 * @param[in] fieldName null terminated field name string
 *
 * @return field definition
 *
 * */
blink_schema_t BLINK_Object_getFieldDefinition(blink_object_t group, const char *fieldName);

/** @} */

#endif
