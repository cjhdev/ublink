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
 * Convert tokens to a schema.
 * 
 * @{
 * */

#ifdef __cplusplus
extern "C" {
#endif

/* includes ***********************************************************/

#include <stdint.h>
#include <stdbool.h>

/* definitions ********************************************************/

#ifndef BLINK_INHERIT_DEPTH
    /** maximum inheritence depth */
    #define BLINK_INHERIT_DEPTH   10U
#endif

/* typdefs ************************************************************/

/** a calloc-like function */
typedef void *(* fn_blink_calloc_t)(size_t, size_t);

/** a free-like function */
typedef void (* fn_blink_free_t)(void *);

/* enums **************************************************************/

/** A field shall represent one of the following types */
enum blink_type_tag {
    TYPE_STRING = 0,        /**< UTF8 encoded string */
    TYPE_BINARY,            /**< octet string */
    TYPE_FIXED,             /**< fixed size string */
    TYPE_BOOL,              /**< boolean */
    TYPE_U8,                /**< 8 bit unsigned integer */
    TYPE_U16,               /**< 16 bit unsigned integer */
    TYPE_U32,               /**< 32 bit unsigned integer */
    TYPE_U64,               /**< 64 bit unsigned integer */
    TYPE_I8,                /**< 8 bit signed integer */
    TYPE_I16,               /**< 16 bit signed integer */
    TYPE_I32,               /**< 32 bit signed integer */
    TYPE_I64,               /**< 64 bit signed integer */
    TYPE_F64,               /**< IEEE 754 double */
    TYPE_DATE,              
    TYPE_TIME_OF_DAY_MILLI,
    TYPE_TIME_OF_DAY_NANO,
    TYPE_NANO_TIME,
    TYPE_MILLI_TIME,        
    TYPE_DECIMAL,           /**< 8 bit signed integer exponent, 64 bit signed integer mantissa */
    TYPE_OBJECT,            /**< any group encoded as dynamic group */
    TYPE_ENUM,              /**< 32 bit signed integer */
    TYPE_REF,               /**< reference */

    TYPE_STATIC_GROUP,      /**< static reference to a group */
    TYPE_DYNAMIC_REF        /**< dynamic reference to a group */
};

/* structs ************************************************************/

struct blink_namespace;     /**< forward declaration */
struct blink_list_element;  /**< forward declaration */
struct blink_group;         /**< forward declaration */
struct blink_field;         /**< forward declaration */
struct blink_enum;          /**< forward declaration */

/** schema */
struct blink_schema {
    struct blink_list_element *ns; /**< a schema has zero or more namespace definitions */
    bool finalised;             /**< when true no more schema definitions can be appended */
    fn_blink_calloc_t calloc;   /**< pointer to a calloc-like function */
    fn_blink_free_t free;       /**< pointer to a free-like function */
};

/** A field iterator stores state required to iterate through all fields of a group (including any inherited fields) */
struct blink_field_iterator {
    const struct blink_list_element *field[BLINK_INHERIT_DEPTH];    /**< stack of pointers to fields within groups (inheritence is limited to MAX_DEPTH) */
    uint16_t depth;                                                 /**< current depth in `field` */
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
struct blink_schema *BLINK_InitSchema(struct blink_schema *schema, fn_blink_calloc_t calloc, fn_blink_free_t free);

/** Destroy a schema object
 *
 * @note this function has no effect if `free` was not provided in the call to BLINK_NewSchema
 *
 * @param[in] self receiver
 *
 * */
void BLINK_DestroySchema(struct blink_schema *self);

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
struct blink_schema *BLINK_Parse(struct blink_schema *self, const char *in, size_t inLen);


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
const struct blink_group *BLINK_GetGroupByName(struct blink_schema *self, const char *qName, size_t qNameLen);

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
const struct blink_group *BLINK_GetGroupByID(struct blink_schema *self, uint64_t id);

/** Initialise a field iterator
 *
 * @param[in] iter iterator
 * @param[in] group group to iterate
 *
 * @return pointer to initialised field iterator
 * 
 * @retval NULL could not initialise field iterator
 * 
 * */
const struct blink_field_iterator *BLINK_InitFieldIterator(struct blink_field_iterator *iter, const struct blink_group *group);

/** Get next field from an iterator
 *
 * @param[in] self receiver
 * 
 * @return pointer to receiver
 * 
 * @retval NULL no next field
 *
 * */
const struct blink_field *BLINK_NextField(struct blink_field_iterator *self);

/** Get the name of a group
 *
 * @param[in] self receiver
 * @param[out] nameLen byte length of group name
 * 
 * @return pointer to name string
 *
 * */
const char *BLINK_GetGroupName(const struct blink_group *self, size_t *nameLen);

/** Get the super group definition for this group
 *
 * @param[in] self receiver
 *
 * @return pointer to super group
 * 
 * @retval NULL super group does not exist
 *
 * */
const struct blink_group *BLINK_GetSuperGroup(const struct blink_group *self);

/** Get the name of this field
 *
 * @param[in] self receiver
 * @param[out] nameLen byte length of field name
 *
 * @return pointer to name string
 *
 * */
const char *BLINK_GetFieldName(const struct blink_field *self, size_t *nameLen);

/** Is this field optional?
 *
 * @param[in] self receiver
 *
 * @return optional?
 * 
 * @retval true
 * @retval false
 *
 * */
bool BLINK_FieldIsOptional(const struct blink_field *self);

/** Get the type of this field
 *
 * @param[in] self receiver
 * 
 * @return field type
 * 
 * */
enum blink_type_tag BLINK_GetFieldType(const struct blink_field *self);

/** Get the size of this field (if applicable)
 *
 * @note applicable if #TYPE_FIXED, #TYPE_BINARY, or #TYPE_STRING
 * @note for #TYPE_STRING and #TYPE_BINARY size means maximum size
 *
 * @param[in] self receiver
 * 
 * @return size of field
 * 
 * */
uint32_t BLINK_GetFieldSize(const struct blink_field *self);

/** Get the type referencestring of this field (if applicable)
 *
 * @note applicable if #TYPE_REF or #TYPE_DYNAMIC_REF
 *
 * @param[in] self receiver
 * @param[out] refLen byte length of reference string
 *
 * @return pointer to reference string
 * 
 * @retval NULL this field is not a #TYPE_REF or #TYPE_DYNAMIC_REF
 *
 * */
const char *BLINK_GetFieldRef(const struct blink_field *self, size_t *refLen);

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
const struct blink_symbol *BLINK_GetSymbolValue(const struct blink_enum *self, const char *name, size_t nameLen, int32_t *value);

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
const struct blink_symbol *BLINK_GetSymbolName(const struct blink_enum *self, int32_t value, const char **name, size_t *nameLen);

/** returns field type
 *
 * @param[in] self receiver
 *
 * @return type of field
 *
 * */
const struct blink_type *BLINK_GetType(const struct blink_field *self);

#ifdef __cplusplus
}
#endif


/** @} */
#endif
