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
 * @defgroup blink_parser Parser
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

#ifndef BLINK_LINK_DEPTH
    /** maximum number of reference to reference links */
    #define BLINK_LINK_DEPTH    10U
#endif

/* typdefs ************************************************************/

/** a calloc-like function */
typedef void *(* fn_blink_calloc_t)(size_t, size_t);

/** a free-like function */
typedef void (* fn_blink_free_t)(void *);

/* enums **************************************************************/

/** A #blink_field shall be one of the following types */
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

/** type */
struct blink_type {
    bool isSequence;            /**< this is a SEQUENCE of type */                
    uint32_t size;              /**< size attribute (applicable to #TYPE_BINARY, #TYPE_FIXED, and #TYPE_STRING) */
    const char *ref;            /**< name of reference (applicable to #TYPE_DYNAMIC_REF, #TYPE_REF, #TYPE_ENUM) */
    size_t refLen;              /**< byte length of `ref` */
    enum blink_type_tag tag;    /**< what type is this? */
    const void *resolvedRef;    /**< `ref` resolves to this structure (cast according to `tag`) */
};

/** field */
struct blink_field {
    struct blink_field *next;   /**< next field in group */
    const char *name;           /**< name of this field */
    size_t nameLen;             /**< byte length of `name` */
    bool isOptional;            /**< field is optional */
    struct blink_type type;     /**< field type information */
};

/** group */
struct blink_group {
    struct blink_group *next;       /**< next group in namespace */
    const char *name;               /**< name of this group */
    size_t nameLen;                 /**< byte length of `name` */
    bool hasID;                     /**< group has an ID */
    uint64_t id;                    /**< group ID */
    const char *superGroup;         /**< name of super group */
    size_t superGroupLen;           /**< byte length of supergroup name */
    const struct blink_group *s;    /**< pointer to supergroup definition */
    struct blink_field *f;          /**< fields belonging to group */
};

/** enumeration symbol */
struct blink_symbol {
    struct blink_symbol *next;      /**< next symbol in enum */
    const char *name;               /**< name of symbol */
    size_t nameLen;                 /**< byte length of `name` */
    uint64_t value;                 /**< integer value */
    bool implicitID;                /**< true if `value` is not explicitly defined */
};

/** enumeration */
struct blink_enum {
    struct blink_enum *next;    /**< next enum in namespace */
    const char *name;           /**< name of this field */
    size_t nameLen;             /**< byte length of `name` */
    struct blink_symbol *s;     /**< symbols belonging to enumeration */    
};

/** type definition */
struct blink_type_def {
    struct blink_type_def *next;    /**< next type definition in namespace */
    const char *name;               /**< name of type definition */
    size_t nameLen;                 /**< byte length of `name` */
    struct blink_type type;         /**< type information */
};

/** annotation - key-value meta data */
struct blink_annote {
    struct blink_annote *next;      /**< next annotation in namespace */
    const char *key;                /**< key */
    size_t keyLen;                  /**< byte length of `key` */
    const char *value;              /**< value */            
    size_t valueLen;                /**< byte length of `value` */
    uint64_t number;                /**< sometimes value is a numeric value */
    bool isNumeric;                 /**< true if this annotation is a numeric value */  
};

struct blink_inline_annote {
    struct blink_inline_annote *next;
    const char *name;
    size_t nameLen;
    struct blink_annote *a;         /** list of annotes to apply */
};

/** namespace */
struct blink_namespace {
    struct blink_namespace *next;   /**< next namespace definition in schema */
    const char *name;               /**< name of this namespace */
    size_t nameLen;                 /**< byte length of `name` */
    struct blink_group *groups;     /**< groups defined in namespace */
    struct blink_enum *enums;       /**< enums defined in namespace */        
    struct blink_type_def *types;   /**< types defined in namespace */    
};

/** schema */
struct blink_schema {
    struct blink_namespace *ns; /**< a schema has zero or more namespace definitions */
    bool finalised;             /**< when true no more schema definitions can be appended */
    fn_blink_calloc_t calloc;   /**< pointer to a calloc-like function */
    fn_blink_free_t free;       /**< pointer to a free-like function */
};

/** A field iterator stores state required to iterate through all fields of a group (including any inherited fields) */
struct blink_field_iterator {
    const struct blink_field *field[BLINK_INHERIT_DEPTH];   /**< stack of pointers to fields within groups (inheritence is limited to MAX_DEPTH) */
    uint16_t depth;                                         /**< current depth in `group` */
};

/* function prototypes ************************************************/

/** Create a new schema object
 *
 * @param[in] self schema object
 * @param[in] calloc calloc()
 * @param[in] free free()
 *
 * @return struct blink_schema *
 *
 * */
struct blink_schema *BLINK_NewSchema(struct blink_schema *self, fn_blink_calloc_t calloc, fn_blink_free_t free);

/** Destroy a schema object
 * 
 * @param[in] self schema object
 *
 * */
void BLINK_DestroySchema(struct blink_schema *self);

/** Parse a single Blink Protocol schema definition
 *
 * @note finalises the schema representation
 * 
 * @param[in] self schema object
 * @param[in] in Blink Protocol schema definition
 * @param[in] inLen byte length of `in`
 *
 * @return struct blink_schema *
 * @retval NULL (`in` could not be parsed)
 *
 * */
struct blink_schema *BLINK_Parse(struct blink_schema *self, const char *in, size_t inLen);


/** Find a group by name
 * 
 * @param[in] self schema object
 * @param[in] qName qualified group name
 * @param[in] qNameLen byte length of `qName`
 *
 * @return const struct blink_group *
 * @retval NULL (no group found)
 *
 * */
const struct blink_group *BLINK_GetGroupByName(struct blink_schema *self, const char *qName, size_t qNameLen);

/** Find a group by ID
 * 
 * @param[in] self schema object
 * @param[in] id group ID
 *
 * @return const struct blink_group *
 * @retval NULL (no group found)
 *
 * */
const struct blink_group *BLINK_GetGroupByID(struct blink_schema *self, uint64_t id);

/** Create a new field iterator for a given group
 *
 * @param[in] group group to iterate
 * @param[in] iter iterator state
 *
 * @return const struct blink_field_iterator *
 * @retval NULL (could not initialise iterator)
 * 
 * */
const struct blink_field_iterator *BLINK_NewFieldIterator(const struct blink_group *group, struct blink_field_iterator *iter);

/** Get next field from iterator
 *
 * @param[in] iter field iterator
 * 
 * @return const struct blink_field *
 * @retval NULL (no next field)
 *
 * */
const struct blink_field *BLINK_NextField(struct blink_field_iterator *iter);

/** Get the name of this group
 *
 * @param[in] group group definition
 * @param[out] nameLen byte length of group name
 * 
 * @return const char *
 *
 * */
const char *BLINK_GetGroupName(const struct blink_group *group, size_t *nameLen);

/** Get the super group definition for this group
 *
 * @param[in] group group definition
 *
 * @return const struct blink_group *
 * @retval NULL (this group does not have a super group)
 *
 * */
const struct blink_group *BLINK_GetSuperGroup(const struct blink_group *group);

/** Get the name of this field
 *
 * @param[in] field field definition
 * @param[out] nameLen byte length of field name
 *
 * @return const char *
 *
 * */
const char *BLINK_GetFieldName(const struct blink_field *field, size_t *nameLen);

/** Is this field optional?
 *
 * @param[in] field field definition
 *
 * @return bool
 * @retval true (field is optional)
 * @retval false (field is not optional)
 *
 * */
bool BLINK_FieldIsOptional(const struct blink_field *field);

/** Get the type of this field
 *
 * @param[in] field field definition
 * 
 * @return enum #blink_type_tag
 * 
 * */
enum blink_type_tag BLINK_GetFieldType(const struct blink_field *field);

/** Get the size of this field (if applicable)
 *
 * @note applicable if #TYPE_FIXED, #TYPE_BINARY, or #TYPE_STRING
 * @note for #TYPE_STRING and #TYPE_BINARY size means maximum size
 *
 * @param[in] field field definition
 * 
 * @return uint32_t
 * 
 * */
uint32_t BLINK_GetFieldSize(const struct blink_field *field);

/** Get the type referencestring of this field (if applicable)
 *
 * @note applicable if #TYPE_REF or #TYPE_DYNAMIC_REF
 *
 * @param[in] field field definition
 * @param[out] refLen byte length of reference string
 *
 * @return const char *
 * @retval NULL (this field is not a #TYPE_REF or #TYPE_DYNAMIC_REF)
 *
 * */
const char *BLINK_GetFieldRef(const struct blink_field *field, size_t *refLen);

/**
 * @param[in] e enum definition
 * @param[in] name symbol name
 * @param[in] nameLen byte length of `name`
 * @param[out] value symbol value
 *
 * @return const struct blink_symbol *
 * @retval NULL (symbol not found)
 * 
 * */
const struct blink_symbol *BLINK_GetSymbolValue(const struct blink_enum *e, const char *name, size_t nameLen, int32_t *value);

/**
 * @param[in] e enum definition
 * @param[in] value symbol value
 * @param[out] name symbol name
 * @param[out] nameLen byte length of `name`
 *
 * @return const struct blink_symbol *
 * @retval NULL (symbol not found)
 * 
 * */
const struct blink_symbol *BLINK_GetSymbolName(const struct blink_enum *e, int32_t value, const char **name, size_t *nameLen);

/** returns field type
 *
 * @param[in] field field definition
 *
 * @return const struct blink_type *
 *
 * */
const struct blink_type *BLINK_GetType(const struct blink_field *field);

#ifdef __cplusplus
}
#endif


/** @} */
#endif
