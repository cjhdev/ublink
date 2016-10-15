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

/* enums **************************************************************/

enum definition_type {
    GROUP,
    ENUMERATION,
    TYPE
};

/* static prototypes **************************************************/

/* try to parse an annotation and return true if either no annotation is found, or annotation is found with correct encoding */
static bool parseAnnote(struct blink_schema *self, const char *in, size_t inLen, size_t *read, const char **key, size_t *keyLen, const char **value, size_t *valueLen);

/* return true if name is unique in namespace */
static bool nameIsUnique(const struct blink_namespace *ns, const char *name, size_t nameLen);

/* return a pointer to a definition structure for a given name
 *
 * @param[in] self schema
 * @param[in] cname name to lookup
 * @param[in] cnameLen byte length of `cname`
 * @param[in] recursive mode will search again (and again) until the result is not a reference
 * @param[out] the type of definition structure the returned pointer points to
 *
 * @return void *
 *
 * @retval NULL (cname does not resolve to a definition)
 * 
 * */
static void *resolve(const struct blink_schema *self, const char *cname, size_t cnameLen, bool recursive, enum definition_type *type);

static bool resolveType(const struct blink_schema *self, struct blink_type *type);
static bool resolveGroup(const struct blink_schema *self, struct blink_group *group);

static struct blink_schema *parse(struct blink_schema *self, const char *in, size_t inLen);
static struct blink_enum *parseEnum(struct blink_schema *self, struct blink_namespace *ns, const char *in, size_t inLen, size_t *read);
static struct blink_type *parseType(const char *in, size_t inLen, size_t *read, struct blink_type *type);

static struct blink_namespace *newNamespace(struct blink_schema *self, const char *name, size_t nameLen);
static struct blink_group *newGroup(struct blink_schema *self, struct blink_namespace *ns);
static struct blink_field *newField(struct blink_schema *self, struct blink_group *group);
static struct blink_enum *newEnum(struct blink_schema *self, struct blink_namespace *ns);
static struct blink_symbol *newSymbol(struct blink_schema *self, struct blink_enum *e);
static struct blink_type_def *newTypeDef(struct blink_schema *self, struct blink_namespace *ns);

const struct blink_enum * getEnumByName(const struct blink_schema *self, const char *name, size_t nameLen);
const struct blink_enum * getDefinitionByName(const struct blink_schema *self, const char *name, size_t nameLen);

/* split a cname into namespace and name components */
static void splitCName(const char *in, size_t inLen, const char **nsName, size_t *nsNameLen, const char **name, size_t *nameLen);

/* functions **********************************************************/

struct blink_schema *BLINK_NewSchema(struct blink_schema *self, fn_blink_calloc_t calloc, fn_blink_free_t free)
{
    ASSERT(self != NULL)
    memset(self, 0x0, sizeof(*self));

    self->calloc = calloc;
    self->free = free;
    
    return self;
}

//todo: yes, memory leak
void BLINK_DestroySchema(struct blink_schema *self)
{
    ASSERT(self != NULL)    
    memset(self, 0x0, sizeof(*self));
}

const struct blink_group *BLINK_GetGroupByName(struct blink_schema *self, const char *qName, size_t qNameLen)
{
    ASSERT(self != NULL)
    ASSERT(qName != NULL)
    
    const struct blink_namespace *ns = self->ns;
    const struct blink_group *ptr = NULL;
    
    const char *name;
    size_t nameLen;
    const char *nsName;
    size_t nsNameLen;

    splitCName(qName, qNameLen, &nsName, &nsNameLen, &name, &nameLen);

    while(ns != NULL){

        if((ns->nameLen == nsNameLen) && (memcmp(ns->name, nsName, nsNameLen) == 0)){

            ptr = ns->groups;

            while(ptr != NULL){

                if((ptr->nameLen == nameLen) && !memcmp(ptr->name, name, nameLen)){

                    return ptr;
                }

                ptr = ptr->next;
            }

            return NULL;
        }
        
        ns = ns->next;
    }

    return NULL;
}

const struct blink_group *BLINK_GetGroupByID(struct blink_schema *self, uint64_t id)
{
    ASSERT(self != NULL)

    const struct blink_group *retval = NULL;
    const struct blink_namespace *ns = self->ns;
    const struct blink_group *ptr;
    
    while(ns != NULL){

        ptr = ns->groups;

        while(ptr != NULL){

            if(ptr->hasID && (ptr->id == id)){

                retval = ptr;
                break;
            }
            else{

                ptr = ptr->next;
            }   
        }

        if(retval != NULL){

            break;            
        }
        else{

            ns = ns->next;
        }
    }

    return retval;
}

const struct blink_field_iterator *BLINK_NewFieldIterator(const struct blink_group *group, struct blink_field_iterator *iter)
{
    ASSERT(group != NULL)
    ASSERT(iter != NULL)

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
    ASSERT(iter != NULL)
    
    const struct blink_field *retval = NULL;

    while(retval == NULL){

        if(iter->field[iter->depth] != NULL){

            retval = iter->field[iter->depth];
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
    ASSERT(self != NULL)
    ASSERT(in != NULL)
    
    bool errors = false;
    struct blink_schema *retval = parse(self, in, inLen);
    struct blink_namespace *ns;

    if(retval != NULL){

        /* resolve all type definitions */

        ns = retval->ns;

        while(ns != NULL){

            struct blink_type_def *t = ns->types;

            while(t != NULL){

                if(!resolveType(self, &t->type)){

                    errors = true;
                }

                t = t->next;
            }

            ns = ns->next;
        }

        /* resolve all references within groups */

        if(errors == false){
        
            ns = retval->ns;

            while(ns != NULL){
            
                struct blink_group *g = ns->groups;

                while(g != NULL){

                    if(!resolveGroup(self, g)){

                        errors = true;
                    }
                    
                    g = g->next;                            
                }
                
                ns = ns->next;
            }
        }
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

const char *BLINK_GetGroupName(const struct blink_group *group, size_t *nameLen)
{
    ASSERT(group != NULL)
    ASSERT(nameLen != NULL)

    *nameLen = group->nameLen;
    return group->name;
}

const struct blink_group *BLINK_GetSuperGroup(const struct blink_group *group)
{
    ASSERT(group != NULL)
    
    return group->s;
}

const char *BLINK_GetFieldName(const struct blink_field *field, size_t *nameLen)
{
    ASSERT(field != NULL)
    ASSERT(nameLen != NULL)

    *nameLen = field->nameLen;
    return field->name;
}

bool BLINK_FieldIsOptional(const struct blink_field *field)
{
    ASSERT(field != NULL)

    return field->isOptional;
}

enum blink_type_tag BLINK_GetFieldType(const struct blink_field *field)
{
    ASSERT(field != NULL)

    return field->type.tag;
}

uint32_t BLINK_GetFieldSize(const struct blink_field *field)
{
    ASSERT(field != NULL)

    return field->type.size;
}

const char *BLINK_GetFieldRef(const struct blink_field *field, size_t *refLen)
{
    ASSERT(field != NULL)
    ASSERT(refLen != NULL)

    *refLen = field->nameLen;
    return field->name;
}

const struct blink_symbol *BLINK_GetSymbolValue(const struct blink_enum *e, const char *name, size_t nameLen, int32_t *value)
{
    const struct blink_symbol *retval = NULL;
    const struct blink_symbol *ptr = e->s;
    
    while(ptr != NULL){

        if((ptr->nameLen == nameLen) && (memcmp(ptr->name, name, nameLen) == 0)){

            *value = ptr->value;
            retval = ptr;
            break;
        }

        ptr = ptr->next;
    }

    return retval;
}

const struct blink_symbol *BLINK_GetSymbolName(const struct blink_enum *e, int32_t value, const char **name, size_t *nameLen)
{
    const struct blink_symbol *retval = NULL;
    const struct blink_symbol *ptr = e->s;
    
    while(ptr != NULL){

        if(ptr->value == value){

            *name = ptr->name;
            *nameLen = ptr->nameLen;
            retval = ptr;
            break;
        }

        ptr = ptr->next;
    }

    return retval;
}

/* static functions ***************************************************/

static bool nameIsUnique(const struct blink_namespace *ns, const char *name, size_t nameLen)
{
    const struct blink_type_def *t = ns->types;
    const struct blink_enum *e = ns->enums;
    const struct blink_group *g = ns->groups;
    bool errors = false;
    
    while(!errors && (t != NULL)){

        if((t->nameLen == nameLen) && (memcmp(t->name, name, nameLen) == 0)){
            
            errors = true;
        }

        t = t->next;
    }

    while(!errors && (e != NULL)){

        if((e->nameLen == nameLen) && (memcmp(e->name, name, nameLen) == 0)){
            
            errors = true;
        }

        e = e->next;
    }

    while(!errors && (g != NULL)){

        if((g->nameLen == nameLen) && (memcmp(g->name, name, nameLen) == 0)){
            
            errors = true;
        }

        g = g->next;
    }

    return (errors ? false : true);
}

static void *resolve(const struct blink_schema *self, const char *cname, size_t cnameLen, bool recursive, enum definition_type *type)
{
    struct blink_namespace *ns = self->ns;

    void *retval = NULL;

    const char *ptr = cname;
    size_t ptrLen = cnameLen;

    const char *nsName;
    size_t nsNameLen;

    const char *name;
    size_t nameLen;

    struct blink_type_def *defStack[BLINK_LINK_DEPTH];
    uint8_t depth;
    uint8_t i;
    bool loopAgain;
    
    do{

        loopAgain = false;

        splitCName(ptr, ptrLen, &nsName, &nsNameLen, &name, &nameLen);

        while((retval == NULL) && (ns != NULL)){

            if((ns->nameLen == nsNameLen) && (memcmp(ns->name, nsName, nsNameLen) == 0)){

                struct blink_group *g = ns->groups;
                
                while(g != NULL){

                    if((g->nameLen == nameLen) && (memcmp(g->name, name, nameLen) == 0)){

                        *type = GROUP;
                        retval = (void *)g;
                        break;
                    }

                    g = g->next;
                }

                if(retval == NULL){

                    struct blink_enum *e = ns->enums;

                    while(e != NULL){

                        if((e->nameLen == nameLen) && (memcmp(e->name, name, nameLen) == 0)){

                            *type = ENUMERATION;
                            retval = (void *)e;                    
                            break;
                        }

                        e = e->next;
                    }
                }

                if(retval == NULL){

                    struct blink_type_def *t = ns->types;

                    while(t != NULL){

                        if((t->nameLen == nameLen) && (memcmp(t->name, name, nameLen) == 0)){

                            *type = TYPE;
                            
                            if(recursive){

                                for(i=0; i < depth; i++){

                                    if(defStack[i] == t){

                                        ERROR("error: circular reference")
                                    }
                                }

                                if(i == depth){

                                    if((depth+1) == (sizeof(defStack)/sizeof(*defStack))){

                                        ERROR("reference depth limit")
                                    }
                                    else{

                                        defStack[depth] = t;
                                        depth++;

                                        ptr = t->name;
                                        ptrLen = t->nameLen;
                                        
                                        loopAgain = true;                                        
                                    }
                                }
                            }
                            else{

                                retval = (void *)t;
                            }

                            break;
                        }

                        t = t->next;
                    }
                }

                break;
            }
            
            ns = ns->next;            
        }
    }
    while(loopAgain);

    return retval;
}

static bool resolveType(const struct blink_schema *self, struct blink_type *type)
{
    ASSERT(self != NULL)
    ASSERT(type != NULL)

    bool retval = false;
    enum definition_type typeType;

    if((type->tag == TYPE_REF) || (type->tag == TYPE_DYNAMIC_REF)){

        type->resolvedRef = resolve(self, type->ref, type->refLen, false, &typeType);

        if(type->resolvedRef == NULL){            

            ERROR("reference does not resolve")
        }
        else{

            retval = true;

            switch(typeType){
            case GROUP:
                type->tag = (type->tag == TYPE_REF) ? TYPE_STATIC_GROUP : TYPE_DYNAMIC_REF;
                break;
            case ENUMERATION:
                type->tag = TYPE_ENUM;
                break;
            case TYPE:                
            default:
                break;            
            }            
        }        
    }
    else{

        retval = true;
    }

    return retval;
}

static bool resolveGroup(const struct blink_schema *self, struct blink_group *group)
{
    ASSERT(self != NULL)
    ASSERT(group != NULL)

    bool errors = false;
    enum definition_type typeType;
    struct blink_field *f = group->f;

    /* resolve supergroup reference */
    if(group->superGroupLen > 0){

        group->s = (const struct blink_group *)resolve(self, group->superGroup, group->superGroupLen, true, &typeType);

        if(group->s == NULL){

            ERROR("supergroup does not resolve")
            errors = true;
        }
        else{

            if(typeType != GROUP){

                ERROR("supergroup must reference a group")
                group->s = NULL;
            }
        }        
    }

    while(f != NULL){

        switch(f->type.tag){
        case TYPE_REF:
        case TYPE_DYNAMIC_REF:        
        default:
            break;
        }
        
        f = f->next;
    }
    
    return (errors ? false : true);
}

static struct blink_schema *parse(struct blink_schema *self, const char *in, size_t inLen)
{
    ASSERT(self != NULL)
    ASSERT(in != NULL)
    
    size_t pos = 0U;
    struct blink_schema *retval = NULL;
    size_t read;

    struct blink_namespace *ns;
    struct blink_group *g;
    union blink_token_value value;
    enum blink_token tok;
    enum blink_token tokn;

    if(BLINK_GetToken(in, inLen, &read, &value, NULL) == TOK_NAMESPACE){

        pos += read;

        if(BLINK_GetToken(&in[pos], inLen - pos, &read, &value, NULL) == TOK_NAME){
        
            pos += read;
            ns = newNamespace(self, value.literal.ptr, value.literal.len);            
        }
        else{

            ERROR("expecting name")
            return retval;          
        }
    }
    else{

        ns = newNamespace(self, NULL, 0U);
    }

    if(ns != NULL){

        /* parse all definitions */
        while(BLINK_GetToken(&in[pos], inLen-pos, &read, &value, NULL) != TOK_EOF){

            const char *name = NULL;
            size_t nameLen = 0U;

            if(BLINK_GetToken(&in[pos], inLen-pos, &read, &value, NULL) == TOK_NAME){

                pos += read;

                name = value.literal.ptr;
                nameLen = value.literal.len;
                
                uint64_t id = 0U;
                bool hasID = false;
                
                if(BLINK_GetToken(&in[pos], inLen - pos, &read, &value, NULL) == TOK_SLASH){

                    pos += read;

                    if(BLINK_GetToken(&in[pos], inLen - pos, &read, &value, NULL) == TOK_NUMBER){

                        pos += read;
                        id = value.number;
                        hasID = true;
                    }
                    else{
                                            
                        ERROR("error: expecting integer or hexnum")
                        return retval;
                    }                
                }

                /* type or enum */
                if(!hasID && BLINK_GetToken(&in[pos], inLen - pos, &read, &value, NULL) == TOK_EQUAL){

                    pos += read;

                    if(!nameIsUnique(ns, name, nameLen)){

                        ERROR("error: duplicate name")
                        return retval;
                    }

                    tok = BLINK_GetToken(&in[pos], inLen - pos, &read, &value, NULL);

                    if(tok == TOK_BAR){

                        if(parseEnum(self, ns, &in[pos], inLen - pos, &read) == NULL){

                            return retval;
                        }
                        else{

                            pos += read;
                        }
                    }
                    else{

                        tokn = BLINK_GetToken(&in[pos+read], inLen - (pos + read), &read, &value, NULL);

                        if((tok == TOK_NAME) && ((tokn == TOK_SLASH) || (tok == TOK_BAR))){
    
                            if(parseEnum(self, ns, &in[pos], inLen - pos, &read) == NULL){

                                return retval;
                            }
                            else{

                                pos += read;
                            }   
                        }
                        else{

                            struct blink_type_def *t = newTypeDef(self, ns);

                            t->name = name;
                            t->nameLen = nameLen;

                            if(t != NULL){

                                if(parseType(&in[pos], inLen - pos, &read, &t->type) == NULL){
                                    
                                    return retval;
                                }
                            }
                            else{

                                ERROR("malloc")
                                return retval;
                            }
                        }
                    }                    
                }
                else if(!hasID && BLINK_GetToken(&in[pos], inLen - pos, &read, &value, NULL) == TOK_LARROW){

                    ERROR("todo: inline annote")
                    return retval;
                }
                else{

                    if(!nameIsUnique(ns, name, nameLen)){

                        ERROR("error: duplicate name")
                        return retval;
                    }

                    g = newGroup(self, ns);

                    if(g != NULL){

                        g->name = name;
                        g->nameLen = nameLen;
                        g->id = id;
                        g->hasID = hasID;

                        if(BLINK_GetToken(&in[pos], inLen - pos, &read, &value, NULL) == TOK_COLON){

                            pos += read;
                            
                            if((BLINK_GetToken(&in[pos], inLen - pos, &read, &value, NULL) == TOK_CNAME) || (BLINK_GetToken(&in[pos], inLen - pos, &read, &value, NULL) == TOK_NAME)){

                                pos += read;

                                g->superGroup = value.literal.ptr;
                                g->superGroupLen = value.literal.len;
                            }
                            else{

                                ERROR("error: expecting super class name (qname)")
                                return retval;
                            }
                        }

                        if(BLINK_GetToken(&in[pos], inLen - pos, &read, &value, NULL) == TOK_RARROW){

                            pos += read;

                            do{

                                struct blink_field *f = newField(self, g);

                                if(parseType(&in[pos], inLen - pos, &read, &f->type) == &f->type){

                                    pos += read;
                                    
                                    if(BLINK_GetToken(&in[pos], inLen - pos, &read, &value, NULL) == TOK_NAME){

                                        pos += read;

                                        f->name = value.literal.ptr;
                                        f->nameLen = value.literal.len;

                                        if(BLINK_GetToken(&in[pos], inLen - pos, &read, &value, NULL) == TOK_QUESTION){
                                            
                                            pos += read;
                                            f->isOptional = true;
                                        }
                                        else{

                                            f->isOptional = false;
                                        }
                                    }
                                    else{

                                        ERROR("expecting field name")
                                        return retval;
                                    }    
                                }
                                else{

                                    return retval;
                                }                                
                            }
                            while(BLINK_GetToken(&in[pos], inLen - pos, &read, &value, NULL) == TOK_COMMA);                   
                        }                    
                    }
                    else{

                        ERROR("malloc()")
                        return retval;
                    }
                }
            }
            else{

                /* null terminated input */
                if(in[pos + read] == '\0'){

                    retval = self;
                }
                else{

                    ERROR("unknown character %u", in[pos + read])
                    return retval;
                }
                break;
            }            
        }

        retval = self;    
    }
    else{

        ERROR("malloc()")
    }

    return retval; 
}

static struct blink_type *parseType(const char *in, size_t inLen, size_t *read, struct blink_type *type)
{
    ASSERT(in != NULL)
    ASSERT(read != NULL)
    ASSERT(type != NULL)

    size_t pos = 0U;
    size_t r;
    union blink_token_value value;
    enum blink_token tok = BLINK_GetToken(&in[pos], inLen - pos, &r, &value, NULL);
    struct blink_type *retval = NULL;

    enum blink_token tokenToType[] = {
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

        ASSERT(type->tag < (sizeof(tokenToType)/sizeof(*tokenToType)))
        type->tag = tokenToType[tok];

        pos += r;

        if((tok == TOK_STRING) || (tok == TOK_BINARY) || (tok == TOK_FIXED)){

            if(BLINK_GetToken(&in[pos], inLen - pos, &r, &value, NULL) == TOK_LPAREN){

                pos += r;

                if(BLINK_GetToken(&in[pos], inLen - pos, &r, &value, NULL) == TOK_NUMBER){

                    pos += r;
                }
                else{

                    ERROR("expecting a size");
                    return retval;
                }

                if(value.number > 0xffffffffU){

                    ERROR("size decode but is out of range")
                    return retval;
                }

                type->size = (uint32_t)value.number;

                if(BLINK_GetToken(&in[pos], inLen - pos, &r, &value, NULL) == TOK_RPAREN){

                    pos += r;
                }   
                else{

                    ERROR("expecting a ')'")
                    return retval;
                }
            }
            else if(tok == TOK_FIXED){

                ERROR("expecting a '('")
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

        ERROR("expecting a type");
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

            ERROR("expecting ']' character")
            return retval;
        }                        
    }
    else{

        type->isSequence = false;
    }

    *read = pos;

    return type;
}

static struct blink_enum *parseEnum(struct blink_schema *self, struct blink_namespace *ns, const char *in, size_t inLen, size_t *read)
{
    ASSERT(self != NULL)
    ASSERT(ns != NULL)
    ASSERT(in != NULL)
    ASSERT(read != NULL)
    
    size_t pos = 0U;
    size_t r;
    union blink_token_value value;
    struct blink_symbol *s;
    struct blink_enum *retval = NULL;
    struct blink_enum *e = newEnum(self, ns);
    bool single = false;

    if(e != NULL){

        if(BLINK_GetToken(&in[pos], inLen - pos, &r, &value, NULL) == TOK_BAR){

            pos += r;            
            single = true;
        }

        r = 0U;

        do{

            pos += r;
        
            if(BLINK_GetToken(&in[pos], inLen - pos, &r, &value, NULL) == TOK_NAME){

                pos += r;

                s = newSymbol(self, e);
                
                if(s != NULL){

                    s->name = value.literal.ptr;
                    s->nameLen = value.literal.len;
                    
                    if(BLINK_GetToken(&in[pos], inLen - pos, &r, &value, NULL) == TOK_SLASH){

                        pos += r;

                        if(BLINK_GetToken(&in[pos], inLen - pos, &r, &value, NULL) == TOK_NUMBER){

                            pos += r;
                            
                            s->value = value.number;
                            s->implicitID = false;
                        }
                        else{

                            ERROR("expecting number")
                            return retval;
                        }
                    }
                    else{

                        s->implicitID = true;
                    }
                }
                else{

                    ERROR("malloc")
                    return retval;
                }                        
            }
            else{

                ERROR("error: expecting name")
                return retval;
            }
        }
        while(!single && (BLINK_GetToken(&in[pos], inLen - pos, &r, &value, NULL) == TOK_BAR));

        retval = e;
        *read = pos;
    }

    return retval;
}

static bool parseAnnote(struct blink_schema *self, const char *in, size_t inLen, size_t *read, const char **key, size_t *keyLen, const char **value, size_t *valueLen)
{
    ASSERT(self != NULL)
    ASSERT(read != NULL)
    ASSERT(key != NULL)
    ASSERT(keyLen != NULL)
    ASSERT(value != NULL)
    ASSERT(valueLen != NULL)

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

            ASSERT(key != NULL)
            
            break;
            
        default:
            ERROR("unexpected token")
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

                ERROR("expecting literal")
            }            
        }
        else{

            ERROR("expecting '='")            
        }
    }

    return retval;
}

static struct blink_namespace *newNamespace(struct blink_schema *self, const char *name, size_t nameLen)
{
    ASSERT(self != NULL)
    ASSERT((name != NULL) || (nameLen == 0U))

    struct blink_namespace *retval = NULL;
    struct blink_namespace *ptr;    
    
    retval = self->ns;
    while(retval != NULL){

        if((retval->nameLen == nameLen) && !memcmp(retval->name, name, nameLen)){

            break;
        }    
    }

    if(retval == NULL){
        
        retval = self->calloc(1, sizeof(struct blink_namespace));
        
        retval->name = name;
        retval->nameLen = nameLen;

        ptr = self->ns;

        if(ptr == NULL){

            self->ns = retval;
        }
        else{

            while(ptr->next != NULL){

                ptr = ptr->next;
            }

            ptr->next = retval;
        }    
    }

    return retval;
}

static struct blink_group *newGroup(struct blink_schema *self, struct blink_namespace *ns)
{
    ASSERT(self != NULL)
    ASSERT(ns != NULL)
    
    struct blink_group *retval;
    struct blink_group *ptr = ns->groups;

    retval = self->calloc(1, sizeof(struct blink_group));

    if(retval != NULL){

        if(ptr == NULL){

            ns->groups = retval;
        }
        else{

            while(ptr->next != NULL){

                ptr = ptr->next;
            }

            ptr->next = retval;
        }       
    }

    return retval;
}

static struct blink_field *newField(struct blink_schema *self, struct blink_group *group)
{
    ASSERT(self != NULL)
    ASSERT(group != NULL)

    struct blink_field *ptr = group->f;
    struct blink_field *retval;

    retval = self->calloc(1, sizeof(struct blink_field));
    
    if(retval != NULL){
    
        if(ptr == NULL){

            group->f = retval;
        }
        else{

            while(ptr->next != NULL){

                ptr = ptr->next;
            }

            ptr->next = retval;
        }
    }

    return retval;
}

static struct blink_enum *newEnum(struct blink_schema *self, struct blink_namespace *ns)
{
    ASSERT(self != NULL)
    ASSERT(ns != NULL)
    
    struct blink_enum *retval;
    struct blink_enum *ptr = ns->enums;

    retval = self->calloc(1, sizeof(struct blink_enum));

    if(retval != NULL){

        if(ptr == NULL){

            ns->enums = retval;
        }
        else{

            while(ptr->next != NULL){

                ptr = ptr->next;
            }

            ptr->next = retval;
        }       
    }

    return retval;
}

static struct blink_symbol *newSymbol(struct blink_schema *self, struct blink_enum *e)
{
    ASSERT(self != NULL)
    ASSERT(e != NULL)

    struct blink_symbol *ptr = e->s;
    struct blink_symbol *retval;

    retval = self->calloc(1, sizeof(struct blink_enum));
    
    if(retval != NULL){
    
        if(ptr == NULL){

            e->s = retval;
        }
        else{

            while(ptr->next != NULL){

                ptr = ptr->next;
            }

            ptr->next = retval;
        }
    }

    return retval;
}

static struct blink_type_def *newTypeDef(struct blink_schema *self, struct blink_namespace *ns)
{
    ASSERT(self != NULL)
    ASSERT(ns != NULL)
    
    struct blink_type_def *retval;
    struct blink_type_def *ptr = ns->types;

    retval = self->calloc(1, sizeof(struct blink_type_def));

    if(retval != NULL){

        if(ptr == NULL){

            ns->types = retval;
        }
        else{

            while(ptr->next != NULL){

                ptr = ptr->next;
            }

            ptr->next = retval;
        }       
    }

    return retval;
}

static void splitCName(const char *in, size_t inLen, const char **nsName, size_t *nsNameLen, const char **name, size_t *nameLen)
{
    ASSERT(in != NULL)
    ASSERT(nsName != NULL)
    ASSERT(nsNameLen != NULL)
    ASSERT(name != NULL)
    ASSERT(nameLen != NULL)
    
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

