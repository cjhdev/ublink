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
static void splitCName(const char *in, size_t inLen, const char **nsName, size_t *nsNameLen, const char **name, size_t *nameLen);

static struct blink_namespace *newNamespace(struct blink_schema *ctxt, const char *name, size_t nameLen);
static struct blink_group *newGroup(struct blink_schema *ctxt, struct blink_namespace *ns);
static struct blink_field *newField(struct blink_schema *ctxt, struct blink_group *group);

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

        iter->group[iter->depth] = ptr;
        
        if(group->s == NULL){

            break;            
        }
        else{

            ptr = group->s;
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

        if(iter->group[iter->depth] != NULL){

            retval = iter->group[iter->depth];
            iter->group[iter->depth] = iter->group[iter->depth]->next;
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

    return ((errors) ? NULL : retval);
}

/* static functions ***************************************************/

static struct blink_schema *parse(struct blink_schema *ctxt, const char *in, size_t inLen)
{
    ASSERT(ctxt != NULL)
    ASSERT(in != NULL)
    
    size_t pos = 0U;
    struct blink_schema *retval = NULL;
    size_t read;

    const char *name = NULL;
    size_t nameLen = 0U;

    struct blink_namespace *ns;
    struct blink_group *g;
    union blink_token_value value;

    if(BLINK_GetToken(in, inLen, &read, &value) == T_NAMESPACE){

        pos += read;

        if(BLINK_GetToken(&in[pos], inLen - pos, &read, &value) == T_NAME){
        
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
        while(BLINK_GetToken(&in[pos], inLen-pos, &read, &value) != T_EOF){

            const char *name = NULL;
            size_t nameLen = 0U;

            if(BLINK_GetToken(&in[pos], inLen-pos, &read, &value) == T_NAME){

                pos += read;

                name = value.literal.ptr;
                nameLen = value.literal.len;
                
                uint64_t id = 0U;
                bool hasID = false;
                
                if(BLINK_GetToken(&in[pos], inLen - pos, &read, &value) == T_SLASH){

                    pos += read;

                    if(BLINK_GetToken(&in[pos], inLen - pos, &read, &value) == T_NUMBER){

                        pos += read;
                        id = value.number;
                        hasID = true;
                    }
                    else{
                                            
                        ERROR("error: expecting integer or hexnum")
                        return retval;
                    }                
                }
                
                if(BLINK_GetToken(&in[pos], inLen - pos, &read, &value) == T_EQUAL){

                    ERROR("no support")
                    return retval;                    
                }
                else{

                    g = newGroup(ctxt, ns);

                    if(g != NULL){

                        g->name = name;
                        g->nameLen = nameLen;
                        g->id = id;
                        g->hasID = hasID;

                        if(BLINK_GetToken(&in[pos], inLen - pos, &read, &value) == T_COLON){

                            pos += read;
                            
                            if((BLINK_GetToken(&in[pos], inLen - pos, &read, &value) == T_CNAME) || (BLINK_GetToken(&in[pos], inLen - pos, &read, &value) == T_NAME)){

                                pos += read;

                                g->superGroup = value.literal.ptr;
                                g->superGroupLen = value.literal.len;
                            }
                            else{

                                ERROR("error: expecting super class name (qname)")
                                return retval;
                            }
                        }

                        if(BLINK_GetToken(&in[pos], inLen - pos, &read, &value) == T_RARROW){

                            pos += read;

                            do{

                                struct blink_field *f = newField(ctxt, g);
                                enum blink_token t = BLINK_GetToken(&in[pos], inLen - pos, &read, &value);

                                switch(t){
                                case T_U8:
                                case T_U16:
                                case T_U32:
                                case T_U64:
                                case T_I8:
                                case T_I16:
                                case T_I32:
                                case T_I64:
                                case T_F64:
                                case T_STRING:
                                case T_BINARY:
                                case T_FIXED:
                                case T_DECIMAL:
                                case T_DATE:
                                case T_MILLI_TIME:
                                case T_NANO_TIME:
                                case T_TIME_OF_DAY_MILLI:
                                case T_TIME_OF_DAY_NANO:

                                    pos += read;

                                    if((t == T_STRING) || (t == T_BINARY)){

                                        if(BLINK_GetToken(&in[pos], inLen - pos, &read, &value) == T_LPAREN){

                                            pos += read;

                                            if(BLINK_GetToken(&in[pos], inLen - pos, &read, &value) == T_NUMBER){

                                                pos += read;
                                            }
                                            else{

                                                ERROR("expecting a size");
                                                return retval;
                                            }

                                            if(value.number > 0xffffffffU){

                                                ERROR("size decode but is out of range")
                                                return retval;
                                            }

                                            f->size = (uint32_t)value.number;
            
                                            if(BLINK_GetToken(&in[pos], inLen - pos, &read, &value) == T_RPAREN){

                                                pos += read;
                                            }   
                                            else{

                                                ERROR("expecting a ')'")
                                                return retval;
                                            }
                                        }
                                        else{

                                            f->size = 0xffffffffU;
                                        }
                                    }
                                    break;

                                case T_NAME:
                                case T_CNAME:
                
                                    pos += read;

                                    f->ref = value.literal.ptr;
                                    f->refLen = value.literal.len;

                                    if(BLINK_GetToken(&in[pos], inLen - pos, &read, &value) == T_STAR){

                                        pos += read;
                                        f->dynamic = true;                                    
                                    }
                                    else{

                                        f->dynamic = false;
                                    }
                                    break;

                                default:

                                    ERROR("expecting a type");
                                    return retval;                                    
                                }

                                /* sequence of type */
                                if(BLINK_GetToken(&in[pos], inLen - pos, &read, &value) == T_LBRACKET){

                                    pos += read;

                                    if(BLINK_GetToken(&in[pos], inLen - pos, &read, &value) == T_RBRACKET){
                                        
                                        pos += read;
                                        f->isSequence = true;
                                    }
                                    else{

                                        ERROR("expecting ']' character")
                                        return retval;
                                    }                        
                                }
                                else{

                                    f->isSequence = false;
                                }

                                /* field name */
                                if(BLINK_GetToken(&in[pos], inLen - pos, &read, &value) == T_NAME){

                                    pos += read;

                                    f->name = value.literal.ptr;
                                    f->nameLen = value.literal.len;

                                    if(BLINK_GetToken(&in[pos], inLen - pos, &read, &value) == T_QUESTION){
                                        
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
                            while(BLINK_GetToken(&in[pos], inLen - pos, &read, &value) == T_COMMA);                   
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

                    ERROR("unknown character")
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

static void destructor(struct blink_schema *ctxt)
{
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

