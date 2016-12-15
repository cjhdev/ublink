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

#ifndef BLINK_PARSER_H
#define BLINK_PARSER_H

/**
 * @defgroup blink_parser
 *
 * Use this interface to convert a string into a blink_schema instance.
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

/* definitions ********************************************************/

#ifndef BLINK_INHERIT_DEPTH
    /** maximum inheritence depth */
    #define BLINK_INHERIT_DEPTH   10U
#endif

/* typdefs ************************************************************/

/** A calloc-like function
 *
 * @param[in] nelem number of elements
 * @param[in] elsize byte size of an element
 * @return a pointer to the allocated memory
 * @retval NULL allocation failed
 * 
 * */
typedef void *(* fn_blink_calloc_t)(size_t nelem, size_t elsize);

/** A free-like function
 *
 * @param[in] ptr pointer to memory previously allocated
 *
 * */
typedef void (* fn_blink_free_t)(void *ptr);

/* enums **************************************************************/

/** A field shall represent one of the following types */
enum blink_type_tag {
    BLINK_TYPE_STRING = 0,        /**< UTF8 encoded string */
    BLINK_TYPE_BINARY,            /**< octet string */
    BLINK_TYPE_FIXED,             /**< fixed size string */
    BLINK_TYPE_BOOL,              /**< boolean */
    BLINK_TYPE_U8,                /**< 8 bit unsigned integer */
    BLINK_TYPE_U16,               /**< 16 bit unsigned integer */
    BLINK_TYPE_U32,               /**< 32 bit unsigned integer */
    BLINK_TYPE_U64,               /**< 64 bit unsigned integer */
    BLINK_TYPE_I8,                /**< 8 bit signed integer */
    BLINK_TYPE_I16,               /**< 16 bit signed integer */
    BLINK_TYPE_I32,               /**< 32 bit signed integer */
    BLINK_TYPE_I64,               /**< 64 bit signed integer */
    BLINK_TYPE_F64,               /**< IEEE 754 double */
    BLINK_TYPE_DATE,              
    BLINK_TYPE_TIME_OF_DAY_MILLI,
    BLINK_TYPE_TIME_OF_DAY_NANO,
    BLINK_TYPE_NANO_TIME,
    BLINK_TYPE_MILLI_TIME,        
    BLINK_TYPE_DECIMAL,           /**< 8 bit signed integer exponent, 64 bit signed integer mantissa */
    BLINK_TYPE_OBJECT,            /**< any group encoded as dynamic group */
    BLINK_TYPE_ENUM,              /**< 32 bit signed integer */
    BLINK_TYPE_STATIC_GROUP,      /**< static group */
    BLINK_TYPE_DYNAMIC_GROUP      /**< dynamic group */
};

/* structs ************************************************************/

struct blink_namespace;     /**< forward declaration */
struct blink_list_element;  /**< forward declaration */
struct blink_group;         /**< forward declaration */
struct blink_field;         /**< forward declaration */
struct blink_enum;          /**< forward declaration */

/** schema */
struct blink_schema {
    struct blink_list_element *ns;  /**< a schema has zero or more namespace definitions */
    bool finalised;                 /**< when true no more schema definitions can be appended */
    fn_blink_calloc_t calloc;       /**< pointer to a calloc-like function */
    fn_blink_free_t free;           /**< pointer to a free-like function */
    struct blink_element *elements; /**< used to keep track of everything allocated */
};

/** A field iterator stores state required to iterate through all fields of a group (including any inherited fields) */
struct blink_field_iterator {
    struct blink_list_element *field[BLINK_INHERIT_DEPTH];  /**< stack of pointers to fields within groups */
    uint16_t depth;                                         /**< current depth in `field` */
};

/* function prototypes ************************************************/

/** Initialise a schema object
 *
 * @note Applications must provide a calloc-like function
 * @note Applications may provide a free-like function
 *
 * @param[in] schema
 * @param[in] calloc a function like calloc() (MANDATORY)
 * @param[in] free a function like free free() (OPTIONAL)
 *
 * @return pointer to initialised schema
 *
 * */
struct blink_schema *BLINK_SchemaInit(struct blink_schema *schema, fn_blink_calloc_t calloc, fn_blink_free_t free);

/** Destroy a schema object
 *
 * @note this function has no effect if `free` was not provided in the call to BLINK_NewSchema
 *
 * @param[in] self receiver
 *
 * */
void BLINK_SchemaDestroy(struct blink_schema *self);

/** Parse a single Blink Protocol schema definition
 *
 * @note finalises the schema representation
 *
 * @param[in] self receiver
 * @param[in] in Blink Protocol schema definition
 * @param[in] inLen byte length of `in`
 *
 * @return pointer to receiver
 * 
 * @retval NULL parse failed
 *
 * */
struct blink_schema *BLINK_SchemaParse(struct blink_schema *self, const char *in, size_t inLen);


/** Find a group by name
 *
 * @param[in] self receiver
 * @param[in] qName qualified group name
 * @param[in] qNameLen byte length of `qName`
 *
 * @return pointer to receiver
 * 
 * @retval NULL group not found
 *
 * */
const struct blink_group *BLINK_SchemaGetGroupByName(const struct blink_schema *self, const char *qName, size_t qNameLen);

/** Find a group by ID
 * 
 * @param[in] self receiver
 * @param[in] id group ID
 *
 * @return pointer to receiver
 * 
 * @retval NULL group not found
 *
 * */
const struct blink_group *BLINK_SchemaGetGroupByID(const struct blink_schema *self, uint64_t id);

/** Initialise a field iterator
 *
 * @param[in] iter iterator
 * @param[in] group group to iterate
 * 
 * */
void BLINK_FieldIteratorInit(struct blink_field_iterator *iter, const struct blink_group *group);

/** Get next field from an iterator
 *
 * @param[in] self receiver
 * 
 * @return pointer to receiver
 * 
 * @retval NULL no next field
 *
 * */
const struct blink_field *BLINK_FieldIteratorNext(struct blink_field_iterator *self);

/** Get the name of a group
 *
 * @param[in] self receiver
 * @param[out] nameLen byte length of group name
 * 
 * @return pointer to name string
 *
 * */
const char *BLINK_GroupGetName(const struct blink_group *self, size_t *nameLen);

/** Get the name of this field
 *
 * @param[in] self receiver
 * @param[out] nameLen byte length of field name
 *
 * @return pointer to name string
 *
 * */
const char *BLINK_FieldGetName(const struct blink_field *self, size_t *nameLen);

/** 
 * @param[in] self receiver
 * @return Is this field optional?
 * @retval true
 * @retval false
 *
 * */
bool BLINK_FieldGetIsOptional(const struct blink_field *self);

/** 
 * @param[in] self receiver
 * @return Is this field a sequence?
 * @retval true
 * @retval false
 *
 * */
bool BLINK_FieldGetIsSequence(const struct blink_field *self);

/** Get the type of this field
 *
 * @param[in] self receiver
 * 
 * @return field type
 * 
 * */
enum blink_type_tag BLINK_FieldGetType(const struct blink_field *self);

/** Get the size of this field (if applicable)
 *
 * @note applicable if #BLINK_TYPE_FIXED, #BLINK_TYPE_BINARY, or #BLINK_TYPE_STRING
 * @note for #BLINK_TYPE_STRING and #BLINK_TYPE_BINARY size means maximum size
 *
 * @param[in] self receiver
 * 
 * @return size of field
 * 
 * */
uint32_t BLINK_FieldGetSize(const struct blink_field *self);


/** Return group for field type
 * 
 * @param[in] self receiver
 *
 * @return pointer to group definition
 *
 * @retval NULL field type is not a static or dynamic group
 *
 * */
const struct blink_group *BLINK_FieldGetGroup(const struct blink_field *self);

/** Return enum for field type
 *
 * @param[in] self receiver
 *
 * @return pointer to enum definition
 *
 * @retval NULL field type is not an enum
 *
 * */
const struct blink_enum *BLINK_FieldGetEnum(const struct blink_field *self);

/** test if self is group or a subclass of group
 * 
 * @param[in] self receiver
 * @param[in] group
 *
 * @return is self a group or subclass of group?
 *
 * */ 
bool BLINK_GroupIsKindOf(const struct blink_group *self, const struct blink_group *group);

/**
 * @param[in] self receiver
 * @param[in] name symbol name
 * @param[in] nameLen byte length of `name`
 * @param[out] value symbol value
 *
 * @return pointer to symbol
 * 
 * @retval NULL symbol not found
 * 
 * */
const struct blink_symbol *BLINK_EnumGetSymbolByName(const struct blink_enum *self, const char *name, size_t nameLen);

/**
 * @param[in] self receiver
 * @param[in] value symbol value
 * @param[out] name symbol name
 * @param[out] nameLen byte length of `name`
 *
 * @return pointer to symbol
 * 
 * @retval NULL symbol not found
 * 
 * */
const struct blink_symbol *BLINK_EnumGetSymbolByValue(const struct blink_enum *self, int32_t value);

const char *BLINK_SymbolGetName(const struct blink_symbol *self, size_t *nameLen);

int32_t BLINK_SymbolGetValue(const struct blink_symbol *self);

#ifdef __cplusplus
}
#endif


/** @} */
#endif
