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

#include <malloc.h>
#include <string.h>

#include "blink_debug.h"
#include "blink_parser.h"
#include "blink_lexer.h"

/* static prototypes **************************************************/

static struct blink_schema *parse(struct blink_schema *ctxt, const char *in, size_t inLen);
static struct blink_enum *parseEnum(struct blink_schema *ctxt, struct blink_namespace *ns, const char *in, size_t inLen, size_t *read);
static void splitCName(const char *in, size_t inLen, const char **nsName, size_t *nsNameLen, const char **name, size_t *nameLen);
static struct blink_type *parseType(const char *in, size_t inLen, size_t *read, struct blink_type *type);

static struct blink_namespace *newNamespace(struct blink_schema *ctxt, const char *name, size_t nameLen);
static struct blink_group *newGroup(struct blink_schema *ctxt, struct blink_namespace *ns);
static struct blink_field *newField(struct blink_schema *ctxt, struct blink_group *group);
static struct blink_enum *newEnum(struct blink_schema *ctxt, struct blink_namespace *ns);
static struct blink_symbol *newSymbol(struct blink_schema *ctxt, struct blink_enum *e);
static struct blink_type_def *newTypeDef(struct blink_schema *ctxt, struct blink_namespace *ns);

/* functions **********************************************************/

struct blink_schema *BLINK_NewSchema(struct blink_schema *ctxt)
{
    ASSERT(ctxt != NULL)
    memset(ctxt, 0x0, sizeof(*ctxt));
    return ctxt;
}

void BLINK_DestroySchema(struct blink_schema *ctxt)
{
    ASSERT(ctxt != NULL)
    memset(ctxt, 0x0, sizeof(*ctxt));
}

const struct blink_group *BLINK_GetGroupByName(struct blink_schema *ctxt, const char *qName, size_t qNameLen)
{
    ASSERT(ctxt != NULL)
    ASSERT(qName != NULL)
    
    const struct blink_namespace *ns = ctxt->ns;
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

const struct blink_group *BLINK_GetGroupByID(struct blink_schema *ctxt, uint64_t id)
{
    ASSERT(ctxt != NULL)

    const struct blink_group *retval = NULL;
    const struct blink_namespace *ns = ctxt->ns;
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

struct blink_schema *BLINK_Parse(struct blink_schema *ctxt, const char *in, size_t inLen)
{
    ASSERT(ctxt != NULL)
    ASSERT(in != NULL)
    
    bool errors = false;
    struct blink_schema *retval = parse(ctxt, in, inLen);

    if(retval != NULL){

        struct blink_namespace *ns = retval->ns;

        while(ns != NULL){

            struct blink_group *g = ns->groups;

            while(g != NULL){

                if(g->superGroupLen > 0U){

                    g->s = BLINK_GetGroupByName(ctxt, g->superGroup, g->superGroupLen);

                    if(g->s == NULL){

                        ERROR("supergroup is undefined")
                        errors = true;
                    }
                }

                g = g->next;
            }

            ns = ns->next;
        }
    }
    else{

        errors = true;
    }

    if(errors){

        BLINK_DestroySchema(ctxt);
        retval = NULL;        
    }
    else{

        ctxt->finalised = true;
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

/* static functions ***************************************************/

static struct blink_schema *parse(struct blink_schema *ctxt, const char *in, size_t inLen)
{
    ASSERT(ctxt != NULL)
    ASSERT(in != NULL)
    
    size_t pos = 0U;
    struct blink_schema *retval = NULL;
    size_t read;

    struct blink_namespace *ns;
    struct blink_group *g;
    union blink_token_value value;
    enum blink_token tok;
    enum blink_token tokn;

    if(BLINK_GetToken(in, inLen, &read, &value) == TOK_NAMESPACE){

        pos += read;

        if(BLINK_GetToken(&in[pos], inLen - pos, &read, &value) == TOK_NAME){
        
            pos += read;
            ns = newNamespace(ctxt, value.literal.ptr, value.literal.len);            
        }
        else{

            ERROR("expecting name")
            return retval;          
        }
    }
    else{

        ns = newNamespace(ctxt, NULL, 0U);
    }

    if(ns != NULL){

        /* parse all definitions */
        while(BLINK_GetToken(&in[pos], inLen-pos, &read, &value) != TOK_EOF){

            const char *name = NULL;
            size_t nameLen = 0U;

            if(BLINK_GetToken(&in[pos], inLen-pos, &read, &value) == TOK_NAME){

                pos += read;

                name = value.literal.ptr;
                nameLen = value.literal.len;
                
                uint64_t id = 0U;
                bool hasID = false;
                
                if(BLINK_GetToken(&in[pos], inLen - pos, &read, &value) == TOK_SLASH){

                    pos += read;

                    if(BLINK_GetToken(&in[pos], inLen - pos, &read, &value) == TOK_NUMBER){

                        pos += read;
                        id = value.number;
                        hasID = true;
                    }
                    else{
                                            
                        ERROR("error: expecting integer or hexnum")
                        return retval;
                    }                
                }
                
                if(BLINK_GetToken(&in[pos], inLen - pos, &read, &value) == TOK_EQUAL){

                    pos += read;

                    tok = BLINK_GetToken(&in[pos], inLen - pos, &read, &value);

                    if(tok == TOK_BAR){

                        if(parseEnum(ctxt, ns, &in[pos], inLen - pos, &read) == NULL){

                            return retval;
                        }
                        else{

                            pos += read;
                        }
                    }
                    else{

                        tokn = BLINK_GetToken(&in[pos+read], inLen - (pos + read), &read, &value);

                        if((tok == TOK_NAME) && ((tokn == TOK_SLASH) || (tok == TOK_BAR))){
    
                            if(parseEnum(ctxt, ns, &in[pos], inLen - pos, &read) == NULL){

                                return retval;
                            }
                            else{

                                pos += read;
                            }   
                        }
                        else{

                            struct blink_type_def *t = newTypeDef(ctxt, ns);

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
                else{

                    g = newGroup(ctxt, ns);

                    if(g != NULL){

                        g->name = name;
                        g->nameLen = nameLen;
                        g->id = id;
                        g->hasID = hasID;

                        if(BLINK_GetToken(&in[pos], inLen - pos, &read, &value) == TOK_COLON){

                            pos += read;
                            
                            if((BLINK_GetToken(&in[pos], inLen - pos, &read, &value) == TOK_CNAME) || (BLINK_GetToken(&in[pos], inLen - pos, &read, &value) == TOK_NAME)){

                                pos += read;

                                g->superGroup = value.literal.ptr;
                                g->superGroupLen = value.literal.len;
                            }
                            else{

                                ERROR("error: expecting super class name (qname)")
                                return retval;
                            }
                        }

                        if(BLINK_GetToken(&in[pos], inLen - pos, &read, &value) == TOK_RARROW){

                            pos += read;

                            do{

                                struct blink_field *f = newField(ctxt, g);

                                if(parseType(&in[pos], inLen - pos, &read, &f->type) == &f->type){

                                    pos += read;
                                    
                                    if(BLINK_GetToken(&in[pos], inLen - pos, &read, &value) == TOK_NAME){

                                        pos += read;

                                        f->name = value.literal.ptr;
                                        f->nameLen = value.literal.len;

                                        if(BLINK_GetToken(&in[pos], inLen - pos, &read, &value) == TOK_QUESTION){
                                            
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
                            while(BLINK_GetToken(&in[pos], inLen - pos, &read, &value) == TOK_COMMA);                   
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

                    retval = ctxt;
                }
                else{

                    ERROR("unknown character %u", in[pos + read])
                }
                break;
            }           
        }        
    }
    else{

        ERROR("malloc()")
    }

    return retval; 
}

static struct blink_namespace *newNamespace(struct blink_schema *ctxt, const char *name, size_t nameLen)
{
    ASSERT(ctxt != NULL)
    ASSERT((name != NULL) || (nameLen == 0U))

    struct blink_namespace *retval = NULL;
    struct blink_namespace *ptr;    
    
    retval = ctxt->ns;
    while(retval != NULL){

        if((retval->nameLen == nameLen) && !memcmp(retval->name, name, nameLen)){

            break;
        }    
    }

    if(retval == NULL){

        //fixme
        retval = calloc(1, sizeof(struct blink_namespace));

        retval->name = name;
        retval->nameLen = nameLen;

        ptr = ctxt->ns;

        if(ptr == NULL){

            ctxt->ns = retval;
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

static struct blink_type *parseType(const char *in, size_t inLen, size_t *read, struct blink_type *type)
{
    ASSERT(in != NULL)
    ASSERT(read != NULL)
    ASSERT(type != NULL)

    size_t pos = 0U;
    size_t r;
    union blink_token_value value;
    enum blink_token tok = BLINK_GetToken(&in[pos], inLen - pos, &r, &value);
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

            if(BLINK_GetToken(&in[pos], inLen - pos, &r, &value) == TOK_LPAREN){

                pos += r;

                if(BLINK_GetToken(&in[pos], inLen - pos, &r, &value) == TOK_NUMBER){

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

                if(BLINK_GetToken(&in[pos], inLen - pos, &r, &value) == TOK_RPAREN){

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

        if(BLINK_GetToken(&in[pos], inLen - pos, &r, &value) == TOK_STAR){

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
    if(BLINK_GetToken(&in[pos], inLen - pos, &r, &value) == TOK_LBRACKET){

        pos += r;

        if(BLINK_GetToken(&in[pos], inLen - pos, &r, &value) == TOK_RBRACKET){
            
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

static struct blink_enum *parseEnum(struct blink_schema *ctxt, struct blink_namespace *ns, const char *in, size_t inLen, size_t *read)
{
    ASSERT(ctxt != NULL)
    ASSERT(ns != NULL)
    ASSERT(in != NULL)
    ASSERT(read != NULL)
    
    size_t pos = 0U;
    size_t r;
    union blink_token_value value;
    struct blink_symbol *s;
    struct blink_enum *retval = NULL;
    struct blink_enum *e = newEnum(ctxt, ns);
    bool single = false;

    if(e != NULL){

        if(BLINK_GetToken(&in[pos], inLen - pos, &r, &value) == TOK_BAR){

            pos += r;            
            single = true;
        }

        r = 0U;

        do{

            pos += r;
        
            if(BLINK_GetToken(&in[pos], inLen - pos, &r, &value) == TOK_NAME){

                pos += r;

                s = newSymbol(ctxt, e);
                
                if(s != NULL){

                    s->name = value.literal.ptr;
                    s->nameLen = value.literal.len;
                    
                    if(BLINK_GetToken(&in[pos], inLen - pos, &r, &value) == TOK_SLASH){

                        pos += r;

                        if(BLINK_GetToken(&in[pos], inLen - pos, &r, &value) == TOK_NUMBER){

                            pos += r;
                            
                            s->id = value.number;
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
        while(!single && (BLINK_GetToken(&in[pos], inLen - pos, &r, &value) == TOK_BAR));

        retval = e;
        *read = pos;
    }

    return retval;
}

static struct blink_group *newGroup(struct blink_schema *ctxt, struct blink_namespace *ns)
{
    ASSERT(ctxt != NULL)
    ASSERT(ns != NULL)
    
    struct blink_group *retval;
    struct blink_group *ptr = ns->groups;

    //fixme
    retval = calloc(1, sizeof(struct blink_group));

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

static struct blink_field *newField(struct blink_schema *ctxt, struct blink_group *group)
{
    ASSERT(ctxt != NULL)
    ASSERT(group != NULL)

    struct blink_field *ptr = group->f;
    struct blink_field *retval;

    //fixme
    retval = calloc(1, sizeof(struct blink_field));
    
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

static struct blink_enum *newEnum(struct blink_schema *ctxt, struct blink_namespace *ns)
{
    ASSERT(ctxt != NULL)
    ASSERT(ns != NULL)
    
    struct blink_enum *retval;
    struct blink_enum *ptr = ns->enums;

    //fixme
    retval = calloc(1, sizeof(struct blink_enum));

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

static struct blink_symbol *newSymbol(struct blink_schema *ctxt, struct blink_enum *e)
{
    ASSERT(ctxt != NULL)
    ASSERT(e != NULL)

    struct blink_symbol *ptr = e->s;
    struct blink_symbol *retval;

    //fixme
    retval = calloc(1, sizeof(struct blink_enum));
    
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

static struct blink_type_def *newTypeDef(struct blink_schema *ctxt, struct blink_namespace *ns)
{
    ASSERT(ctxt != NULL)
    ASSERT(ns != NULL)
    
    struct blink_type_def *retval;
    struct blink_type_def *ptr = ns->types;

    //fixme
    retval = calloc(1, sizeof(struct blink_type_def));

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

