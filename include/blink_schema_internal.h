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
 * Definitions required by blink_schema and other modules.
 *
 * @{
 * */

/* includes ***********************************************************/

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

#include "blink_alloc.h"

/* types **************************************************************/

/** internal field types */
enum blink_itype_tag {
    BLINK_ITYPE_STRING = 0,
    BLINK_ITYPE_BINARY,
    BLINK_ITYPE_FIXED,
    BLINK_ITYPE_BOOL,
    BLINK_ITYPE_U8,
    BLINK_ITYPE_U16,
    BLINK_ITYPE_U32,
    BLINK_ITYPE_U64,
    BLINK_ITYPE_I8,
    BLINK_ITYPE_I16,
    BLINK_ITYPE_I32,
    BLINK_ITYPE_I64,
    BLINK_ITYPE_F64,
    BLINK_ITYPE_DATE,              
    BLINK_ITYPE_TIME_OF_DAY_MILLI,
    BLINK_ITYPE_TIME_OF_DAY_NANO,
    BLINK_ITYPE_NANO_TIME,
    BLINK_ITYPE_MILLI_TIME,        
    BLINK_ITYPE_DECIMAL,
    BLINK_ITYPE_OBJECT,
    BLINK_ITYPE_REF                /**< reference to a typedef, enum, or group */
};

enum blink_schema_subclass {
    BLINK_SCHEMA = 0,
    BLINK_SCHEMA_NS,
    BLINK_SCHEMA_GROUP,
    BLINK_SCHEMA_FIELD,
    BLINK_SCHEMA_ENUM,
    BLINK_SCHEMA_SYMBOL,
    BLINK_SCHEMA_TYPE_DEF,
    BLINK_SCHEMA_ANNOTE,
    BLINK_SCHEMA_INCR_ANNOTE,        
};

struct blink_schema {
    enum blink_schema_subclass type;
    const char *name;                   /**< name of type definition */          
    struct blink_schema *next;
};

/** type */
struct blink_schema_type {
    const char *name;               /**< name of reference (applicable to #BLINK_ITYPE_REF) */
#ifndef BLINK_NO_ANNOTES    
    struct blink_schema *a;         /**< annotations */
#endif    
    union {
        struct blink_schema *resolved;  /**< resolved attribute (applicable to #BLINK_ITYPE_REF) */
        uint32_t size;                  /**< size attribute (applicable to #BLINK_ITYPE_BINARY, #BLINK_ITYPE_FIXED, and #BLINK_ITYPE_STRING) */
    } attr;
    enum blink_itype_tag tag;       /**< what type is this? */    
    bool isDynamic;                 /**< reference is dynamic (applicable to #BLINK_ITYPE_REF) */
    bool isSequence;                /**< this is a SEQUENCE of type */
};

/** field */
struct blink_schema_field {
    struct blink_schema super;
    struct blink_schema *a;         /**< annotations */    
    struct blink_schema_type type;  /**< field type information */
    bool isOptional;                /**< field is optional */
};

/** namespace */
struct blink_schema_namespace {
    struct blink_schema super;    
    struct blink_schema *defs;  /**< list of groups, enums, and types in this namespace */
#ifndef BLINK_NO_ANNOTES    
    struct blink_schema *a;     /**< schema <- <annotes> */
#endif    
};

/** group */
struct blink_schema_group {
    struct blink_schema super;
#ifndef BLINK_NO_ANNOTES    
    struct blink_schema *a;
#endif    
    uint64_t id;                    /**< group ID */
    const char *superGroup;         /**< name of super group */    
    struct blink_schema *s;         /**< optional supergroup */
    struct blink_schema *f;         /**< fields belonging to group */
    struct blink_schema_namespace *ns;     /**< link back to namespace */
    bool hasID;                     /**< group has an ID */
};

/** enumeration symbol */
struct blink_schema_symbol {
    struct blink_schema super;
#ifndef BLINK_NO_ANNOTES    
    struct blink_schema *a;
#endif    
    int32_t value;                  /**< integer value */
};

/** enumeration */
struct blink_schema_enum {
    struct blink_schema super;
#ifndef BLINK_NO_ANNOTES
    struct blink_schema *a;
#endif    
    struct blink_schema *s;   /**< symbols belonging to enumeration */
};

/** type definition */
struct blink_schema_type_def {
    struct blink_schema super;
#ifndef BLINK_NO_ANNOTES
    struct blink_schema *a;
#endif    
    struct blink_schema_type type;         /**< type information */
};

struct blink_schema_annote {
    struct blink_schema super;
    const char *value;              /**< annotation value */    
    uint64_t number;
};

struct blink_schema_incr_annote {
    struct blink_schema super;
    const char *fieldName;
#ifndef BLINK_NO_ANNOTES
    struct blink_schema *a;   /**< annotations */
#endif    
    bool type;    
};

struct blink_schema_base {
    struct blink_schema super;
    struct blink_schema *ns;        /**< a schema has zero or more namespace definitions */
    struct blink_allocator alloc;
};

/** @} */

 #endif
