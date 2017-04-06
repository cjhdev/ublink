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
 * Create a schema object from schema syntax:
 *
 * @code
 * const char syntax[] =
 *      "InsertOrder/1 ->\n"
 *      "   string Symbol,\n"
 *      "   string OrderId,\n"
 *      "   u32 Price,\n"
 *      "   u32 Quantity\n";
 *
 * blink_schema_t schema = BLINK_Schema_new((struct blink_allocator){.calloc = calloc, .free = free}, syntax, sizeof(syntax));
 * @endcode
 *
 * To operate on the "InsertOrder" group we need to get a reference to
 * it:
 *
 * @code
 * blink_schema_t group = BLINK_Schema_getGroupByName(schema, "InsertOrder");
 * @endcode
 *
 * It's also possible to get a reference to "InsertOrder" by its group
 * ID:
 *
 * @code
 * blink_schema_t group = BLINK_Schema_getGroupByID(schema, 1U);
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
 * To access the fields of a group you will need a field iterator. An iterator
 * is composed of an array of schema references, one entry for this group plus any
 * inherited supergroups. Where the memory is stored depends on your requirements
 * for the lifetime of the iterator - below we use a VLA.
 * 
 * @code
 * size_t stackDimension = BLINK_Group_numberOfSuperGroup(group)+1U;
 * blink_schema_t stack[stackDimension];
 * struct blink_field_iterator iter = BLINK_FieldIterator_init(stack, stackDimension, group);
 * @endcode
 *
 * Below is an example of accessing field definitions sequentally using
 * the iterator:
 *
 * @code
 * // this call does not advance the iterator state
 * blink_schema_t field = BLINK_FieldIterator_peek(&iter);
 *
 * // these calls advance the iterator state 
 * blink_schema_t fieldSymbol = BLINK_FieldIterator_next(&iter);
 * blink_schema_t fieldOrderID = BLINK_FieldIterator_next(&iter);
 * blink_schema_t fieldPrice = BLINK_FieldIterator_next(&iter);
 * blink_schema_t fieldQuantity = BLINK_FieldIterator_next(&iter);
 *
 * // the iterator will be exhausted after returning all fields
 * assert(BLINK_FieldIterator_peek(&iter) == NULL);
 * @endcode
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

/* types **************************************************************/

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

struct blink_stream;
typedef struct blink_stream * blink_stream_t;

struct blink_schema;

struct blink_allocator;

/** this type refers to any immutable schema object */
typedef struct blink_schema * blink_schema_t;

/** A field iterator stores state required to iterate through all fields of a group (including any inherited fields) */
struct blink_field_iterator {
    blink_schema_t *field;      /**< stack of pointers to fields within groups */
    size_t index;               /**< current index in `field` */
};

struct blink_group_iterator {
    struct blink_schema *ns;    /**< namespace pointer */
    struct blink_schema *def;   /**< definition pointer */
};

/* function prototypes ************************************************/

/** Create a new schema object from schema syntax object
 *
 * @param[in] alloc allocator
 * @param[in] in schema syntax stream
 * @return schema
 * @retval NULL
 *
 * */
blink_schema_t BLINK_Schema_new(const struct blink_allocator *alloc, blink_stream_t in);

/** Find group by name
 *
 * @param[in] self
 * @param[in] name null terminated name string
 * @return group
 * @retval NULL group not found
 *
 * */
blink_schema_t BLINK_Schema_getGroupByName(blink_schema_t self, const char *name);

/** Find a group by ID
 * 
 * @param[in] self
 * @param[in] id group ID
 * @return group
 * @retval NULL group not found
 *
 * */
blink_schema_t BLINK_Schema_getGroupByID(blink_schema_t schema, uint64_t id);

const char *BLINK_Namespace_getName(blink_schema_t self);

/** Get group name (within namespace)
 * 
 * @param[in] self
 * @return null terminated name
 *
 * */ 
const char *BLINK_Group_getName(blink_schema_t self);

struct blink_group_iterator BLINK_GroupIterator_init(blink_schema_t schema);
blink_schema_t BLINK_GroupIterator_next(struct blink_group_iterator *iter);

/** Get group namespace
 * 
 * @param[in] self
 * @return namespace
 *
 * */ 
blink_schema_t BLINK_Group_getNamespace(blink_schema_t self);

/** Get group ID
 *
 * @note only valid if BLINK_GroupHasID returns true
 * 
 * @param[in] self
 * @return ID
 *
 * */
uint64_t BLINK_Group_getID(blink_schema_t self);

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
bool BLINK_Group_hasID(blink_schema_t self);

/** Return number of fields in this group
 *
 * @param[in] self
 * @return number of fields
 *
 * */
size_t BLINK_Group_numberOfFields(blink_schema_t self);

/** Get field name
 * 
 * @param[in] self
 * @return null terminated string
 *
 * */
const char *BLINK_Field_getName(blink_schema_t self);

/** Discover if field is optional
 *
 * @param[in] self
 * @return Is field optional?
 * @retval true
 * @retval false
 *
 * */
bool BLINK_Field_isOptional(blink_schema_t self);

/** Discover if field is a sequence 
 *
 * @param[in] self
 * @return Is field a sequence?
 * @retval true
 * @retval false
 *
 * */
bool BLINK_Field_isSequence(blink_schema_t self);

/** Get field type
 *
 * @param[in] self
 * @return field type
 * 
 * */
enum blink_type_tag BLINK_Field_getType(blink_schema_t self);

/** Get field size
 *
 * @note relevant if field type is #BLINK_TYPE_FIXED, #BLINK_TYPE_BINARY, or #BLINK_TYPE_STRING
 * 
 * @param[in] self
 * @return field size
 * 
 * */
uint32_t BLINK_Field_getSize(blink_schema_t self);


/** Get group (nested within field)
 *
 * @note relevant if field type is #BLINK_TYPE_DYNAMIC_GROUP or #BLINK_TYPE_STATIC_GROUP
 * 
 * @param[in] self
 * @return group
 * @retval NULL field type is not #BLINK_TYPE_DYNAMIC_GROUP or #BLINK_TYPE_STATIC_GROUP
 *
 * */
blink_schema_t BLINK_Field_getGroup(blink_schema_t self);

/** Get enum (nested within field)
 *
 * @note relevant if field type is #BLINK_TYPE_ENUM
 *
 * @param[in] self
 * @return enum
 * @retval NULL field type is not #BLINK_TYPE_ENUM
 *
 * */
blink_schema_t BLINK_Field_getEnum(blink_schema_t self);

/** Test if self is group or a subclass of group
 * 
 * @param[in] self group object
 * @param[in] group
 * @return is self a kind of group?
 * @retval true
 * @retval false
 *
 * */ 
bool BLINK_Group_isKindOf(blink_schema_t self, blink_schema_t group);

/** Find enum symbol by name
 *
 * @param[in] self enum object
 * @param[in] name null terminated name string
 * @return symbol
 * @retval NULL symbol not found
 * 
 * */
blink_schema_t BLINK_Enum_getSymbolByName(blink_schema_t self, const char *name);

/** Find enum symbol by value
 *
 * @param[in] self enum object
 * @param[in] value
 * @return symbol
 * @retval NULL symbol not found
 * 
 * */
blink_schema_t BLINK_Enum_getSymbolByValue(blink_schema_t self, int32_t value);

/** Get symbol name
 *
 * @param[in] self
 * @return NULL null terminated string
 *
 * */
const char *BLINK_Symbol_getName(blink_schema_t self);

/** Get symbol value
 * 
 * @param[in] self
 * @return signed 32 bit integer
 *
 * */
int32_t BLINK_Symbol_getValue(blink_schema_t self);

/** Call handler for each field in group
 *
 * @param[in] self
 * @param[in] block this handler will be called for each field
 * @param[in] user this pointer will be passed through to handler
 * 
 * */
void BLINK_Group_eachField(blink_schema_t group, bool (*block)(blink_schema_t group, blink_schema_t field, void *user), void *user);


/** Create field iterator object
 *
 * Use the returned object to iterate through group fields.
 *
 * @param[in] stack an array of references which must be maintained
 *                  for same lifetime as iterator object
 * 
 * @param[in] depth dimension of `stack`
 *
 * To iterate through all inherited fields `depth` must be equal to 
 * (BLINK_Group_getAncestorCount(self) + 1U).
 *
 * To iterate only through the subgroup `depth` must be 1U.

 * @return field iterator
 * 
 * */
struct blink_field_iterator BLINK_FieldIterator_init(blink_schema_t *stack, size_t depth, blink_schema_t group);

/** Get next field from a field iterator but do not change the iterator state
 * 
 * @param[in] self
 * @return field
 * @retval NULL no next field in group
 *
 * */
blink_schema_t BLINK_FieldIterator_peek(struct blink_field_iterator *self);

/** Get next field from a field iterator
 * 
 * @param[in] self
 * @return field
 * @retval NULL no next field in group
 *
 * */
blink_schema_t BLINK_FieldIterator_next(struct blink_field_iterator *self);

/** Return the number of supergroups this group inherits from
 *
 * @param[in] self group
 * @return number of supergroups
 * 
 * */
size_t BLINK_Group_numberOfSuperGroup(blink_schema_t self);

#ifdef __cplusplus
}
#endif


/** @} */
#endif
