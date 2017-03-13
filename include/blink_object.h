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
 * @param[in] symbol
 *
 * @return true if successful
 *
 * */
bool BLINK_Object_setEnum(blink_object_t group, const char *fieldName, const char *symbol);

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
 * @param[in] mantissa
 * @param[in] exponent
 *
 * @return true if successful
 *
 * */
bool BLINK_Object_setDecimal(blink_object_t group, const char *fieldName, int64_t mantissa, int8_t exponent);

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
 * @param[in] str
 * @param[in] len byte length of str
 *
 * @return true if successful
 *
 * */
bool BLINK_Object_setString(blink_object_t group, const char *fieldName, const char *str, uint32_t len);

bool BLINK_Object_setString2(blink_object_t group, const char *fieldName, const char *str);

/** Write binary to field
 *
 * @param[in] group
 * @param[in] fieldName null terminated field name string
 * @param[in] data
 * @param[in] len byte length of data
 *
 * @return true if successful
 *
 * */
bool BLINK_Object_setBinary(blink_object_t group, const char *fieldName, const uint8_t *data, uint32_t len);

/** Write fixed to field
 *
 * @param[in] group
 * @param[in] fieldName null terminated field name string
 * @param[in] data
 * @param[in] value byte length of data
 *
 * @return true if successful
 *
 * */
bool BLINK_Object_setFixed(blink_object_t group, const char *fieldName, const uint8_t *data, uint32_t len);

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
 *
 * @return pointer to enum string
 *
 * */
const char *BLINK_Object_getEnum(blink_object_t group, const char *fieldName);

/** Read boolean from field
 *
 * @param[in] group
 * @param[in] fieldName null terminated field name string
 *
 * @return boolean
 *
 * */
bool BLINK_Object_getBool(blink_object_t group, const char *fieldName);

/** Read decimal from field
 *
 * @param[in] group
 * @param[in] fieldName null terminated field name string
 *
 *
 * */
void BLINK_Object_getDecimal(blink_object_t group, const char *fieldName, int64_t *mantissa, int8_t *exponent);

/** Read Uint from field
 *
 * @param[in] group
 * @param[in] fieldName null terminated field name string
 *
 * @return uint
 *
 * */
uint64_t BLINK_Object_getUint(blink_object_t group, const char *fieldName);

/** Read Int from field
 *
 * @param[in] group
 * @param[in] fieldName null terminated field name string
 *
 * @return int
 *
 * */
int64_t BLINK_Object_getInt(blink_object_t group, const char *fieldName);

/** Read f64 from field
 *
 * @param[in] group
 * @param[in] fieldName null terminated field name string
 *
 * @return f64
 *
 * */
double BLINK_Object_getF64(blink_object_t group, const char *fieldName);

/** Read string from field
 *
 * @param[in] group
 * @param[in] fieldName null terminated field name string
 *
 * */
void BLINK_Object_getString(blink_object_t group, const char *fieldName, const char **str, uint32_t *len);

/** Read binary from field
 *
 * @param[in] group
 * @param[in] fieldName null terminated field name string
 *
 * @return binary string
 *
 * */
void BLINK_Object_getBinary(blink_object_t group, const char *fieldName, const uint8_t **data, uint32_t *len);

/** Read fixed from field
 *
 * @param[in] group
 * @param[in] fieldName null terminated field name string
 *
 * @return fixed size string
 *
 * */
void BLINK_Object_getFixed(blink_object_t group, const char *fieldName, const uint8_t **data, uint32_t *len);

/** Read group from field
 *
 * @param[in] group
 * @param[in] fieldName null terminated field name string
 *
 * @return group
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

bool BLINK_Object_encodeCompact(blink_object_t group, blink_stream_t out);

blink_object_t BLINK_Object_decodeCompact(blink_stream_t in, blink_schema_t schema, const struct blink_allocator *alloc);

/** @} */

#endif
