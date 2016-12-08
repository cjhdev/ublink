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
 * */

/* includes ***********************************************************/

#include <string.h>
#include <stddef.h>

#include "blink_debug.h"
#include "blink_parser.h"
#include "blink_lexer.h"

/* definitions ********************************************************/

#ifndef BLINK_LINK_DEPTH
    /** maximum number of reference to reference links */
    #define BLINK_LINK_DEPTH    10U
#endif

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

/** type */
struct blink_type {
    bool isDynamic;             /**< reference is dynamic (applicable to #BLINK_ITYPE_REF) */
    bool isSequence;            /**< this is a SEQUENCE of type */                
    uint32_t size;              /**< size attribute (applicable to #BLINK_ITYPE_BINARY, #BLINK_ITYPE_FIXED, and #BLINK_ITYPE_STRING) */
    const char *ref;            /**< name of reference (applicable to #BLINK_ITYPE_REF) */
    size_t refLen;                          /**< byte length of `ref` */
    enum blink_itype_tag tag;               /**< what type is this? */
    struct blink_list_element *resolvedRef; /**< `ref` resolves to this structure */
    struct blink_list_element *a;
};

/** field */
struct blink_field {
    const char *name;               /**< name of this field */
    size_t nameLen;                 /**< byte length of `name` */
    bool isOptional;                /**< field is optional */
    uint64_t id;
    bool hasID;
    struct blink_type type;         /**< field type information */
    struct blink_list_element *a;
};

/** group */
struct blink_group {
    const char *name;               /**< name of this group */
    size_t nameLen;                 /**< byte length of `name` */
    bool hasID;                     /**< group has an ID */
    uint64_t id;                    /**< group ID */
    const char *superGroup;         /**< name of super group */
    size_t superGroupLen;           /**< byte length of supergroup name */    
    struct blink_list_element *s;   /**< optional supergroup */
    struct blink_list_element *f;   /**< fields belonging to group */
    struct blink_list_element *a;
};

/** enumeration symbol */
struct blink_symbol {
    const char *name;               /**< name of symbol */
    size_t nameLen;                 /**< byte length of `name` */
    int32_t value;                  /**< integer value */
    bool implicitValue;             /**< true if `value` is not explicitly defined */
    struct blink_list_element *a;
};

/** enumeration */
struct blink_enum {
    const char *name;               /**< name of this field */
    size_t nameLen;                 /**< byte length of `name` */
    struct blink_list_element *s;   /**< symbols belonging to enumeration */
    struct blink_list_element *a;
};

/** type definition */
struct blink_type_def {
    const char *name;               /**< name of type definition */
    size_t nameLen;                 /**< byte length of `name` */
    struct blink_type type;         /**< type information */
    struct blink_list_element *a;
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

struct blink_element {

    struct blink_list_element *ptr;
    struct blink_element *next;
};

struct blink_def_iterator {

    struct blink_list_element *ns;
    struct blink_list_element *def;
};

/* static prototypes **************************************************/

static struct blink_schema *parseSchema(struct blink_schema *self, const char *in, size_t inLen);
static bool parseType(const char *in, size_t inLen, size_t *read, struct blink_type *type);
static bool parseAnnote(struct blink_schema *self, const char *in, size_t inLen, size_t *read, struct blink_annote *annote);
static bool parseAnnotes(struct blink_schema *self, const char *in, size_t inLen, size_t *read, struct blink_list_element **annotes);

static struct blink_list_element *newListElement(struct blink_schema *self, struct blink_list_element **head, enum blink_list_type type);
static struct blink_list_element *searchListByName(struct blink_list_element *head, const char *name, size_t nameLen);

static void splitCName(const char *in, size_t inLen, const char **nsName, size_t *nsNameLen, const char **name, size_t *nameLen);

static bool resolveDefinitions(struct blink_schema *self);
static struct blink_list_element *resolve(struct blink_schema *self, const char *cName, size_t cNameLen);

static bool testConstraints(struct blink_schema *self);
static bool testReferenceConstraint(const struct blink_schema *self, const struct blink_list_element *reference);
static bool testSuperGroupReferenceConstraint(const struct blink_schema *self, const struct blink_group *group);
static bool testSuperGroupShadowConstraint(const struct blink_schema *self, const struct blink_group *group);

static struct blink_list_element *getTerminal(struct blink_list_element *element, bool *dynamic);

static struct blink_list_element *initDefinitionIterator(struct blink_def_iterator *iter, struct blink_schema *schema);
static struct blink_list_element *nextDefinition(struct blink_def_iterator *iter);

static struct blink_enum *castEnum(struct blink_list_element *self);
static struct blink_symbol *castSymbol(struct blink_list_element *self);
static struct blink_field *castField(struct blink_list_element *self);
static struct blink_group *castGroup(struct blink_list_element *self);
static struct blink_namespace *castNamespace(struct blink_list_element *self);
static struct blink_type_def *castTypeDef(struct blink_list_element *self);
static const struct blink_type_def *castConstTypeDef(const struct blink_list_element *self);
static struct blink_annote *castAnnote(struct blink_list_element *self);
static struct blink_incr_annote *castIncrAnnote(struct blink_list_element *self);

/* functions **********************************************************/

struct blink_schema *BLINK_InitSchema(struct blink_schema *schema, fn_blink_calloc_t calloc, fn_blink_free_t free)
{
    BLINK_ASSERT(schema != NULL)
    BLINK_ASSERT(calloc != NULL)

    (void)memset(schema, 0x0, sizeof(*schema));

    schema->calloc = calloc;
    schema->free = free;
    
    return schema;
}

void BLINK_DestroySchema(struct blink_schema *self)
{
    BLINK_ASSERT(self != NULL)

    if(self->free != NULL){

        struct blink_element *nextElement = self->elements;

        /* see newListElement */
        while(nextElement != NULL){

            struct blink_element *element = nextElement;
            nextElement = nextElement->next;

            if(element->ptr != NULL){
                self->free(element->ptr->ptr);    /* this pointer may be null */
                self->free(element->ptr);
            }            
            self->free(element);
        }
    }
      
    (void)memset(self, 0x0, sizeof(*self));
}

const struct blink_group *BLINK_GetGroupByName(struct blink_schema *self, const char *qName, size_t qNameLen)
{
    BLINK_ASSERT(self != NULL)
    BLINK_ASSERT(qName != NULL)
    
    const char *name;
    size_t nameLen;
    const char *nsName;
    size_t nsNameLen;

    splitCName(qName, qNameLen, &nsName, &nsNameLen, &name, &nameLen);

    struct blink_namespace *ns = castNamespace(searchListByName(self->ns, nsName, nsNameLen));

    return (ns == NULL) ? NULL : (const struct blink_group *)castGroup(searchListByName(ns->defs, name, nameLen));
}

const struct blink_group *BLINK_GetGroupByID(struct blink_schema *self, uint64_t id)
{
    BLINK_ASSERT(self != NULL)

    const struct blink_group *retval = NULL;
    struct blink_def_iterator iter;
    struct blink_list_element *defPtr = initDefinitionIterator(&iter, self);

    while((retval == NULL) && (defPtr != NULL)){

        struct blink_group *gPtr = (defPtr->type == BLINK_ELEM_GROUP) ? castGroup(defPtr) : NULL;

        if((gPtr != NULL) && gPtr->hasID && (gPtr->id == id)){    

            retval = gPtr;
        }
        else{
            
            defPtr = nextDefinition(&iter);
        }
    }

    return retval;
}

void BLINK_InitFieldIterator(struct blink_field_iterator *iter, const struct blink_group *group)
{
    BLINK_ASSERT(iter != NULL)
    BLINK_ASSERT(group != NULL)
    
    const struct blink_group *ptr = group;
    bool dynamic;
    
    (void)memset(iter, 0, sizeof(*iter));

    for(iter->depth=0U; iter->depth < BLINK_INHERIT_DEPTH; iter->depth++){

        iter->field[iter->depth] = ptr->f;
        
        if(ptr->s == NULL){

            break;            
        }
        else{

            ptr = castGroup(getTerminal(ptr->s, &dynamic));
        }
    }

    /* it is not possible to initialise a blink_schema that trips this */
    BLINK_ASSERT(iter->depth < BLINK_INHERIT_DEPTH)
}

const struct blink_field *BLINK_NextField(struct blink_field_iterator *self)
{
    BLINK_ASSERT(self != NULL)
    
    const struct blink_field *retval = NULL;

    while(retval == NULL){

        if(self->field[self->depth] != NULL){

            retval = castField(self->field[self->depth]);
            self->field[self->depth] = self->field[self->depth]->next;
        }
        else if(self->depth > 0U){

            self->depth--;
        }
        else{

            break;
        }
    }

    return retval;    
}

struct blink_schema *BLINK_Parse(struct blink_schema *self, const char *in, size_t inLen)
{
    BLINK_ASSERT(self != NULL)
    BLINK_ASSERT(in != NULL)

    struct blink_schema *retval = NULL;
    
    if(parseSchema(self, in, inLen) == self){

        if(resolveDefinitions(self)){

            if(testConstraints(self)){

                self->finalised = true;
                retval = self;
            }
        }
    }

    if(retval != self){

        BLINK_DestroySchema(self);
    }

    return retval;    
}

const char *BLINK_GetGroupName(const struct blink_group *self, size_t *nameLen)
{
    BLINK_ASSERT(self != NULL)
    BLINK_ASSERT(nameLen != NULL)

    *nameLen = self->nameLen;
    return self->name;
}

const char *BLINK_GetFieldName(const struct blink_field *self, size_t *nameLen)
{
    BLINK_ASSERT(self != NULL)
    BLINK_ASSERT(nameLen != NULL)

    *nameLen = self->nameLen;
    return self->name;
}

bool BLINK_GetFieldIsOptional(const struct blink_field *self)
{
    BLINK_ASSERT(self != NULL)

    return self->isOptional;
}

enum blink_type_tag BLINK_GetFieldType(const struct blink_field *self)
{
    BLINK_ASSERT(self != NULL)

    static const enum blink_type_tag translate[] = {
        BLINK_TYPE_STRING,
        BLINK_TYPE_BINARY,
        BLINK_TYPE_FIXED,
        BLINK_TYPE_BOOL,
        BLINK_TYPE_U8,
        BLINK_TYPE_U16,
        BLINK_TYPE_U32,
        BLINK_TYPE_U64,
        BLINK_TYPE_I8,
        BLINK_TYPE_I16,
        BLINK_TYPE_I32,
        BLINK_TYPE_I64,
        BLINK_TYPE_F64,
        BLINK_TYPE_DATE,              
        BLINK_TYPE_TIME_OF_DAY_MILLI,
        BLINK_TYPE_TIME_OF_DAY_NANO,
        BLINK_TYPE_NANO_TIME,
        BLINK_TYPE_MILLI_TIME,        
        BLINK_TYPE_DECIMAL,
        BLINK_TYPE_OBJECT            
    };

    enum blink_type_tag retval;
    bool dynamic;

    if(self->type.tag == BLINK_ITYPE_REF){ 

        struct blink_list_element *ptr = getTerminal(self->type.resolvedRef, &dynamic);

        switch(ptr->type){
        case BLINK_ELEM_ENUM:
            retval = BLINK_TYPE_ENUM;
            break;
        case BLINK_ELEM_GROUP:
            retval = (dynamic) ? BLINK_TYPE_DYNAMIC_GROUP : BLINK_TYPE_STATIC_GROUP;
            break;
        default:
            BLINK_ASSERT((size_t)castTypeDef(ptr)->type.tag < (sizeof(translate)/sizeof(*translate)))
            retval = translate[castTypeDef(ptr)->type.tag];
            break;
        }        
    }    
    else{

        BLINK_ASSERT((size_t)self->type.tag < (sizeof(translate)/sizeof(*translate)))
        retval = translate[self->type.tag];
    }

    return retval;
}

uint32_t BLINK_GetFieldSize(const struct blink_field *self)
{
    BLINK_ASSERT(self != NULL)

    return self->type.size;
}

const struct blink_group *BLINK_GetFieldGroup(const struct blink_field *self)
{
    BLINK_ASSERT(self != NULL)

    const struct blink_group *retval = NULL;

    if(self->type.tag == BLINK_ITYPE_REF){

        bool dynamic;
        struct blink_list_element *ref = getTerminal(self->type.resolvedRef, &dynamic);

        BLINK_ASSERT(ref != NULL)

        if(ref->type == BLINK_ELEM_GROUP){

            retval = (const struct blink_group *)castGroup(ref);
        }
    }

    return retval;
}

const struct blink_symbol *BLINK_GetSymbolValue(const struct blink_enum *self, const char *name, size_t nameLen, int32_t *value)
{
    BLINK_ASSERT(self != NULL)
    BLINK_ASSERT(name != NULL)
    BLINK_ASSERT(value != NULL)

    const struct blink_symbol *retval = NULL;
    struct blink_list_element *element = searchListByName(self->s, name, nameLen);

    if(element != NULL){

        retval = castSymbol(element);
        *value = retval->value;
    }

    return retval;
}

const struct blink_symbol *BLINK_GetSymbolName(const struct blink_enum *self, int32_t value, const char **name, size_t *nameLen)
{
    BLINK_ASSERT(self != NULL)
    BLINK_ASSERT(name != NULL)
    BLINK_ASSERT(nameLen != NULL)
    
    struct blink_list_element *ptr = self->s;
    const struct blink_symbol *retval = NULL;

    while(ptr != NULL){

        if(castSymbol(ptr)->value == value){

            retval = castSymbol(ptr);
            *name = retval->name;
            *nameLen = retval->nameLen;
            break;
        }

        ptr = ptr->next;
    }

    return retval;
}

bool BLINK_GroupIsKindOf(const struct blink_group *self, const struct blink_group *group)
{
    bool retval = true;

    //todo

    return retval;
}

/* static functions ***************************************************/

static struct blink_schema *parseSchema(struct blink_schema *self, const char *in, size_t inLen)
{
    BLINK_ASSERT(self != NULL)
    BLINK_ASSERT(in != NULL)
    
    size_t pos = 0U;
    struct blink_namespace *ns;    
    struct blink_list_element *element;
    struct blink_list_element *defAnnotes;

    enum blink_token tok;
    union blink_token_value value;
    size_t read;
    enum blink_token nextTok;
    union blink_token_value nextValue;
    size_t nextRead;

    /* specific namespace */
    if(BLINK_GetToken(in, inLen, &read, &value, NULL) == TOK_NAMESPACE){

        pos += read;

        if(BLINK_GetToken(&in[pos], inLen - pos, &read, &value, NULL) != TOK_NAME){

            BLINK_ERROR("expecting a namespace name")
            return NULL;
        }
        
        pos += read;
    }
    else{

        (void)memset(&value, 0, sizeof(value));
    }

    element = searchListByName(self->ns, value.literal.ptr, value.literal.len);

    /* namespace not yet defined */
    if(element == NULL){

        ns = castNamespace(newListElement(self, &self->ns, BLINK_ELEM_NS));

        if(ns == NULL){
            return NULL;
        }

        ns->name = value.literal.ptr;
        ns->nameLen = value.literal.len;
    }
    else{

        ns = castNamespace(element);
    }

    /* parse all definitions */
    while(BLINK_GetToken(&in[pos], inLen-pos, &read, &value, NULL) != TOK_EOF){

        if(!parseAnnotes(self, &in[pos], inLen - pos, &read, &defAnnotes)){
            return NULL;
        }

        tok = BLINK_GetToken(&in[pos], inLen-pos, &read, &value, NULL);
        nextTok = BLINK_GetToken(&in[pos+read], inLen-pos-read, &nextRead, &nextValue, NULL);

        /* incremental annote */
        if((tok == TOK_SCHEMA) || (tok == TOK_CNAME) || ((tok == TOK_NAME) && ((nextTok == TOK_PERIOD) || (nextTok == TOK_LARROW)))){

            pos += read;
            
            if(defAnnotes != NULL){

                BLINK_ERROR("expecting a group, type, or enum definition")
                return NULL;
            }

            struct blink_incr_annote *ia = castIncrAnnote(newListElement(self, &ns->defs, BLINK_ELEM_INCR_ANNOTE));

            if(ia == NULL){
                return NULL;
            }

            if(tok == TOK_SCHEMA){

                if(nextTok != TOK_LARROW){
    
                    BLINK_ERROR("expecting '<-'")
                    return NULL;
                }
            }
            else{

                ia->name = value.literal.ptr;
                ia->nameLen = value.literal.len;
            }

            if(nextTok == TOK_PERIOD){

                pos += nextRead;

                tok = BLINK_GetToken(&in[pos], inLen - pos, &read, &value, NULL);

                pos += read;

                if(tok == TOK_NAME){

                    ia->fieldName = value.literal.ptr;
                    ia->fieldNameLen = value.literal.len;

                    tok = BLINK_GetToken(&in[pos], inLen - pos, &read, &value, NULL);

                    if(tok == TOK_PERIOD){

                        pos += read;

                        if(BLINK_GetToken(&in[pos], inLen - pos, &read, &value, NULL) != TOK_TYPE){
                            BLINK_ERROR("expecting 'type'")
                            return NULL;
                        }

                        pos += read;

                        ia->type = true;
                    }
                }
                else if (tok == TOK_TYPE){

                    ia->type = true;
                }
                else{
                
                    BLINK_ERROR("expecting <name> or 'type'")
                    return NULL;
                }
            }

            if(BLINK_GetToken(&in[pos], inLen - pos, &read, &value, NULL) != TOK_LARROW){
                BLINK_ERROR("expecting '<-'")
                return NULL;                 
            }

            read = 0U;

            do{

                pos += read;

                tok = BLINK_GetToken(&in[pos], inLen - pos, &read, &value, NULL);

                pos += read;

                struct blink_annote *annote = NULL;

                if(tok == TOK_AT){

                    struct blink_annote a;

                    if(!parseAnnote(self, &in[pos], inLen - pos, &read, &a)){
                        return NULL;
                    }

                    pos += read;
                    
                    annote = castAnnote(searchListByName(ia->a, a.name, a.nameLen));

                    if(annote == NULL){

                        annote = castAnnote(newListElement(self, &ia->a, BLINK_ELEM_ANNOTE));

                        if(annote == NULL){
                            return NULL;
                        }
                    }

                    *annote = a;    /*copy*/
                }
                else if(tok == TOK_UINT){

                    struct blink_list_element *ptr = ia->a;

                    /* overwrite existing number */
                    while(ptr != NULL){

                        annote = castAnnote(ptr);
                        
                        if(annote->name == NULL){
                            break;
                        }
                        ptr = ptr->next;
                    }

                    if(ptr == NULL){
                                  
                        annote = castAnnote(newListElement(self, &ia->a, BLINK_ELEM_ANNOTE));
                        if(annote == NULL){
                            return NULL;
                        }                                
                    }

                    annote->number = value.number;
                }
                else{

                    BLINK_ERROR("expecting <number> or '@'")
                    return NULL;
                }                
            }
            while(BLINK_GetToken(&in[pos], inLen - pos, &read, &value, NULL) == TOK_LARROW);                
        }
        /* definition */
        else if(tok == TOK_NAME){
                    
            pos += read;

            const char *name = value.literal.ptr;
            size_t nameLen = value.literal.len;                
            
            if(searchListByName(ns->defs, name, nameLen) != NULL){

                BLINK_ERROR("duplicate definition name")
                return NULL;
            }
            
            /* type or enum */
            if(nextTok == TOK_EQUAL){

                pos += nextRead;

                bool singleton;
                struct blink_list_element *typeAnnotes;

                if(BLINK_GetToken(&in[pos], inLen - pos, &read, &value, NULL) == TOK_BAR){

                    singleton = true;
                    pos += read;
                }
                else{

                    singleton = false;
                }

                if(!parseAnnotes(self, &in[pos], inLen - pos, &read, &typeAnnotes)){
                    return NULL;
                }

                pos += read;

                tok = BLINK_GetToken(&in[pos], inLen - pos, &read, &value, NULL);
                nextTok = BLINK_GetToken(&in[pos+read], inLen-pos-read, &nextRead, &nextValue, NULL);
                
                /* enum */
                if(singleton || ((tok == TOK_NAME) && ((nextTok == TOK_SLASH) || nextTok == TOK_BAR))){

                    struct blink_enum *e = castEnum(newListElement(self, &ns->defs, BLINK_ELEM_ENUM));

                    if(e == NULL){
                        return NULL;
                    }

                    e->name = name;
                    e->nameLen = nameLen;
                    e->a = defAnnotes;

                    read = 0U;
                    bool first = true;

                    do{

                        pos += read;

                        struct blink_symbol *s = castSymbol(newListElement(self, &e->s, BLINK_ELEM_SYMBOL));

                        if(s == NULL){
                            return NULL;
                        }

                        /* a kludge because we already parse the annotes for the first
                         * symbol in sequence */
                        if(first){

                            s->a = typeAnnotes;
                            first = false;
                        }
                        else{

                            if(!parseAnnotes(self, &in[pos], inLen - pos, &read, &s->a)){
                                return NULL;
                            }
                            pos += read;     
                        }
                        
                        if(BLINK_GetToken(&in[pos], inLen - pos, &read, &value, NULL) != TOK_NAME){

                            BLINK_ERROR("expecting enum symbol name")
                            return NULL;
                        }

                        pos += read;

                        if(searchListByName(e->s, value.literal.ptr, value.literal.len) != NULL){
                            
                            BLINK_ERROR("duplicate enum symbol name")
                            return NULL;
                        }

                        s->name = value.literal.ptr;
                        s->nameLen = value.literal.len;
                            
                        if(BLINK_GetToken(&in[pos], inLen - pos, &read, &value, NULL) == TOK_SLASH){

                            pos += read;

                            tok = BLINK_GetToken(&in[pos], inLen - pos, &read, &value, NULL);

                            pos += read;

                            if(tok == TOK_UINT){

                                if(value.number > (uint64_t)INT32_MAX){

                                    BLINK_ERROR("enum symbol value out of range")
                                    return NULL;
                                }
                                s->value = (int32_t)value.number;
                            }
                            else if(tok == TOK_INT){
                            
                                if((value.signedNumber > (int64_t)INT32_MAX) || (value.signedNumber < (int64_t)INT32_MIN)){

                                    BLINK_ERROR("enum symbol value out of range")
                                    return NULL;
                                }

                                s->value = (int32_t)value.signedNumber;
                            }
                            else{
                                
                                BLINK_ERROR("expecting enum symbol value")
                                return NULL;
                            }
                            
                            s->implicitValue = false;                                
                        }
                        else{

                            s->implicitValue = true;
                        }

                        if(castSymbol(e->s) == s){

                            if(s->implicitValue){
                                s->value = 0;
                            }
                        }
                        else{

                            struct blink_list_element *ptr = e->s;
                            while(castSymbol(ptr->next) != s){
                                ptr = ptr->next;
                            }
                            
                            if(s->implicitValue){

                                if(castSymbol(ptr)->value == INT32_MAX){

                                    BLINK_ERROR("no next implicit enum value possible")
                                    return NULL;
                                }
                                s->value = castSymbol(ptr)->value + 1;
                            }
                            else{
                                
                                if(s->value <= castSymbol(ptr)->value){

                                    BLINK_ERROR("enum value is ambiguous")
                                    return NULL;
                                }
                            }                            
                        }

                        if(singleton){

                            break;
                        }           
                    }
                    while(BLINK_GetToken(&in[pos], inLen - pos, &read, &value, NULL) == TOK_BAR);
                }
                /* type */
                else{

                    struct blink_type_def *t = castTypeDef(newListElement(self, &ns->defs, BLINK_ELEM_TYPE));

                    if(t == NULL){
                        return NULL;
                    }

                    t->name = name;
                    t->nameLen = nameLen;
                    t->a = defAnnotes;
                    t->type.a = typeAnnotes;

                    if(!parseType(&in[pos], inLen - pos, &read, &t->type)){                                                
                        return NULL;
                    }
                
                    pos += read;
                }
            }
            /* group */
            else{

                struct blink_group *g = castGroup(newListElement(self, &ns->defs, BLINK_ELEM_GROUP));

                if(g == NULL){
                    return NULL;
                }

                g->name = name;
                g->nameLen = nameLen;
                g->a = defAnnotes;

                /* id field */
                if(nextTok == TOK_SLASH){

                    pos += nextRead;

                    if(BLINK_GetToken(&in[pos], inLen - pos, &read, &value, NULL) != TOK_UINT){
                        BLINK_ERROR("error: expecting integer or hexnum")
                        return NULL;
                    }

                    pos += read;

                    g->hasID = true;
                    g->id = value.number;                        
                }

                /* supergroup */
                if(BLINK_GetToken(&in[pos], inLen - pos, &read, &value, NULL) == TOK_COLON){

                    pos += read;

                    tok = BLINK_GetToken(&in[pos], inLen - pos, &read, &value, NULL);

                    if((tok != TOK_CNAME) && (tok != TOK_NAME)){
                        BLINK_ERROR("expecting super class name (qname)")
                        return NULL;
                    }

                    pos += read;

                    g->superGroup = value.literal.ptr;
                    g->superGroupLen = value.literal.len;
                }

                read = 0U;

                /* fields */
                if(BLINK_GetToken(&in[pos], inLen - pos, &read, &value, NULL) == TOK_RARROW){

                    do{

                        pos += read;

                        struct blink_field *f = castField(newListElement(self, &g->f, BLINK_ELEM_FIELD));

                        if(f == NULL){
                            return NULL;
                        }
                        
                        if(!parseAnnotes(self, &in[pos], inLen - pos, &read, &f->type.a)){
                            return NULL;
                        }

                        pos += read;
                        
                        if(!parseType(&in[pos], inLen - pos, &read, &f->type)){
                            return NULL;
                        }
                        
                        pos += read;

                        if(!parseAnnotes(self, &in[pos], inLen - pos, &read, &f->a)){
                            return NULL;
                        }

                        pos += read;                        

                        if(BLINK_GetToken(&in[pos], inLen - pos, &read, &value, NULL) != TOK_NAME){
                            BLINK_ERROR("expecting name")
                            return NULL;
                        }

                        pos += read;

                        if(searchListByName(g->f, value.literal.ptr, value.literal.len) != NULL){
                            BLINK_ERROR("duplicate field name")
                            return NULL;
                        }

                        f->name = value.literal.ptr;
                        f->nameLen = value.literal.len;

                        if(BLINK_GetToken(&in[pos], inLen - pos, &read, &value, NULL) == TOK_SLASH){

                            pos += read;

                            if(BLINK_GetToken(&in[pos], inLen - pos, &read, &value, NULL) != TOK_UINT){
                                
                                BLINK_ERROR("expecting a number")
                                return NULL;
                            }

                            f->id = value.number;
                            f->hasID = true;

                            pos += read;
                        }

                        if(BLINK_GetToken(&in[pos], inLen - pos, &read, &value, NULL) == TOK_QUESTION){
                            pos += read;
                            f->isOptional = true;
                        }
                    }
                    while(BLINK_GetToken(&in[pos], inLen - pos, &read, &value, NULL) == TOK_COMMA);                   
                }                                    
            }
        }
        else if(tok == TOK_EOF){

            if(defAnnotes != NULL){

                BLINK_ERROR("expecting group, enum, or type definition")
                return NULL;
            }
        }
        else{

            BLINK_ERROR("unexpected token");
            return NULL;
        }
    }

    return self; 
}

static bool parseType(const char *in, size_t inLen, size_t *read, struct blink_type *type)
{
    BLINK_ASSERT(in != NULL)
    BLINK_ASSERT(read != NULL)
    BLINK_ASSERT(type != NULL)

    size_t pos = 0U;
    size_t r;
    union blink_token_value value;
    enum blink_token tok = BLINK_GetToken(&in[pos], inLen - pos, &r, &value, NULL);

    static const enum blink_itype_tag tokenToType[] = {
        BLINK_ITYPE_STRING,
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
        BLINK_ITYPE_OBJECT
    };

    if((size_t)tok < (sizeof(tokenToType)/sizeof(*tokenToType))){
    
        type->tag = tokenToType[tok];   /*lint !e661 !e662 no way this is out of bounds */

        pos += r;

        if((tok == TOK_STRING) || (tok == TOK_BINARY) || (tok == TOK_FIXED)){

            if(BLINK_GetToken(&in[pos], inLen - pos, &r, &value, NULL) == TOK_LPAREN){

                pos += r;

                if(BLINK_GetToken(&in[pos], inLen - pos, &r, &value, NULL) == TOK_UINT){

                    pos += r;
                }
                else{

                    BLINK_ERROR("expecting a size")
                    return false;
                }

                if(value.number > 0xffffffffU){

                    BLINK_ERROR("size decode but is out of range")
                    return false;
                }

                type->size = (uint32_t)value.number;

                if(BLINK_GetToken(&in[pos], inLen - pos, &r, &value, NULL) == TOK_RPAREN){

                    pos += r;
                }   
                else{

                    BLINK_ERROR("expecting a ')'")
                    return false;
                }
            }
            else if(tok == TOK_FIXED){

                BLINK_ERROR("expecting a '('")
                return false;
            }
            else{

                type->size = 0xffffffffU;
            }
        }
    }
    else if((tok == TOK_NAME) || (tok == TOK_CNAME)){
        
        pos += r;

        type->ref = value.literal.ptr;
        type->refLen = value.literal.len;
        type->tag = BLINK_ITYPE_REF;

        if(BLINK_GetToken(&in[pos], inLen - pos, &r, &value, NULL) == TOK_STAR){

            pos += r;
            type->isDynamic = true;            
        }
    }
    else{

        BLINK_ERROR("expecting a type")
        return false;                                    
    }

    /* sequence of type */
    if(BLINK_GetToken(&in[pos], inLen - pos, &r, &value, NULL) == TOK_LBRACKET){

        pos += r;

        if(BLINK_GetToken(&in[pos], inLen - pos, &r, &value, NULL) == TOK_RBRACKET){
            
            pos += r;
            type->isSequence = true;
        }
        else{

            BLINK_ERROR("expecting ']' character")
            return false;
        }                        
    }
    else{

        type->isSequence = false;
    }

    *read = pos;

    return true;
}

static bool parseAnnote(struct blink_schema *self, const char *in, size_t inLen, size_t *read, struct blink_annote *annote)
{
    BLINK_ASSERT(self != NULL)
    BLINK_ASSERT(annote != NULL)
    
    size_t pos = 0U;
    size_t r;
    union blink_token_value value;
    enum blink_token tok;
    
    tok = BLINK_GetToken(&in[pos], inLen - pos, &r, &value, NULL);

    pos += r;

    switch(tok){
    case TOK_CNAME:
    case TOK_NAME:
        annote->name = value.literal.ptr;
        annote->nameLen = value.literal.len;        
        break;
    case TOK_U8:
    case TOK_U16:
    case TOK_U32:
    case TOK_U64:
    case TOK_I8:
    case TOK_I16:
    case TOK_I32:
    case TOK_I64:
    case TOK_BOOL:
    case TOK_BINARY:
    case TOK_STRING:
    case TOK_DATE:
    case TOK_DECIMAL:

        annote->name = BLINK_TokenToString(tok, &annote->nameLen);
        BLINK_ASSERT(annote->name != NULL)        
        break;
        
    default:
        BLINK_ERROR("unexpected token")
        return false;
    }    

    if(BLINK_GetToken(&in[pos], inLen - pos, &r, &value, NULL) != TOK_EQUAL){

        BLINK_ERROR("expecting '='")
        return false;
    }

    pos += r;

    if(BLINK_GetToken(&in[pos], inLen - pos, &r, &value, NULL) != TOK_LITERAL){

        BLINK_ERROR("expecting <literal>")
        return false;
    }

    annote->value = value.literal.ptr;
    annote->valueLen = value.literal.len;

    *read = pos + r;

    return true;
}

static bool parseAnnotes(struct blink_schema *self, const char *in, size_t inLen, size_t *read, struct blink_list_element **annotes)
{
    BLINK_ASSERT(self != NULL)
    BLINK_ASSERT(in != NULL)
    BLINK_ASSERT(read != NULL)
    BLINK_ASSERT(annotes != NULL)

    size_t r;
    size_t pos = 0U;
    union blink_token_value value;
    struct blink_annote *annote;
    struct blink_annote a;

    while(BLINK_GetToken(&in[pos], inLen - pos, &r, &value, NULL) == TOK_AT){

        pos += r;

        if(!parseAnnote(self, &in[pos], inLen - pos, &r, &a)){
            return false;
        }
        
        annote = castAnnote(searchListByName(*annotes, a.name, a.nameLen));

        if(annote == NULL){

            annote = castAnnote(newListElement(self, annotes, BLINK_ELEM_ANNOTE));

            if(annote == NULL){
                return false;
            }
        }

        *annote = a;    /*copy*/
    }

    *read = pos;

    return true;
}

static bool resolveDefinitions(struct blink_schema *self)
{
    BLINK_ASSERT(self != NULL)
    
    struct blink_group *g;
    struct blink_field *f;
    struct blink_type_def *t;
    struct blink_list_element *fieldPtr;
    struct blink_def_iterator iter;
    struct blink_list_element *defPtr = initDefinitionIterator(&iter, self);

    while(defPtr != NULL){
        
        switch(defPtr->type){
        case BLINK_ELEM_TYPE:

            t = castTypeDef(defPtr);
        
            if(t->type.tag == BLINK_ITYPE_REF){

                t->type.resolvedRef = resolve(self, t->type.ref, t->type.refLen);

                if(castTypeDef(defPtr)->type.resolvedRef == NULL){

                    BLINK_ERROR("unresolved")
                    return false;
                }
            }
            break;
            
        case BLINK_ELEM_GROUP:

            g = castGroup(defPtr);

            if(g->superGroup != NULL){

                g->s = resolve(self, g->superGroup, g->superGroupLen);

                if(g->s == NULL){

                    BLINK_ERROR("cannot resolve supergroup")    
                    return false;
                }
            }

            fieldPtr = g->f;

            while(fieldPtr != NULL){

                f = castField(fieldPtr);

                if(f->type.tag == BLINK_ITYPE_REF){

                    f->type.resolvedRef = resolve(self, f->type.ref, f->type.refLen);

                    if(f->type.resolvedRef == NULL){

                        BLINK_ERROR("unresolved")
                        return false;
                    }
                }

                fieldPtr = fieldPtr->next;
            }
            break;

        default:
            /*do not resolve*/
            break;
        }

        defPtr = nextDefinition(&iter);
    }

    return true;
}

static struct blink_list_element *resolve(struct blink_schema *self, const char *cName, size_t cNameLen)
{
    BLINK_ASSERT(self != NULL)
    BLINK_ASSERT(cName != NULL)

    const char *nsName;
    size_t nsNameLen;
    const char *name;
    size_t nameLen;
    struct blink_list_element *nsPtr;

    splitCName(cName, cNameLen, &nsName, &nsNameLen, &name, &nameLen);

    nsPtr = searchListByName(self->ns, nsName, nsNameLen);
    
    return (nsPtr != NULL) ? searchListByName(castNamespace(nsPtr)->defs, name, nameLen) : NULL;
}

static bool testConstraints(struct blink_schema *self)
{   
    BLINK_ASSERT(self != NULL)

    /* test reference constraints */

    struct blink_def_iterator iter;
    struct blink_list_element *defPtr = initDefinitionIterator(&iter, self);

    while(defPtr != NULL){

        if(!testReferenceConstraint(self, defPtr)){

            return false;
        }
        
        defPtr = nextDefinition(&iter);
    }

    /* test super group constraints */

    defPtr = initDefinitionIterator(&iter, self);

    while(defPtr != NULL){

        if(defPtr->type == BLINK_ELEM_GROUP){

            struct blink_group *group = castGroup(defPtr);

            if(group->s != NULL){

                /* supergroup reference */
                if(!testSuperGroupReferenceConstraint(self, group)){

                    return false;
                }

                /* supergroup shadow field names */
                if(!testSuperGroupShadowConstraint(self, group)){

                    return false;
                }
            }            
        }

        defPtr = nextDefinition(&iter);
    }

    return true;
}

static bool testReferenceConstraint(const struct blink_schema *self, const struct blink_list_element *reference)
{
    bool retval = true;
    const struct blink_list_element *ptr = reference;
    const struct blink_list_element *stack[BLINK_LINK_DEPTH];
    bool dynamic = false;
    bool sequence = false;
    size_t depth = 0U;
    size_t i;

    (void)memset(stack, 0, sizeof(stack));

    while(retval && (ptr->type == BLINK_ELEM_TYPE) && (castConstTypeDef(ptr)->type.tag == BLINK_ITYPE_REF)){    /*lint !e9007 no side effect */

        for(i=0U; i < depth; i++){

            if(stack[i] == ptr){

                BLINK_ERROR("reference cycle detected")
                retval = false;
                break;
            }
        }

        if(i == depth){
        
            if(castConstTypeDef(ptr)->type.isDynamic){

                /* dynamic reference to dynamic reference */
                if(dynamic){

                    BLINK_ERROR("dynamic reference must resolve to a group")
                    retval = false;
                }
                else{

                    dynamic = true;
                }
            }

            if(retval && castConstTypeDef(ptr)->type.isSequence){   /*lint !e9007 no side effect */

                /* sequence of a sequence */
                if(sequence){

                    BLINK_ERROR("cannot have a sequence of a sequence")
                    retval = false;
                }
                else{

                    sequence = true;
                }
            }

            if(retval){

                depth++;
                if(depth == (sizeof(stack)/sizeof(*stack))){

                    BLINK_ERROR("depth")
                    retval = false;
                }
                else{

                    stack[depth] = ptr;
                    ptr = castConstTypeDef(ptr)->type.resolvedRef;
                }
            }
        }
    }

    if(dynamic && (ptr->type != BLINK_ELEM_GROUP)){

        BLINK_ERROR("dynamic reference must resolve to a group")
        retval = false;
    }

    return retval;
}

static bool testSuperGroupReferenceConstraint(const struct blink_schema *self, const struct blink_group *group)
{
    bool retval = true;
    struct blink_list_element *ptr = group->s;
    struct blink_list_element *stack[BLINK_LINK_DEPTH];
    size_t depth = 0U;
    size_t i;

    (void)memset(stack, 0, sizeof(stack));

    while(retval && (ptr->type == BLINK_ELEM_TYPE) && (castTypeDef(ptr)->type.tag == BLINK_ITYPE_REF)){ /*lint !e9007 no side effect */

        for(i=0U; i < depth; i++){

            if(stack[i] == ptr){

                BLINK_ERROR("reference cycle detected")
                retval = false;
                break;
            }
        }

        if(i == depth){

            if(castTypeDef(ptr)->type.isSequence){

                BLINK_ERROR("supergroup cannot be sequence");
                retval = false;
            }
            else{

                if(castTypeDef(ptr)->type.isDynamic){

                    BLINK_ERROR("supergroup cannot be dynamic")
                    retval = false;
                }
                else{

                    depth++;
                    if(depth == (sizeof(stack)/sizeof(*stack))){

                        BLINK_ERROR("depth")
                        retval = false;
                    }
                    else{

                        stack[depth] = ptr;
                        ptr = castTypeDef(ptr)->type.resolvedRef;
                    }
                }
            }
        }
    }

    if(retval){

        if(ptr->type != BLINK_ELEM_GROUP){

            BLINK_ERROR("supergroup must be a group")
            retval = false;
        }
        else{

            if(castGroup(ptr) == group){

                BLINK_ERROR("group cannot be own supergroup")
                retval = false;
            }
        }
    }
    
    return retval;
}

static bool testSuperGroupShadowConstraint(const struct blink_schema *self, const struct blink_group *group)
{
    struct blink_field_iterator fi;
    BLINK_InitFieldIterator(&fi, group);
    const struct blink_field *field = BLINK_NextField(&fi);

    while(field != NULL){

        struct blink_field_iterator fii;
        BLINK_InitFieldIterator(&fii, group);
        const struct blink_field *f = BLINK_NextField(&fii);

        while(f != NULL){
            
            if(f != field){

                if(f->nameLen == field->nameLen){

                    if(memcmp(f->name, field->name, f->nameLen) == 0){

                        BLINK_ERROR("field name shadowed in subgroup")
                        return false;
                    }
                }
            }

            f = BLINK_NextField(&fii);
        }
    
        field = BLINK_NextField(&fi);
    }

    return true;
}

static struct blink_list_element *newListElement(struct blink_schema *self, struct blink_list_element **head, enum blink_list_type type)
{
    BLINK_ASSERT(self != NULL)
    BLINK_ASSERT(head != NULL)

    struct blink_list_element *retval = NULL;
    struct blink_element *element;

    if(type != BLINK_ELEM_NULL){

        element = (struct blink_element *)self->calloc(1, sizeof(struct blink_element));    /*lint !e9087 casting to the type that was allocated */

        if(element != NULL){

            if(self->elements == NULL){

                self->elements = element;
            }
            else{

                element->next = self->elements;
                self->elements = element;
            }
    
            element->ptr = self->calloc(1, sizeof(struct blink_list_element));

            retval = element->ptr;
    
            if(retval != NULL){

                if(*head == NULL){

                    *head = retval;
                }
                else{

                    struct blink_list_element *ptr = *head;

                    while(ptr->next != NULL){

                        ptr = ptr->next;
                    }

                    ptr->next = retval;                
                }

                switch(type){
                case BLINK_ELEM_NS:
                    retval->ptr = self->calloc(1, sizeof(struct blink_namespace));
                    break;                    
                case BLINK_ELEM_GROUP:
                    retval->ptr = self->calloc(1, sizeof(struct blink_group));
                    break;            
                case BLINK_ELEM_FIELD:
                    retval->ptr = self->calloc(1, sizeof(struct blink_field));
                    break;            
                case BLINK_ELEM_ENUM:
                    retval->ptr = self->calloc(1, sizeof(struct blink_enum));
                    break;            
                case BLINK_ELEM_SYMBOL:
                    retval->ptr = self->calloc(1, sizeof(struct blink_symbol));
                    break;            
                case BLINK_ELEM_TYPE:
                    retval->ptr = self->calloc(1, sizeof(struct blink_type_def));
                    break;            
                case BLINK_ELEM_ANNOTE:
                    retval->ptr = self->calloc(1, sizeof(struct blink_annote));
                    break;            
                case BLINK_ELEM_INCR_ANNOTE:
                    retval->ptr = self->calloc(1, sizeof(struct blink_incr_annote));
                    break;            
                case BLINK_ELEM_NULL:
                default:
                    /*unused*/
                    break;
                }

                if(type != BLINK_ELEM_NULL){

                    if(retval->ptr == NULL){

                        BLINK_ERROR("calloc()")
                        retval = NULL;
                    }
                    else{

                        retval->type = type;
                    }
                }
            }
            else{

                /* calloc() */
                BLINK_ERROR("calloc()")
            }
        }
        else{

            /* calloc() */
            BLINK_ERROR("calloc()")
        }
    }
    else{
        
        /* bad argument */
        BLINK_ERROR("cannot create a BLINK_ELEM_NULL")
    }

    return retval;
}

static struct blink_list_element *searchListByName(struct blink_list_element *head, const char *name, size_t nameLen)
{
    struct blink_list_element *ptr = head;
    struct blink_list_element *retval = NULL;

    /* all list elements have the same first two members */
    struct blink_base_type {
        const char *name;
        size_t nameLen;
    };

    BLINK_ASSERT(offsetof(struct blink_namespace, name) == offsetof(struct blink_base_type, name))
    BLINK_ASSERT(offsetof(struct blink_namespace, nameLen) == offsetof(struct blink_base_type, nameLen))

    BLINK_ASSERT(offsetof(struct blink_group, name) == offsetof(struct blink_base_type, name))
    BLINK_ASSERT(offsetof(struct blink_group, nameLen) == offsetof(struct blink_base_type, nameLen))

    BLINK_ASSERT(offsetof(struct blink_field, name) == offsetof(struct blink_base_type, name))
    BLINK_ASSERT(offsetof(struct blink_field, nameLen) == offsetof(struct blink_base_type, nameLen))
    
    BLINK_ASSERT(offsetof(struct blink_type_def, name) == offsetof(struct blink_base_type, name))
    BLINK_ASSERT(offsetof(struct blink_type_def, nameLen) == offsetof(struct blink_base_type, nameLen))

    BLINK_ASSERT(offsetof(struct blink_enum, name) == offsetof(struct blink_base_type, name))
    BLINK_ASSERT(offsetof(struct blink_enum, nameLen) == offsetof(struct blink_base_type, nameLen))

    BLINK_ASSERT(offsetof(struct blink_symbol, name) == offsetof(struct blink_base_type, name))
    BLINK_ASSERT(offsetof(struct blink_symbol, nameLen) == offsetof(struct blink_base_type, nameLen))

        
    BLINK_ASSERT(   (ptr == NULL) ||
                    (ptr->type == BLINK_ELEM_NS)||
                    (ptr->type == BLINK_ELEM_GROUP)||
                    (ptr->type == BLINK_ELEM_FIELD)||
                    (ptr->type == BLINK_ELEM_TYPE)||
                    (ptr->type == BLINK_ELEM_ENUM)||
                    (ptr->type == BLINK_ELEM_SYMBOL))
    
    while(ptr != NULL){

        const struct blink_base_type *deref = (const struct blink_base_type *)ptr->ptr; /*lint !e9087 'struct blink_base_type' verified as common subtype by BLINK_ASSERT */

        if(deref->nameLen == nameLen){

            if(memcmp(deref->name, name, nameLen) == 0){

                retval = ptr;
                break;
            }
        }

        ptr = ptr->next;
    }

    return retval;
}

static void splitCName(const char *in, size_t inLen, const char **nsName, size_t *nsNameLen, const char **name, size_t *nameLen)
{
    BLINK_ASSERT(in != NULL)
    BLINK_ASSERT(nsName != NULL)
    BLINK_ASSERT(nsNameLen != NULL)
    BLINK_ASSERT(name != NULL)
    BLINK_ASSERT(nameLen != NULL)
    
    size_t i;

    *nsName = NULL;
    *nsNameLen = 0U;
    *name = NULL;
    *nameLen = 0U;
    
    for(i=0U; i < inLen; i++){

        if(in[i] == ':'){
            *nsName = in;
            *nsNameLen = i;
            *name = &in[i+1U];
            *nameLen = inLen - (i+1U);
            break;
        }
    }

    if(i == inLen){

        *name = in;
        *nameLen = inLen;        
    }
}

static struct blink_list_element *getTerminal(struct blink_list_element *element, bool *dynamic)
{
    BLINK_ASSERT(element != NULL)
    BLINK_ASSERT(dynamic != NULL)

    struct blink_list_element *ptr = element;
    *dynamic = false;

    while((ptr->type == BLINK_ELEM_TYPE) && (castTypeDef(ptr)->type.tag == BLINK_ITYPE_REF)){   /*lint !e9007 no side effect */

        if(castTypeDef(ptr)->type.isDynamic){

            *dynamic = true;
        }
        
        ptr = castTypeDef(ptr)->type.resolvedRef;
    }

    return ptr;
}

static struct blink_list_element *initDefinitionIterator(struct blink_def_iterator *iter, struct blink_schema *schema)
{
    BLINK_ASSERT(iter != NULL)
    BLINK_ASSERT(schema != NULL)

    (void)memset(iter, 0x0, sizeof(*iter));
    iter->ns = schema->ns;

    while(iter->ns != NULL){
        iter->def = castNamespace(iter->ns)->defs;
        if(iter->def != NULL){

            break;
        }
        else{

            iter->ns = iter->ns->next;
        }
    }

    return iter->def;
}

static struct blink_list_element *nextDefinition(struct blink_def_iterator *iter)
{
    BLINK_ASSERT(iter != NULL)
    
    struct blink_list_element *retval = iter->def;

    if(retval != NULL){

        iter->def = iter->def->next;

        if(iter->def == NULL){

            while(iter->ns != NULL){

                iter->ns = iter->ns->next;

                if(iter->ns != NULL){

                    iter->def = castNamespace(iter->ns)->defs;

                    if(iter->def != NULL){

                        break;
                    }    
                }    
            }
        }
    }

    return retval;            
}

static struct blink_enum *castEnum(struct blink_list_element *self)
{
    return (self == NULL) ? NULL : (struct blink_enum *)self->ptr;  /*lint !e9087 void pointer was used as intermediary */
}

static struct blink_symbol *castSymbol(struct blink_list_element *self)
{
    return (self == NULL) ? NULL : (struct blink_symbol *)self->ptr;    /*lint !e9087 void pointer was used as intermediary */
}

static struct blink_field *castField(struct blink_list_element *self)
{
    return (self == NULL) ? NULL : (struct blink_field *)self->ptr; /*lint !e9087 void pointer was used as intermediary */
}

static struct blink_group *castGroup(struct blink_list_element *self)
{
    return (self == NULL) ? NULL : (struct blink_group *)self->ptr; /*lint !e9087 void pointer was used as intermediary */
}

static struct blink_namespace *castNamespace(struct blink_list_element *self)
{
    return (self == NULL) ? NULL : (struct blink_namespace *)self->ptr; /*lint !e9087 void pointer was used as intermediary */
}

static struct blink_type_def *castTypeDef(struct blink_list_element *self)
{
    return (self == NULL) ? NULL : (struct blink_type_def *)self->ptr;  /*lint !e9087 void pointer was used as intermediary */
}

static struct blink_annote *castAnnote(struct blink_list_element *self)
{
    return (self == NULL) ? NULL : (struct blink_annote *)self->ptr;    /*lint !e9087 void pointer was used as intermediary */
}

static struct blink_incr_annote *castIncrAnnote(struct blink_list_element *self)
{
    return (self == NULL) ? NULL : (struct blink_incr_annote *)self->ptr;   /*lint !e9087 void pointer was used as intermediary */
}

static const struct blink_type_def *castConstTypeDef(const struct blink_list_element *self)
{
    return (self == NULL) ? NULL : (const struct blink_type_def *)self->ptr;    /*lint !e9087 void pointer was used as intermediary */
}
