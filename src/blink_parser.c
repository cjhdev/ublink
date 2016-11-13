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
    BLINK_ITYPE_REF                /**< reference */
};

/* structs ************************************************************/

/** type */
struct blink_type {
    bool isDynamic;             /**< reference is dynamic (applicable to #BLINK_ITYPE_REF) */
    bool isSequence;            /**< this is a SEQUENCE of type */                
    uint32_t size;              /**< size attribute (applicable to #BLINK_ITYPE_BINARY, #BLINK_ITYPE_FIXED, and #BLINK_ITYPE_STRING) */
    const char *ref;            /**< name of reference (applicable to #BLINK_ITYPE_REF) */
    size_t refLen;                                  /**< byte length of `ref` */
    enum blink_itype_tag tag;                       /**< what type is this? */
    const struct blink_list_element *resolvedRef;   /**< `ref` resolves to this structure */
};

/** field */
struct blink_field {
    const char *name;           /**< name of this field */
    size_t nameLen;             /**< byte length of `name` */
    bool isOptional;            /**< field is optional */
    struct blink_type type;     /**< field type information */
};

/** group */
struct blink_group {
    const char *name;               /**< name of this group */
    size_t nameLen;                 /**< byte length of `name` */
    bool hasID;                     /**< group has an ID */
    uint64_t id;                    /**< group ID */
    const char *superGroup;         /**< name of super group */
    size_t superGroupLen;           /**< byte length of supergroup name */    
    const struct blink_list_element *s;     /**< optional supergroup */
    struct blink_list_element *f;   /**< fields belonging to group */
};

/** enumeration symbol */
struct blink_symbol {
    const char *name;               /**< name of symbol */
    size_t nameLen;                 /**< byte length of `name` */
    int32_t value;                  /**< integer value */
    bool implicitID;                /**< true if `value` is not explicitly defined */
};

/** enumeration */
struct blink_enum {
    const char *name;           /**< name of this field */
    size_t nameLen;             /**< byte length of `name` */
    struct blink_list_element *s;     /**< symbols belonging to enumeration */    
};

/** type definition */
struct blink_type_def {
    const char *name;               /**< name of type definition */
    size_t nameLen;                 /**< byte length of `name` */
    struct blink_type type;         /**< type information */
};

/** annotation - key-value meta data */
struct blink_annote {
    const char *key;                /**< key */
    size_t keyLen;                  /**< byte length of `key` */
    const char *value;              /**< value */            
    size_t valueLen;                /**< byte length of `value` */
    uint64_t number;                /**< sometimes value is a numeric value */
    bool isNumeric;                 /**< true if this annotation is a numeric value */  
};

struct blink_inline_annote {
    const char *name;               /**< name to apply annote to */
    size_t nameLen;                 /**< byte length of `name` */
    struct blink_list_element *a;   /**< list of annotes to apply to `name` */
};

/** namespace */
struct blink_namespace {    
    const char *name;   /**< name of this namespace */
    size_t nameLen;     /**< byte length of `name` */  
    /** list of groups, enums, and types in this namespace */
    struct blink_list_element *defs;      
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
        BLINK_ELEM_INLINE_ANNOTE        /**< blink_inline_annote */        
    } type;                             /**< type allocated at `ptr` */
    
    void *ptr;                          /**< points to instance of `type` */
};

struct blink_def_iterator {

    struct blink_list_element *ns;
    struct blink_list_element *def;
};

/* static prototypes **************************************************/

static struct blink_schema *parse(struct blink_schema *self, const char *in, size_t inLen);
static struct blink_type *parseType(const char *in, size_t inLen, size_t *read, struct blink_type *type);
static bool parseAnnote(struct blink_schema *self, const char *in, size_t inLen, size_t *read, const char **key, size_t *keyLen, const char **value, size_t *valueLen);

static struct blink_list_element *newListElement(struct blink_schema *self, struct blink_list_element **head, enum blink_list_type type);
static struct blink_list_element *searchListByName(struct blink_list_element *head, const char *name, size_t nameLen);

static void splitCName(const char *in, size_t inLen, const char **nsName, size_t *nsNameLen, const char **name, size_t *nameLen);

static bool resolveDefinitions(struct blink_schema *self);
static struct blink_list_element *resolve(struct blink_schema *self, const char *cName, size_t cNameLen);

static bool testConstraints(struct blink_schema *self);
static bool testReferenceConstraint(const struct blink_schema *self, const struct blink_list_element *reference);
static bool testSuperGroupReferenceConstraint(const struct blink_schema *self, const struct blink_group *group);
static bool testSuperGroupShadowConstraint(const struct blink_schema *self, const struct blink_group *group);

static bool testNextEnumValue(const struct blink_list_element *s, int32_t value);
static bool nextEnumValue(const struct blink_list_element *s, int32_t *value);

static struct blink_enum *castEnum(struct blink_list_element *self);
static struct blink_symbol *castSymbol(struct blink_list_element *self);
static const struct blink_symbol *castConstSymbol(const struct blink_list_element *self);
static struct blink_field *castField(struct blink_list_element *self);
static const struct blink_field *castConstField(const struct blink_list_element *self);
static struct blink_group *castGroup(struct blink_list_element *self);
static const struct blink_group *castConstGroup(const struct blink_list_element *self);
static struct blink_namespace *castNamespace(struct blink_list_element *self);
static struct blink_type_def *castTypeDef(struct blink_list_element *self);
static const struct blink_type_def *castConstTypeDef(const struct blink_list_element *self);

static struct blink_list_element *getTerminal(struct blink_list_element *element, bool *dynamic);

static struct blink_list_element *initDefinitionIterator(struct blink_def_iterator *iter, struct blink_schema *schema);
static struct blink_list_element *nextDefinition(struct blink_def_iterator *iter);

/* functions **********************************************************/

struct blink_schema *BLINK_InitSchema(struct blink_schema *schema, fn_blink_calloc_t calloc, fn_blink_free_t free)
{
    BLINK_ASSERT(schema != NULL)
    BLINK_ASSERT(calloc != NULL)

    memset(schema, 0x0, sizeof(*schema));

    schema->calloc = calloc;
    schema->free = free;
    
    return schema;
}

//todo: yes, memory leak
void BLINK_DestroySchema(struct blink_schema *self)
{
    BLINK_ASSERT(self != NULL)    
    memset(self, 0x0, sizeof(*self));
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

        if((defPtr->type == BLINK_ELEM_GROUP) && castConstGroup(defPtr)->hasID && (castConstGroup(defPtr)->id == id)){

            retval = castConstGroup(defPtr);
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
    
    memset(iter, 0, sizeof(*iter));

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

            retval = castConstField(self->field[self->depth]);
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

    struct blink_schema *retval;
    
    if((parse(self, in, inLen) == self) && resolveDefinitions(self) && testConstraints(self)){

        retval = self;
    }
    else{

        BLINK_DestroySchema(self);
        retval = NULL;
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

bool BLINK_FieldIsOptional(const struct blink_field *self)
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

const char *BLINK_GetFieldRef(const struct blink_field *self, size_t *refLen)
{
    BLINK_ASSERT(self != NULL)
    BLINK_ASSERT(refLen != NULL)

    *refLen = self->nameLen;
    return self->name;
}

const struct blink_symbol *BLINK_GetSymbolValue(const struct blink_enum *self, const char *name, size_t nameLen, int32_t *value)
{
    BLINK_ASSERT(self != NULL)
    BLINK_ASSERT(name != NULL)
    BLINK_ASSERT(value != NULL)

    const struct blink_symbol *retval = NULL;
    const struct blink_list_element *element = searchListByName(self->s, name, nameLen);

    if(element != NULL){

        retval = castConstSymbol(element);
        *value = retval->value;
    }

    return retval;
}

const struct blink_symbol *BLINK_GetSymbolName(const struct blink_enum *self, int32_t value, const char **name, size_t *nameLen)
{
    BLINK_ASSERT(self != NULL)
    BLINK_ASSERT(name != NULL)
    BLINK_ASSERT(nameLen != NULL)
    
    const struct blink_list_element *ptr = self->s;
    const struct blink_symbol *retval = NULL;

    while(ptr != NULL){

        if(castConstSymbol(ptr)->value == value){

            retval = castConstSymbol(ptr);
            *name = retval->name;
            *nameLen = retval->nameLen;
            break;
        }

        ptr = ptr->next;
    }

    return retval;
}

/* static functions ***************************************************/

static struct blink_schema *parse(struct blink_schema *self, const char *in, size_t inLen)
{
    BLINK_ASSERT(self != NULL)
    BLINK_ASSERT(in != NULL)
    
    size_t pos = 0U;
    struct blink_schema *retval = NULL;
    size_t read;
    size_t nextRead;

    struct blink_namespace *ns;
    
    union blink_token_value value;

    enum blink_token tok;
    enum blink_token nextTok;
    
    struct blink_list_element *element;

    /* specific namespace */
    if(BLINK_GetToken(in, inLen, &read, &value, NULL) == TOK_NAMESPACE){

        pos += read;

        if(BLINK_GetToken(&in[pos], inLen - pos, &read, &value, NULL) != TOK_NAME){

            BLINK_ERROR("expecting a namespace name")
            return retval;          
        }
        
        pos += read;
    }
    else{

        memset(&value, 0, sizeof(value));
    }

    element = searchListByName(self->ns, value.literal.ptr, value.literal.len);

    /* namespace not yet defined */
    if(element == NULL){

        element = newListElement(self, &self->ns, BLINK_ELEM_NS);

        if(element == NULL){

            return retval;
        }

        ns = castNamespace(element);

        ns->name = value.literal.ptr;
        ns->nameLen = value.literal.len;
    }
    else{

        ns = castNamespace(element);
    }

    /* parse all definitions */
    while(BLINK_GetToken(&in[pos], inLen-pos, &read, &value, NULL) != TOK_EOF){

        if(BLINK_GetToken(&in[pos], inLen-pos, &read, &value, NULL) == TOK_NAME){

            pos += read;

            const char *name = value.literal.ptr;
            size_t nameLen = value.literal.len;
            
            uint64_t id = 0U;
            bool hasID = false;

            /* group definitions may have an identifier */
            if(BLINK_GetToken(&in[pos], inLen - pos, &read, &value, NULL) == TOK_SLASH){

                pos += read;

                if(BLINK_GetToken(&in[pos], inLen - pos, &read, &value, NULL) != TOK_NUMBER){

                    BLINK_ERROR("error: expecting integer or hexnum")
                    return retval;
                }

                pos += read;
                id = value.number;
                hasID = true;                
            }

            /* type or enum */
            if(!hasID && BLINK_GetToken(&in[pos], inLen - pos, &read, &value, NULL) == TOK_EQUAL){

                pos += read;

                if(searchListByName(ns->defs, name, nameLen) != NULL){

                    BLINK_ERROR("duplicate definition name");
                    return retval;
                }

                tok = BLINK_GetToken(&in[pos], inLen - pos, &read, &value, NULL);

                nextTok = BLINK_GetToken(&in[pos+read], inLen - (pos + read), &nextRead, &value, NULL);

                /* enum */
                if((tok == TOK_BAR) || ((tok == TOK_NAME) && ((nextTok == TOK_SLASH) || (nextTok == TOK_BAR)))){

                    if(tok == TOK_BAR){

                        pos += read;
                    }

                    element = newListElement(self, &ns->defs, BLINK_ELEM_ENUM);

                    if(element == NULL){

                        return retval;
                    }

                    struct blink_enum *e = castEnum(element);

                    e->name = name;
                    e->nameLen = nameLen;

                    bool single = (tok == TOK_BAR) ? true : false;

                    read = 0U;

                    do{

                        pos += read;

                        if(BLINK_GetToken(&in[pos], inLen - pos, &read, &value, NULL) != TOK_NAME){

                            BLINK_ERROR("expecting enum symbol name")
                            return retval;
                        }

                        pos += read;

                        struct blink_symbol s;
                        memset(&s, 0, sizeof(s));

                        s.name = value.literal.ptr;
                        s.nameLen = value.literal.len;
                            
                        if(BLINK_GetToken(&in[pos], inLen - pos, &read, &value, NULL) == TOK_SLASH){

                            pos += read;

                            if(BLINK_GetToken(&in[pos], inLen - pos, &read, &value, NULL) != TOK_NUMBER){
                                
                                BLINK_ERROR("expecting enum symbol value")
                                return retval;
                            }

                            pos += read;
                                
                            s.value = value.number;
                            s.implicitID = false;                                
                        }
                        else{

                            s.implicitID = true;
                        }

                        if(searchListByName(e->s, s.name, s.nameLen) != NULL){
                            
                            BLINK_ERROR("duplicate enum symbol name")
                            return retval;
                        }

                        if(s.implicitID){
                    
                            if(!nextEnumValue(e->s, &s.value)){

                                BLINK_ERROR("no next implicit enum value possible")
                                return retval;
                            }
                        }
                        else{

                            if(!testNextEnumValue(e->s, s.value)){

                                BLINK_ERROR("enum value is ambiguous")
                                return retval;
                            }
                        }

                        element = newListElement(self, &e->s, BLINK_ELEM_SYMBOL);

                        if(element == NULL){

                            return retval;
                        }

                        *castSymbol(element) = s; /* copy */
                    }
                    while(!single && (BLINK_GetToken(&in[pos], inLen - pos, &read, &value, NULL) == TOK_BAR));
                }
                /* type */
                else{

                    struct blink_type type;

                    if(parseType(&in[pos], inLen - pos, &read, &type) == NULL){
                        
                        return retval;
                    }
                
                    pos += read;

                    element = newListElement(self, &ns->defs, BLINK_ELEM_TYPE);

                    if(element == NULL){

                        return retval;
                    }

                    struct blink_type_def *t = castTypeDef(element);

                    t->name = name;
                    t->nameLen = nameLen;
                    t->type = type; /* copy */                                                    
                }
            }
            else if(!hasID && BLINK_GetToken(&in[pos], inLen - pos, &read, &value, NULL) == TOK_LARROW){

                BLINK_ERROR("todo: inline annote")
                return retval;
            }
            else{

                /* definition name must be unique */
                if(searchListByName(ns->defs, name, nameLen) != NULL){
                
                    BLINK_ERROR("duplicate definition name")
                    return retval;
                }

                element = newListElement(self, &ns->defs, BLINK_ELEM_GROUP);

                if(element == NULL){

                    return retval;
                }
                
                struct blink_group *g = castGroup(element);

                g->name = name;
                g->nameLen = nameLen;
                g->id = id;
                g->hasID = hasID;

                if(BLINK_GetToken(&in[pos], inLen - pos, &read, &value, NULL) == TOK_COLON){

                    pos += read;
                    
                    if((BLINK_GetToken(&in[pos], inLen - pos, &read, &value, NULL) != TOK_CNAME) && (BLINK_GetToken(&in[pos], inLen - pos, &read, &value, NULL) != TOK_NAME)){

                        BLINK_ERROR("expecting super class name (qname)")
                        return retval;
                    }

                    pos += read;

                    g->superGroup = value.literal.ptr;
                    g->superGroupLen = value.literal.len;
                }

                if(BLINK_GetToken(&in[pos], inLen - pos, &read, &value, NULL) == TOK_RARROW){

                    do{

                        pos += read;

                        const char *fieldName;
                        size_t fieldNameLen;
                        struct blink_type t;
                        bool isOptional;
                        
                        if(parseType(&in[pos], inLen - pos, &read, &t) != &t){

                            return retval;
                        }
                        
                        pos += read;

                        if(BLINK_GetToken(&in[pos], inLen - pos, &read, &value, NULL) != TOK_NAME){

                            BLINK_ERROR("expecting name")
                            return retval;
                        }

                        pos += read;

                        fieldName = value.literal.ptr;
                        fieldNameLen = value.literal.len;

                        if(BLINK_GetToken(&in[pos], inLen - pos, &read, &value, NULL) == TOK_QUESTION){

                            pos += read;
                            isOptional = true;
                        }
                        else{

                            isOptional = false;
                        }

                        if(searchListByName(g->f, fieldName, fieldNameLen) != NULL){

                            BLINK_ERROR("duplicate field name");
                            return retval;
                        }

                        element = newListElement(self, &g->f, BLINK_ELEM_FIELD);

                        struct blink_field *f = castField(element);

                        f->type = t;    /* copy */
                        f->name = fieldName;
                        f->nameLen = fieldNameLen;
                        f->isOptional = isOptional;                        
                    }
                    while(BLINK_GetToken(&in[pos], inLen - pos, &read, &value, NULL) == TOK_COMMA);                   
                }                                    
            }
        }
        else{

            /* null terminated input */
            if(in[pos + read] == '\0'){

                retval = self;
            }
            else{

                BLINK_ERROR("unknown character %u", in[pos + read])
                return retval;
            }
            break;
        }            
    }

    retval = self;    
    
    return retval; 
}

static struct blink_type *parseType(const char *in, size_t inLen, size_t *read, struct blink_type *type)
{
    BLINK_ASSERT(in != NULL)
    BLINK_ASSERT(read != NULL)
    BLINK_ASSERT(type != NULL)

    size_t pos = 0U;
    size_t r;
    union blink_token_value value;
    enum blink_token tok = BLINK_GetToken(&in[pos], inLen - pos, &r, &value, NULL);
    struct blink_type *retval = NULL;

    static const enum blink_token tokenToType[] = {
        TOK_STRING,
        TOK_BINARY,
        TOK_FIXED,
        TOK_BOOL,
        TOK_U8,
        TOK_U16,
        TOK_U32,
        TOK_U64,
        TOK_I8,
        TOK_I16,
        TOK_I32,
        TOK_I64,
        TOK_F64,
        TOK_DATE,
        TOK_TIME_OF_DAY_MILLI,
        TOK_TIME_OF_DAY_NANO,
        TOK_MILLI_TIME,
        TOK_NANO_TIME,
        TOK_DECIMAL,
        TOK_OBJECT,
    };
    
    switch(tok){
    case TOK_U8:
    case TOK_U16:
    case TOK_U32:
    case TOK_U64:
    case TOK_I8:
    case TOK_I16:
    case TOK_I32:
    case TOK_I64:
    case TOK_F64:
    case TOK_STRING:
    case TOK_BINARY:
    case TOK_FIXED:
    case TOK_DECIMAL:
    case TOK_DATE:
    case TOK_MILLI_TIME:
    case TOK_NANO_TIME:
    case TOK_TIME_OF_DAY_MILLI:
    case TOK_TIME_OF_DAY_NANO:

        BLINK_ASSERT(tok < (sizeof(tokenToType)/sizeof(*tokenToType)))

        type->tag = tokenToType[tok];

        pos += r;

        if((tok == TOK_STRING) || (tok == TOK_BINARY) || (tok == TOK_FIXED)){

            if(BLINK_GetToken(&in[pos], inLen - pos, &r, &value, NULL) == TOK_LPAREN){

                pos += r;

                if(BLINK_GetToken(&in[pos], inLen - pos, &r, &value, NULL) == TOK_NUMBER){

                    pos += r;
                }
                else{

                    BLINK_ERROR("expecting a size");
                    return retval;
                }

                if(value.number > 0xffffffffU){

                    BLINK_ERROR("size decode but is out of range")
                    return retval;
                }

                type->size = (uint32_t)value.number;

                if(BLINK_GetToken(&in[pos], inLen - pos, &r, &value, NULL) == TOK_RPAREN){

                    pos += r;
                }   
                else{

                    BLINK_ERROR("expecting a ')'")
                    return retval;
                }
            }
            else if(tok == TOK_FIXED){

                BLINK_ERROR("expecting a '('")
                return retval;
            }
            else{

                type->size = 0xffffffffU;
            }
        }
        break;

    case TOK_NAME:
    case TOK_CNAME:

        pos += r;

        type->ref = value.literal.ptr;
        type->refLen = value.literal.len;
        type->tag = BLINK_ITYPE_REF;

        if(BLINK_GetToken(&in[pos], inLen - pos, &r, &value, NULL) == TOK_STAR){

            pos += r;
            type->isDynamic = true;            
        }
        break;

    default:

        BLINK_ERROR("expecting a type");
        return retval;                                    
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
            return retval;
        }                        
    }
    else{

        type->isSequence = false;
    }

    *read = pos;

    return type;
}

/* try to parse an annotation and return true if either no annotation is found, or annotation is found with correct encoding */
static bool parseAnnote(struct blink_schema *self, const char *in, size_t inLen, size_t *read, const char **key, size_t *keyLen, const char **value, size_t *valueLen)
{
    BLINK_ASSERT(self != NULL)
    BLINK_ASSERT(read != NULL)
    BLINK_ASSERT(key != NULL)
    BLINK_ASSERT(keyLen != NULL)
    BLINK_ASSERT(value != NULL)
    BLINK_ASSERT(valueLen != NULL)

    size_t pos = 0U;
    size_t r;
    union blink_token_value v;
    enum blink_token type;
    bool retval = true;
    
    if(BLINK_GetToken(&in[pos], inLen - pos, &r, &v, NULL) == TOK_AT){

        pos += r;

        retval = false;

        type = BLINK_GetToken(&in[pos], inLen - pos, &r, &v, NULL);

        pos += r;

        switch(type){
        case TOK_CNAME:
        case TOK_NAME:
            *key = v.literal.ptr;
            *keyLen = v.literal.len;        
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

            *key = BLINK_TokenToString(type, keyLen);

            BLINK_ASSERT(key != NULL)
            
            break;
            
        default:
            BLINK_ERROR("unexpected token")
            break;
        }

        if(BLINK_GetToken(&in[pos], inLen - pos, &r, &v, NULL) == TOK_EQUAL){

            pos += r;

            if(BLINK_GetToken(&in[pos], inLen - pos, &r, &v, NULL) == TOK_LITERAL){

                pos += r;
                *read = r;
                retval = true;
            }
            else{

                BLINK_ERROR("expecting literal")
            }            
        }
        else{

            BLINK_ERROR("expecting '='")            
        }
    }

    return retval;
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

                    BLINK_ERROR("unresolved")    
                    return false;
                }
            }

            fieldPtr = g->f;

            while(fieldPtr != NULL){

                f = castField(fieldPtr);

                if(f->type.tag == BLINK_ITYPE_REF){

                    f->type.resolvedRef = resolve(self, f->type.ref, f->type.refLen);

                    if(f->type.resolvedRef == NULL){

                        BLINK_ERROR("unresolved");
                        return false;
                    }
                }

                fieldPtr = fieldPtr->next;
            }
            break;

        default:
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

    /* test supergroups */

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

static bool testNextEnumValue(const struct blink_list_element *s, int32_t value)
{
    bool retval = true;
    const struct blink_list_element *ptr = s;

    while(ptr != NULL){

        if(ptr->next == NULL){

            if(value <= castSymbol(ptr)->value){

                retval = false;
            }
        }

        ptr = ptr->next;
    }

    return retval;
}

static bool nextEnumValue(const struct blink_list_element *s, int32_t *value)
{
    *value = 0;
    bool retval = true;
    const struct blink_list_element *ptr = s;

    while(ptr != NULL){

        if(ptr->next == NULL){

            *value = castSymbol(ptr)->value;

            if(*value == INT32_MAX){

                retval = false;
            }
            else{

                (*value)++;
            }
        }

        ptr = ptr->next;
    }

    return retval;
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

    while(retval && (ptr->type == BLINK_ELEM_TYPE) && (castConstTypeDef(ptr)->type.tag == BLINK_ITYPE_REF)){

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

            if(retval && castConstTypeDef(ptr)->type.isSequence){

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
    const struct blink_list_element *ptr = group->s;
    const struct blink_list_element *stack[BLINK_LINK_DEPTH];
    size_t depth = 0U;
    size_t i;

    while(retval && (ptr->type == BLINK_ELEM_TYPE) && (castConstTypeDef(ptr)->type.tag == BLINK_ITYPE_REF)){
    
        for(i=0U; i < depth; i++){

            if(stack[i] == ptr){

                BLINK_ERROR("reference cycle detected")
                retval = false;
                break;
            }
        }

        if(i == depth){

            if(castConstTypeDef(ptr)->type.isSequence){

                BLINK_ERROR("supergroup cannot be sequence");
                retval = false;
            }

            if(retval && castConstTypeDef(ptr)->type.isDynamic){

                BLINK_ERROR("supergroup cannot be dynamic");
                retval = false;
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

                if((f->nameLen == field->nameLen) && (memcmp(f->name, field->name, f->nameLen) == 0)){

                    BLINK_ERROR("field name shadowed in subgroup")
                    return false;
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

    struct blink_list_element *retval;

    if(type != BLINK_ELEM_NULL){
    
        retval = self->calloc(1, sizeof(struct blink_list_element));

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
            case BLINK_ELEM_NULL:
            default:
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

            BLINK_ERROR("calloc()")
        }
    }
    else{

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

    while(ptr != NULL){

        const struct blink_base_type *deref = (const struct blink_base_type *)ptr->ptr;

        if((deref->nameLen == nameLen) && (memcmp(deref->name, name, nameLen) == 0)){

            retval = ptr;
            break;
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
            *name = &in[i+1];
            *nameLen = inLen - (i+1);
            break;
        }
    }

    if(i == inLen){

        *name = in;
        *nameLen = inLen;        
    }
}

static struct blink_enum *castEnum(struct blink_list_element *self)
{
    struct blink_enum *retval = NULL;
    if(self != NULL){
        BLINK_ASSERT(self->type == BLINK_ELEM_ENUM)
        retval = (struct blink_enum *)self->ptr;
    }
    return retval;
}

static struct blink_symbol *castSymbol(struct blink_list_element *self)
{
    struct blink_symbol *retval = NULL;
    if(self != NULL){
        BLINK_ASSERT(self->type == BLINK_ELEM_SYMBOL)
        retval = (struct blink_symbol *)self->ptr;
    }
    return retval;
}

static const struct blink_symbol *castConstSymbol(const struct blink_list_element *self)
{
    const struct blink_symbol *retval = NULL;
    if(self != NULL){
        BLINK_ASSERT(self->type == BLINK_ELEM_SYMBOL)
        retval = (const struct blink_symbol *)self->ptr;
    }
    return retval;
}

static struct blink_field *castField(struct blink_list_element *self)
{
    struct blink_field *retval = NULL;
    if(self != NULL){
        BLINK_ASSERT(self->type == BLINK_ELEM_FIELD)
        retval = (struct blink_field *)self->ptr;
    }
    return retval;
}

static const struct blink_field *castConstField(const struct blink_list_element *self)
{
    const struct blink_field *retval = NULL;
    if(self != NULL){
        BLINK_ASSERT(self->type == BLINK_ELEM_FIELD)
        retval = (const struct blink_field *)self->ptr;
    }
    return retval;
}

static struct blink_group *castGroup(struct blink_list_element *self)
{
    struct blink_group *retval = NULL;
    if(self != NULL){
        BLINK_ASSERT(self->type == BLINK_ELEM_GROUP)
        retval = (struct blink_group *)self->ptr;
    }
    return retval;
}

static const struct blink_group *castConstGroup(const struct blink_list_element *self)
{
    const struct blink_group *retval = NULL;
    if(self != NULL){
        BLINK_ASSERT(self->type == BLINK_ELEM_GROUP)
        retval = (const struct blink_group *)self->ptr;
    }
    return retval;
}

static struct blink_namespace *castNamespace(struct blink_list_element *self)
{
    struct blink_namespace *retval = NULL;
    if(self != NULL){
        BLINK_ASSERT(self->type == BLINK_ELEM_NS)
        retval = (struct blink_namespace *)self->ptr;
    }
    return retval;
}

static struct blink_type_def *castTypeDef(struct blink_list_element *self)
{
    struct blink_type_def *retval = NULL;
    if(self != NULL){
        BLINK_ASSERT(self->type == BLINK_ELEM_TYPE)
        retval = (struct blink_type_def *)self->ptr;
    }
    return retval;
}

static const struct blink_type_def *castConstTypeDef(const struct blink_list_element *self)
{
    const struct blink_type_def *retval = NULL;
    if(self != NULL){
        BLINK_ASSERT(self->type == BLINK_ELEM_TYPE)
        retval = (const struct blink_type_def *)self->ptr;
    }
    return retval;
}

static struct blink_list_element *getTerminal(struct blink_list_element *element, bool *dynamic)
{
    BLINK_ASSERT(element != NULL)
    BLINK_ASSERT(dynamic != NULL)

    struct blink_list_element *ptr = element;
    *dynamic = false;

    while((ptr->type == BLINK_ELEM_TYPE) && (castTypeDef(ptr)->type.tag == BLINK_ITYPE_REF)){

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

    memset(iter, 0x0, sizeof(*iter));
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
