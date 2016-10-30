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

enum definition_type {
    GROUP,
    ENUMERATION,
    TYPE
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
    const struct blink_group *s;    /**< pointer to supergroup definition */
    struct blink_list_element *f;          /**< fields belonging to group */
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
    struct blink_inline_annote *next;
    const char *name;
    size_t nameLen;
    struct blink_annote *a;         /** list of annotes to apply */
};

/** namespace */
struct blink_namespace {    
    const char *name;   /**< name of this namespace */
    size_t nameLen;     /**< byte length of `name` */  
    /** list of groups, enums, and types in this namespace */
    struct blink_list_element *defs;      
};

struct blink_list_element {
    struct blink_list_element *next;
    enum blink_list_type {
        BLINK_ELEM_NULL,
        BLINK_ELEM_NS,
        BLINK_ELEM_GROUP,
        BLINK_ELEM_FIELD,
        BLINK_ELEM_ENUM,
        BLINK_ELEM_SYMBOL,
        BLINK_ELEM_TYPE,
        BLINK_ELEM_ANNOTE,
    } type;
    void *ptr;    
};

/* static prototypes **************************************************/

static struct blink_schema *parse(struct blink_schema *self, const char *in, size_t inLen);
static struct blink_type *parseType(const char *in, size_t inLen, size_t *read, struct blink_type *type);
static bool parseAnnote(struct blink_schema *self, const char *in, size_t inLen, size_t *read, const char **key, size_t *keyLen, const char **value, size_t *valueLen);

static void *resolve(const struct blink_schema *self, const char *cname, size_t cnameLen, bool recursive, enum definition_type *type);
static bool resolveType(const struct blink_schema *self, struct blink_type *type);
static bool resolveGroup(const struct blink_schema *self, struct blink_group *group);

static struct blink_list_element *newListElement(struct blink_schema *self, struct blink_list_element **head, enum blink_list_type type);
static struct blink_list_element *searchListByName(struct blink_list_element *head, const char *name, size_t nameLen);

static void splitCName(const char *in, size_t inLen, const char **nsName, size_t *nsNameLen, const char **name, size_t *nameLen);

static struct blink_enum *castEnum(struct blink_list_element *self);
static struct blink_symbol *castSymbol(struct blink_list_element *self);
static struct blink_field *castField(struct blink_list_element *self);
static struct blink_group *castGroup(struct blink_list_element *self);
static struct blink_namespace *castNamespace(struct blink_list_element *self);
static struct blink_type_def *castTypeDef(struct blink_list_element *self);

/* functions **********************************************************/

struct blink_schema *BLINK_NewSchema(struct blink_schema *self, fn_blink_calloc_t calloc, fn_blink_free_t free)
{
    BLINK_ASSERT(self != NULL)
    BLINK_ASSERT(calloc != NULL)

    memset(self, 0x0, sizeof(*self));

    self->calloc = calloc;
    self->free = free;
    
    return self;
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
    struct blink_list_element *ptr;
    struct blink_list_element *ns = self->ns;
    
    while((retval == NULL) && (ns != NULL)){

        ptr = castNamespace(ns)->defs;

        while(ptr != NULL){

            if((ptr->type == BLINK_ELEM_GROUP) && castGroup(ptr)->hasID && (castGroup(ptr)->id == id)){

                retval = (const struct blink_group *)castGroup(ptr);
                break;                
            }
            else{
                
                ptr = ptr->next;            
            }
        }

        ns = ns->next;        
    }

    return retval;
}

const struct blink_field_iterator *BLINK_NewFieldIterator(const struct blink_group *group, struct blink_field_iterator *iter)
{
    BLINK_ASSERT(group != NULL)
    BLINK_ASSERT(iter != NULL)

    const struct blink_group *ptr = group;
    const struct blink_field_iterator *retval = NULL;
    
    memset(iter, 0, sizeof(*iter));

    for(iter->depth=0U; iter->depth < BLINK_INHERIT_DEPTH; iter->depth++){

        iter->field[iter->depth] = ptr->f;
        
        if(ptr->s == NULL){

            break;            
        }
        else{

            ptr = ptr->s;
        }
    }

    if(iter->depth < BLINK_INHERIT_DEPTH){

        retval = iter;
    }

    return retval;
}

const struct blink_field *BLINK_NextField(struct blink_field_iterator *iter)
{
    BLINK_ASSERT(iter != NULL)
    
    const struct blink_field *retval = NULL;

    while(retval == NULL){

        if(iter->field[iter->depth] != NULL){

            retval = (const struct blink_field *)castField(iter->field[iter->depth]);
            iter->field[iter->depth] = iter->field[iter->depth]->next;
        }
        else if(iter->depth > 0U){

            iter->depth--;
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
    
    bool errors = false;
    
    /* parse the syntax */
    struct blink_schema *retval = parse(self, in, inLen);
    
    if(retval != NULL){

        /* resolve all type definitions */

        /* resolve all references within groups */
        
    }
    else{

        errors = true;
    }

    if(errors){

        BLINK_DestroySchema(self);
        retval = NULL;        
    }
    else{

        self->finalised = true;
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

const struct blink_group *BLINK_GetSuperGroup(const struct blink_group *self)
{
    BLINK_ASSERT(self != NULL)
    
    return self->s;
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

    return self->type.tag;
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

        retval = (const struct blink_symbol *)element->ptr;
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
    struct blink_symbol *retval = NULL;

    while(ptr != NULL){

        if(((const struct blink_symbol *)ptr->ptr)->value == value){

            retval = (const struct blink_symbol *)ptr->ptr;
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

                    BLINK_ERROR("duplicate definition");
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

                    do{

                        if(BLINK_GetToken(&in[pos], inLen - pos, &read, &value, NULL) != TOK_NAME){

                            BLINK_ERROR("error: expecting name")
                            return retval;
                        }

                        pos += read;

                        struct blink_symbol s;

                        s.name = value.literal.ptr;
                        s.nameLen = value.literal.len;
                            
                        if(BLINK_GetToken(&in[pos], inLen - pos, &read, &value, NULL) == TOK_SLASH){

                            pos += read;

                            if(BLINK_GetToken(&in[pos], inLen - pos, &read, &value, NULL) != TOK_NUMBER){
                                
                                BLINK_ERROR("expecting number")
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
                            
                            BLINK_ERROR("error: duplicate")
                            return retval;
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
                
                    BLINK_ERROR("duplicate name")
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

                        BLINK_ERROR("error: expecting super class name (qname)")
                        return retval;
                    }

                    pos += read;

                    g->superGroup = value.literal.ptr;
                    g->superGroupLen = value.literal.len;
                }

                if(BLINK_GetToken(&in[pos], inLen - pos, &read, &value, NULL) == TOK_RARROW){

                    pos += read;

                    do{

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

        if(BLINK_GetToken(&in[pos], inLen - pos, &r, &value, NULL) == TOK_STAR){

            pos += r;
            type->tag = TYPE_DYNAMIC_REF;
        }
        else{

            type->tag = TYPE_REF;
        }
        break;

    case TOK_EQUAL:
    case TOK_COMMA:
    case TOK_PERIOD:
    case TOK_QUESTION:
    case TOK_LBRACKET:
    case TOK_RBRACKET:
    case TOK_LPAREN:
    case TOK_RPAREN:
    case TOK_STAR:
    case TOK_BAR:
    case TOK_SLASH:
    case TOK_AT:
    case TOK_COLON:
    case TOK_RARROW:
    case TOK_LARROW:
    case TOK_NAMESPACE:
    case TOK_SCHEMA:
    case TOK_TYPE:
    case TOK_NUMBER:
    case TOK_UNKNOWN:
    case TOK_EOF:
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

static struct blink_field *castField(struct blink_list_element *self)
{
    struct blink_field *retval = NULL;
    if(self != NULL){
        BLINK_ASSERT(self->type == BLINK_ELEM_FIELD)
        retval = (struct blink_field *)self->ptr;
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
