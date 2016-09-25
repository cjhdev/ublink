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
 * @defgroup blink_schema Schema Parser
 *
 * - Structures for representing schema syntax
 * - Functions for converting schema syntax to structures
 * - Functions for interacting with structures
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

/* structs ************************************************************/

/** field */
struct blink_field {
    const char *name;               /**< name of this field */
    uint16_t nameLen;               /**< byte length of `name` */
    bool isOptional;                /**< field is optional */
    struct blink_field *next;       /**< next field in group */
};

/** group */
struct blink_group {
    const char *name;               /**< name of this group */
    uint16_t nameLen;               /**< byte length of `name` */
    bool hasID;                     /**< group has an ID */
    uint64_t id;                    /**< group ID */
    const char *super;              /**< name of super group */
    uint16_t superLen;              /**< byte length of supergroup name */
    const struct blink_group *s;    /**< pointer to supergroup definition */
    struct blink_field *f;          /**< fields belonging to group */
};

/** namespace */
struct blink_namespace {
    const char *name;               /**< name of this namespace */
    uint16_t nameLen;               /**< byte length of `name` */
    struct blink_group *groups;     /**< groups defined in namespace */
    struct blink_enum *enums;       /**< enums defined in namespace */    
    struct blink_namespace *next;   /**< next namespace definition in schema */
};

/** schema */
struct blink_schema {
    struct blink_namespace *ns; /**< a schema has zero or more namespace definitions */
    bool finalised;             /**< when true no more schema definitions can be appended */
};

/** enumeration */
struct blink_enum {
    const char *name;               /**< name of this field */
    uint16_t nameLen;               /**< byte length of `name` */
    struct blink_symbol *symbol;    /**< symbols belonging to enumeration */
};

/** enumeration symbol */
struct blink_symbol {
    const char *name;
    uint16_t nameLen;
    uint64_t value;
    struct blink_symbol *next;
};

/** A field iterator stores state required to iterate through all fields of a group (including any inherited fields) */
struct blink_field_iterator {
    const struct blink_group *group[BLINK_INHERIT_DEPTH]; /**< stack of pointers to groups (inheritence is limited to MAX_DEPTH) */
    uint16_t depth;                             /**< current depth in `group` */
    const struct blink_field *field;            /**< current field pointer */
};

/* function prototypes ************************************************/

/** Create a new schema object
 *
 * @param[in] ctxt schema object
 *
 * @return pointer to blink_schema
 *
 * */
struct blink_schema *BLINK_NewSchema(struct blink_schema *ctxt);

/** Destroy a schema object
 * 
 * @param[in] ctxt schema object
 *
 * */
void BLINK_DestroySchema(struct blink_schema *ctxt);

/**
 * Parse next Blink Protocol schema definition
 *
 * - Use this interface to append several schema definitions
 * - Use BLINK_ParseNext to append the final definition
 *
 * 
 * @param[in] ctxt schema object
 * @param[in] in Blink Protocol schema definition
 * @param[in] inLen byte length of `in`
 *
 * @return initialised schema or NULL
 *
 * */
struct blink_schema *BLINK_ParseNext(struct blink_schema *ctxt, const char *in, size_t inLen);

/**
 * Parse final Blink Protocol schema definition
 *
 * - Identical functionality to BLINK_Parse
 *
 * @see BLINK_Parse
 *
 * */
struct blink_schema *BLINK_ParseFinal(struct blink_schema *ctxt, const char *in, size_t inLen);

/**
 * Parse a single Blink Protocol schema definition
 *
 * @param[in] ctxt schema object
 * @param[in] in Blink Protocol schema definition
 * @param[in] inLen byte length of `in`
 *
 * @return initialised schema or NULL
 *
 * */
struct blink_schema *BLINK_Parse(struct blink_schema *ctxt, const char *in, size_t inLen);


/** Find a group by name
 * 
 * @param[in] ctxt schema object
 * @param[in] qName qualified group name
 * @param[in] qNameLen byte length of `qName`
 *
 * @return pointer to blink_group
 *
 * */
const struct blink_group *BLINK_GetGroupByName(struct blink_schema *ctxt, const char *qName, uint16_t qNameLen);

/** Find a group by ID
 * 
 * @param[in] ctxt schema object
 * @param[in] id group ID
 *
 * @return pointer to blink_group
 *
 * */
const struct blink_group *BLINK_GetGroupByID(struct blink_schema *ctxt, uint64_t id);

/** Create a new field iterator for a given group
 *
 * @param[in] group group to iterate
 * @param[in] iter iterator state
 *
 * @return pointer to blink_field_iterator
 * 
 * */
const struct blink_field_iterator *BLINK_NewFieldIterator(const struct blink_group *group, struct blink_field_iterator *iter);

/** Get next field from iterator
 *
 * @param[in] iter field iterator
 * 
 * @return pointer to blink_field
 *
 * */
const struct blink_field *BLINK_NextField(struct blink_field_iterator *iter);

#ifdef __cplusplus
}
#endif


/** @} */
#endif
