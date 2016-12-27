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

#ifndef BLINK_SCHEMA_INTERNAL_H
#define BLINK_SCHEMA_INTERNAL_H

/**
 * @defgroup blink_schema_internal blink_schema_internal
 * @ingroup ublink
 *
 * Internal dependency required by blink_schema.
 *
 * @{
 * */

/* includes ***********************************************************/

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

/* enums **************************************************************/

/** A field shall represent one of the following types */
enum blink_itype_tag {
    BLINK_ITYPE_STRING = 0,        /**< UTF8 encoded string */
    BLINK_ITYPE_BINARY,            /**< octet string */
    BLINK_ITYPE_FIXED,             /**< fixed size string */
    BLINK_ITYPE_BOOL,              /**< boolean */
    BLINK_ITYPE_U8,                /**< 8 bit unsigned integer */
    BLINK_ITYPE_U16,               /**< 16 bit unsigned integer */
    BLINK_ITYPE_U32,               /**< 32 bit unsigned integer */
    BLINK_ITYPE_U64,               /**< 64 bit unsigned integer */
    BLINK_ITYPE_I8,                /**< 8 bit signed integer */
    BLINK_ITYPE_I16,               /**< 16 bit signed integer */
    BLINK_ITYPE_I32,               /**< 32 bit signed integer */
    BLINK_ITYPE_I64,               /**< 64 bit signed integer */
    BLINK_ITYPE_F64,               /**< IEEE 754 double */
    BLINK_ITYPE_DATE,              
    BLINK_ITYPE_TIME_OF_DAY_MILLI,
    BLINK_ITYPE_TIME_OF_DAY_NANO,
    BLINK_ITYPE_NANO_TIME,
    BLINK_ITYPE_MILLI_TIME,        
    BLINK_ITYPE_DECIMAL,           /**< 8 bit signed integer exponent, 64 bit signed integer mantissa */
    BLINK_ITYPE_OBJECT,            /**< any group encoded as dynamic group */
    BLINK_ITYPE_REF                /**< reference to a typedef, enum, or group */
};

/* structs ************************************************************/

/** generic single linked list element */
struct blink_list_element {
    struct blink_list_element *next;    /**< next element */
    enum blink_list_type {
        BLINK_ELEM_NULL,                /**< no type allocated */
        BLINK_ELEM_NS,                  /**< blink_namespace */
        BLINK_ELEM_GROUP,               /**< blink_group */
        BLINK_ELEM_FIELD,               /**< blink_field */
        BLINK_ELEM_ENUM,                /**< blink_enum */
        BLINK_ELEM_SYMBOL,              /**< blink_symbol */
        BLINK_ELEM_TYPE,                /**< blink_type */
        BLINK_ELEM_ANNOTE,              /**< blink_annote */        
        BLINK_ELEM_INCR_ANNOTE        /**< blink_inline_annote */        
    } type;                             /**< type allocated at `ptr` */    
    void *ptr;                          /**< points to instance of `type` */
};

/** type */
struct blink_type {
    const char *name;            /**< name of reference (applicable to #BLINK_ITYPE_REF) */
    size_t nameLen;              /**< byte length of `name` */
    struct blink_list_element *a;
    bool isDynamic;             /**< reference is dynamic (applicable to #BLINK_ITYPE_REF) */
    bool isSequence;            /**< this is a SEQUENCE of type */                
    uint32_t size;              /**< size attribute (applicable to #BLINK_ITYPE_BINARY, #BLINK_ITYPE_FIXED, and #BLINK_ITYPE_STRING) */    
    enum blink_itype_tag tag;               /**< what type is this? */
    struct blink_list_element *resolvedRef; /**< `ref` resolves to this structure */    
};

/** field */
struct blink_field {
    const char *name;               /**< name of this field */
    size_t nameLen;                 /**< byte length of `name` */
    struct blink_list_element *a;
    bool isOptional;                /**< field is optional */
    uint64_t id;
    bool hasID;
    struct blink_type type;         /**< field type information */    
};

/** group */
struct blink_group {
    const char *name;               /**< name of this group */
    size_t nameLen;                 /**< byte length of `name` */
    struct blink_list_element *a;
    bool hasID;                     /**< group has an ID */
    uint64_t id;                    /**< group ID */
    const char *superGroup;         /**< name of super group */
    size_t superGroupLen;           /**< byte length of supergroup name */    
    struct blink_list_element *s;   /**< optional supergroup */
    struct blink_list_element *f;   /**< fields belonging to group */    
};

/** enumeration symbol */
struct blink_symbol {
    const char *name;               /**< name of symbol */
    size_t nameLen;                 /**< byte length of `name` */
    struct blink_list_element *a;
    int32_t value;                  /**< integer value */
    bool implicitValue;             /**< true if `value` is not explicitly defined */    
};

/** enumeration */
struct blink_enum {
    const char *name;               /**< name of this field */
    size_t nameLen;                 /**< byte length of `name` */
    struct blink_list_element *a;
    struct blink_list_element *s;   /**< symbols belonging to enumeration */    
};

/** type definition */
struct blink_type_def {
    const char *name;               /**< name of type definition */
    size_t nameLen;                 /**< byte length of `name` */
    struct blink_list_element *a;
    struct blink_type type;         /**< type information */    
};

struct blink_annote {
    const char *name;               /**< name of annotation */
    size_t nameLen;                 /**< byte length of `name */
    const char *value;              /**< annotation value */
    size_t valueLen;                /**< byte length of `value` */                
    uint64_t number;
};

struct blink_incr_annote {
    const char *name;               /**< key */
    size_t nameLen;                 /**< byte length of `key` */
    const char *fieldName;
    size_t fieldNameLen;
    bool type;
    struct blink_list_element *a;   /**< annotations */
};

/** namespace */
struct blink_namespace {    
    const char *name;   /**< name of this namespace */
    size_t nameLen;     /**< byte length of `name` */  
    /** list of groups, enums, and types in this namespace */
    struct blink_list_element *defs;
    struct blink_list_element *a;   /** schema <- <annotes> */
};

struct blink_element {

    struct blink_list_element *ptr;
    struct blink_element *next;
};

struct blink_def_iterator {

    struct blink_list_element *ns;
    struct blink_list_element *def;
};

/** schema */
struct blink_schema {
    struct blink_list_element *ns;  /**< a schema has zero or more namespace definitions */
    bool finalised;                 /**< when true no more schema definitions can be appended */
    struct blink_element *elements; /**< used to keep track of everything allocated */
    blink_pool_t pool;
};

/** @} */

 #endif
