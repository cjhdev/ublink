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

#ifndef BLINK_SCHEMA_H
#define BLINK_SCHEMA_H

/**
 * @defgroup blink_schema blink_schema
 * @ingroup ublink
 *
 * Functions in this module convert schema syntax into schema objects.
 *
 * ## Example Workflow
 *
 * First initialise a heap that will store the schema objects:
 *
 * @code
 * uint8_t heap[1024U];
 * struct blink_pool pool_state;
 * blink_pool_t pool = BLINK_Pool_init(&pool, heap, sizeof(heap));
 * @endcode
 *
 * Next create a schema object from schema syntax:
 *
 * @code
 * const char syntax[] =
 *      "InsertOrder/1 ->\n"
 *      "   string Symbol,\n"
 *      "   string OrderId,\n"
 *      "   u32 Price,\n"
 *      "   u32 Quantity\n";
 *
 * blink_schema_t schema = BLINK_Schema_new(pool, syntax, sizeof(syntax));
 * @endcode
 *
 * To operate on the "InsertOrder" group we need to get a reference to
 * it:
 *
 * @code
 * blink_group_t group = BLINK_Schema_getGroupByName(schema, "InsertOrder");
 * @endcode
 *
 * It's also possible to get a reference to "InsertOrder" by its group
 * ID:
 *
 * @code
 * blink_group_t group = BLINK_Schema_getGroupByID(schema, 1U);
 * @endcode
 *
 * While not very useful in this example, it is possible to read attributes
 * from a group like so:
 *
 * @code
 * // will return "InsertOrder"
 * BLINK_Group_getName(group);
 *
 * // will return 1
 * BLINK_Group_getID(group);
 * @endcode
 *
 * 
 * 
 * 
 * @{
 * */

#ifdef __cplusplus
extern "C" {
#endif

/* includes ***********************************************************/

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include "blink_pool.h"

/* definitions ********************************************************/

#ifndef BLINK_INHERIT_DEPTH
    /** maximum inheritence depth */
    #define BLINK_INHERIT_DEPTH   10U
#endif

/* enums **************************************************************/

/** A field shall represent one of the following types */
enum blink_type_tag {
    BLINK_TYPE_STRING = 0,          /**< UTF8 encoded string */
    BLINK_TYPE_BINARY,              /**< octet string */
    BLINK_TYPE_FIXED,               /**< fixed size string */
    BLINK_TYPE_BOOL,                /**< boolean */
    BLINK_TYPE_U8,                  /**< 8 bit unsigned integer */
    BLINK_TYPE_U16,                 /**< 16 bit unsigned integer */
    BLINK_TYPE_U32,                 /**< 32 bit unsigned integer */
    BLINK_TYPE_U64,                 /**< 64 bit unsigned integer */
    BLINK_TYPE_I8,                  /**< 8 bit signed integer */
    BLINK_TYPE_I16,                 /**< 16 bit signed integer */
    BLINK_TYPE_I32,                 /**< 32 bit signed integer */
    BLINK_TYPE_I64,                 /**< 64 bit signed integer */
    BLINK_TYPE_F64,                 /**< IEEE 754 double */
    BLINK_TYPE_DATE,                /**< days since 2000-01-01 */
    BLINK_TYPE_TIME_OF_DAY_MILLI,   /**< milliseconds since midnight */
    BLINK_TYPE_TIME_OF_DAY_NANO,    /**< nanoseconds since midnight */
    BLINK_TYPE_NANO_TIME,           /**< nanoseconds since (or before) 1970-01-01 00:00:00.000000000*/
    BLINK_TYPE_MILLI_TIME,          /**< milliseconds since (or before) 1970-01-01 00:00:00.000000000 */
    BLINK_TYPE_DECIMAL,             /**< 8 bit signed integer exponent, 64 bit signed integer mantissa */
    BLINK_TYPE_OBJECT,              /**< any group encoded as dynamic group */
    BLINK_TYPE_ENUM,                /**< 32 bit signed integer */
    BLINK_TYPE_STATIC_GROUP,        /**< static group */
    BLINK_TYPE_DYNAMIC_GROUP        /**< dynamic group */
};

/* structs ************************************************************/

struct blink_schema;
struct blink_namespace;
struct blink_list_element;
struct blink_group;
struct blink_field;
struct blink_enum;

/** A field iterator stores state required to iterate through all fields of a group (including any inherited fields) */
struct blink_field_iterator {
    struct blink_list_element *field[BLINK_INHERIT_DEPTH];  /**< stack of pointers to fields within groups */
    uint16_t depth;                                         /**< current depth in `field` */
};

/* typdefs ************************************************************/

typedef struct blink_schema * blink_partial_schema_t;
typedef const struct blink_schema * blink_schema_t;
typedef const struct blink_group * blink_group_t;
typedef const struct blink_field * blink_field_t;
typedef const struct blink_enum * blink_enum_t;
typedef const struct blink_symbol * blink_symbol_t;
typedef struct blink_list_element * blink_list_element_t;
typedef struct blink_field_iterator * blink_field_iterator_t;

/* function prototypes ************************************************/

/** Create a new schema object from schema syntax
 *
 * @param[in] pool pool to allocate from
 * @param[in] in schema syntax
 * @param[in] inLen byte length of schema syntax
 * @return schema
 * @retval NULL
 *
 * */
blink_schema_t BLINK_Schema_new(blink_pool_t pool, const char *in, size_t inLen);

/** Find group by name
 *
 * @param[in] self
 * @param[in] name null terminated name string
 * @return group
 * @retval NULL group not found
 *
 * */
blink_group_t BLINK_Schema_getGroupByName(blink_schema_t self, const char *name);

/** Find a group by ID
 * 
 * @param[in] self
 * @param[in] id group ID
 * @return group
 * @retval NULL group not found
 *
 * */
blink_group_t BLINK_Schema_getGroupByID(blink_schema_t self, uint64_t id);


/** Get group name
 * 
 * @param[in] self
 * @return null terminated name
 *
 * */
const char *BLINK_Group_getName(blink_group_t self);

/** Get group ID
 *
 * @note only valid if BLINK_GroupHasID returns true
 * 
 * @param[in] self
 * @return ID
 *
 * */
uint64_t BLINK_Group_getID(blink_group_t self);

/** Discover if group has a valid ID
 *
 * @note groups which do not have an ID cannot be serialised
 * as a dynamic group
 * 
 * @param[in] self
 * @return Does group have an ID?
 * @retval true     yes
 * @retval false    no
 *
 * */
bool BLINK_Group_hasID(blink_group_t self);

/** Get field name
 * 
 * @param[in] self
 * @return null terminated string
 *
 * */
const char *BLINK_Field_getName(blink_field_t self);

/** Discover if field is optional
 *
 * @param[in] self
 * @return Is field optional?
 * @retval true
 * @retval false
 *
 * */
bool BLINK_Field_isOptional(blink_field_t self);

/** Discover if field is a sequence 
 *
 * @param[in] self
 * @return Is field a sequence?
 * @retval true
 * @retval false
 *
 * */
bool BLINK_Field_isSequence(blink_field_t self);

/** Get field type
 *
 * @param[in] self
 * @return field type
 * 
 * */
enum blink_type_tag BLINK_Field_getType(blink_field_t self);

/** Get field size
 *
 * @note relevant if field type is #BLINK_TYPE_FIXED, #BLINK_TYPE_BINARY, or #BLINK_TYPE_STRING
 * 
 * @param[in] self
 * @return field size
 * 
 * */
uint32_t BLINK_Field_getSize(blink_field_t self);


/** Get group (nested within field)
 *
 * @note relevant if field type is #BLINK_TYPE_DYNAMIC_GROUP or #BLINK_TYPE_STATIC_GROUP
 * 
 * @param[in] self
 * @return group
 * @retval NULL field type is not #BLINK_TYPE_DYNAMIC_GROUP or #BLINK_TYPE_STATIC_GROUP
 *
 * */
blink_group_t BLINK_Field_getGroup(blink_field_t self);

/** Get enum (nested within field)
 *
 * @note relevant if field type is #BLINK_TYPE_ENUM
 *
 * @param[in] self
 * @return enum
 * @retval NULL field type is not #BLINK_TYPE_ENUM
 *
 * */
blink_enum_t BLINK_Field_getEnum(blink_field_t self);

/** Test if self is group or a subclass of group
 * 
 * @param[in] self
 * @param[in] group
 * @return is self a kind of group?
 * @retval true
 * @retval false
 *
 * */ 
bool BLINK_Group_isKindOf(blink_group_t self, blink_group_t group);

/** Find enum symbol by name
 *
 * @param[in] self
 * @param[in] name null terminated name string
 * @return symbol
 * @retval NULL symbol not found
 * 
 * */
blink_symbol_t BLINK_Enum_getSymbolByName(blink_enum_t self, const char *name);

/** Find enum symbol by value
 *
 * @param[in] self
 * @param[in] value
 * @return symbol
 * @retval NULL symbol not found
 * 
 * */
blink_symbol_t BLINK_Enum_getSymbolByValue(blink_enum_t self, int32_t value);

/** Get symbol name
 *
 * @param[in] self
 * @return NULL null terminated string
 *
 * */
const char *BLINK_Symbol_getName(blink_symbol_t self);

/** Get symbol value
 * 
 * @param[in] self
 * @return signed 32 bit integer
 *
 * */
int32_t BLINK_Symbol_getValue(blink_symbol_t self);

/** Initialise a field iterator
 *
 * @param[in] iter iterator
 * @param[in] group group to iterate
 * @return initialised field iterator
 * 
 * */
blink_field_iterator_t BLINK_FieldIterator_init(struct blink_field_iterator *iter, blink_group_t group);

/** Get next field from a field iterator but do not change the iterator state
 * 
 * @param[in] self
 * @return field
 * @retval NULL no next field in group
 *
 * */
blink_field_t BLINK_FieldIterator_peek(blink_field_iterator_t self);

/** Get next field from a field iterator
 * 
 * @param[in] self
 * @return field
 * @retval NULL no next field in group
 *
 * */
blink_field_t BLINK_FieldIterator_next(blink_field_iterator_t self);

#ifdef __cplusplus
}
#endif


/** @} */
#endif
