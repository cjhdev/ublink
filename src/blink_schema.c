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

#include "blink_debug.h"
#include "blink_schema.h"
#include "blink_lexer.h"
#include "blink_schema_internal.h"

#include <string.h>

/* definitions ********************************************************/

#ifndef BLINK_LINK_DEPTH
    /* maximum number of reference to reference links */
    #define BLINK_LINK_DEPTH    10U
#endif

#ifndef BLINK_TOKEN_MAX_SIZE
    #define BLINK_TOKEN_MAX_SIZE 100U
#endif


/* static prototypes **************************************************/

static bool parseSchema(struct blink_schema_base *self, const struct blink_syntax *in);

static struct blink_schema *newListElement(blink_pool_t pool, struct blink_schema **head, enum blink_schema_subclass type);

static bool tokenToIType(enum blink_token tok, enum blink_itype_tag *type);

static struct blink_schema *searchListByName(struct blink_schema *head, const char *name, size_t nameLen);

static void splitCName(const char *in, size_t inLen, const char **nsName, size_t *nsNameLen, const char **name, size_t *nameLen);

static bool resolveDefinitions(struct blink_schema_base *self);
static struct blink_schema *resolve(struct blink_schema_base *self, const char *cName, size_t cNameLen);

static bool testConstraints(struct blink_schema_base *self);
static bool testReferenceConstraint(struct blink_schema_base *self, struct blink_schema *reference);
static bool testSuperGroupReferenceConstraint(struct blink_schema_base *self, struct blink_schema_group *group);
static bool testSuperGroupShadowConstraint(struct blink_schema_base *self, struct blink_schema_group *group);

static struct blink_schema *getTerminal(struct blink_schema *element, bool *dynamic);

static struct blink_group_iterator initDefinitionIterator(struct blink_schema *ns);
static struct blink_schema *nextDefinition(struct blink_group_iterator *iter);
static struct blink_schema *peekDefinition(struct blink_group_iterator *iter);

static struct blink_schema_enum *castEnum(struct blink_schema *self);
static struct blink_schema_symbol *castSymbol(struct blink_schema *self);
static struct blink_schema_field *castField(struct blink_schema *self);
static struct blink_schema_group *castGroup(struct blink_schema *self);
static struct blink_schema_namespace *castNamespace(struct blink_schema *self);
static struct blink_schema_type_def *castTypeDef(struct blink_schema *self);
static struct blink_schema_annote *castAnnote(struct blink_schema *self);
//static struct blink_schema_incr_annote *castIncrAnnote(struct blink_schema *self);
static struct blink_schema_base *castSchema(struct blink_schema *self);

static const char *newString(blink_pool_t pool, const char *ptr, size_t len);

static blink_schema_t takeAnnotes(blink_schema_t *annotes);

/* functions **********************************************************/

blink_schema_t BLINK_Schema_new(blink_pool_t pool, blink_stream_t in)
{
    BLINK_ASSERT(pool != NULL)
    
    blink_schema_t retval = NULL;
    struct blink_schema_base *self = BLINK_Pool_calloc(pool, sizeof(struct blink_schema_base));
    
    if(self != NULL){

        self->pool = pool;

        struct blink_syntax ctxt = {
            .name = NULL,
            .in = in
        };

        if(parseSchema(self, &ctxt)){

            if(resolveDefinitions(self)){

                if(testConstraints(self)){

                    retval = (blink_schema_t)self;
                }
            }
        }
    }
    else{

        /* calloc() */
        BLINK_ERROR("calloc()")
    }

    return retval;    
}

blink_schema_t BLINK_Schema_getGroupByName(blink_schema_t self, const char *name)
{
    BLINK_ASSERT(self != NULL)
    BLINK_ASSERT(name != NULL)
    
    const char *lName;
    size_t lNameLen;
    const char *nsName;
    size_t nsNameLen;

    splitCName(name, strlen(name), &nsName, &nsNameLen, &lName, &lNameLen);

    struct blink_schema_namespace *ns = castNamespace(searchListByName(castSchema(self)->ns, nsName, nsNameLen));

    return (ns == NULL) ? NULL : (blink_schema_t)castGroup(searchListByName(ns->defs, lName, lNameLen));
}

blink_schema_t BLINK_Schema_getGroupByID(blink_schema_t schema, uint64_t id)
{
    BLINK_ASSERT(schema != NULL)

    blink_schema_t retval = NULL;
    struct blink_group_iterator iter = initDefinitionIterator(castSchema(schema)->ns);
    blink_schema_t defPtr = peekDefinition(&iter);

    while((retval == NULL) && (defPtr != NULL)){

        if(defPtr->type == BLINK_SCHEMA_GROUP){

            if(castGroup(defPtr)->hasID && (castGroup(defPtr)->id == id)){

                retval = defPtr;
            }
        }

        defPtr = nextDefinition(&iter);
    }

    return retval;
}

struct blink_field_iterator BLINK_FieldIterator_init(blink_schema_t *stack, size_t depth, blink_schema_t group)
{
    BLINK_ASSERT((depth == 0U) || (stack != NULL))
    BLINK_ASSERT(group != NULL)

    bool dynamic;
    struct blink_schema_group *ptr = castGroup(group);
    struct blink_field_iterator retval;
    size_t i;

    (void)memset(&retval, 0, sizeof(retval));

    retval.field = stack;

    for(i=0U; i < depth; i++){

        retval.field[i] = (blink_schema_t)ptr->f;
        retval.index = i;
        
        if(ptr->s == NULL){

            break;            
        }
        else{

            ptr = castGroup(getTerminal(ptr->s, &dynamic));
        }
    }

    return retval;
}

blink_schema_t BLINK_FieldIterator_next(struct blink_field_iterator *self)
{
    BLINK_ASSERT(self != NULL)
    
    blink_schema_t retval = NULL;

    while(retval == NULL){

        if(self->field[self->index] != NULL){

            retval = self->field[self->index];
            self->field[self->index] = self->field[self->index]->next;
        }
        else if(self->index > 0U){

            self->index--;
        }
        else{

            break;
        }
    }

    return retval;    
}

blink_schema_t BLINK_FieldIterator_peek(struct blink_field_iterator *self)
{
    BLINK_ASSERT(self != NULL)
    
    return self->field[self->index];
}

const char *BLINK_Namespace_getName(blink_schema_t self)
{
    return BLINK_Group_getName(self);
}

const char *BLINK_Group_getName(blink_schema_t self)
{
    BLINK_ASSERT(self != NULL)

    return self->name;
}

blink_schema_t BLINK_Group_getNamespace(blink_schema_t self)
{
    BLINK_ASSERT(self != NULL)

    return (blink_schema_t)castGroup(self)->ns;
}

uint64_t BLINK_Group_getID(blink_schema_t self)
{
    BLINK_ASSERT(self != NULL)

    return castGroup(self)->id;
}

bool BLINK_Group_hasID(blink_schema_t self)
{
    BLINK_ASSERT(self != NULL)

    return castGroup(self)->hasID;
}

const char *BLINK_Field_getName(blink_schema_t self)
{
    return BLINK_Group_getName(self);
}

bool BLINK_Field_isOptional(blink_schema_t self)
{
    BLINK_ASSERT(self != NULL)

    return castField(self)->isOptional;
}

bool BLINK_Field_isSequence(blink_schema_t self)
{
    BLINK_ASSERT(self != NULL)

    return castField(self)->type.isSequence;
}

enum blink_type_tag BLINK_Field_getType(blink_schema_t self)
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

    struct blink_schema_field *field = castField(self);
    enum blink_type_tag retval;
    bool dynamic;

    if(field->type.tag == BLINK_ITYPE_REF){ 

        struct blink_schema *ptr = getTerminal(field->type.resolved, &dynamic);

        switch(ptr->type){
        case BLINK_SCHEMA_ENUM:
            retval = BLINK_TYPE_ENUM;
            break;
        case BLINK_SCHEMA_GROUP:
            retval = (dynamic) ? BLINK_TYPE_DYNAMIC_GROUP : BLINK_TYPE_STATIC_GROUP;
            break;
        default:
            BLINK_ASSERT((size_t)castTypeDef(ptr)->type.tag < (sizeof(translate)/sizeof(*translate)))
            retval = translate[castTypeDef(ptr)->type.tag];
            break;
        }        
    }    
    else{

        BLINK_ASSERT((size_t)field->type.tag < (sizeof(translate)/sizeof(*translate)))
        retval = translate[field->type.tag];
    }

    return retval;
}

uint32_t BLINK_Field_getSize(blink_schema_t self)
{
    BLINK_ASSERT(self != NULL)

    return castField(self)->type.size;
}

blink_schema_t BLINK_Field_getGroup(blink_schema_t self)
{
    BLINK_ASSERT(self != NULL)

    blink_schema_t retval = NULL;
    struct blink_schema_field *field = castField(self);

    if(field->type.tag == BLINK_ITYPE_REF){

        bool dynamic;
        struct blink_schema *ref = getTerminal(field->type.resolved, &dynamic);

        BLINK_ASSERT(ref != NULL)

        if(ref->type == BLINK_SCHEMA_GROUP){

            retval = (blink_schema_t)ref;
        }
    }

    return retval;
}

blink_schema_t BLINK_Field_getEnum(blink_schema_t self)
{
    BLINK_ASSERT(self != NULL)

    blink_schema_t retval = NULL;
    struct blink_schema_field *field = castField(self);

    if(field->type.tag == BLINK_ITYPE_REF){

        bool dynamic;
        struct blink_schema *ref = getTerminal(field->type.resolved, &dynamic);

        BLINK_ASSERT(ref != NULL)

        if(ref->type == BLINK_SCHEMA_ENUM){

            retval = (blink_schema_t)ref;
        }
    }

    return retval;
}

blink_schema_t BLINK_Enum_getSymbolByName(blink_schema_t self, const char *name)
{
    BLINK_ASSERT(self != NULL)
    BLINK_ASSERT(name != NULL)
    
    return (blink_schema_t)searchListByName(castEnum(self)->s, name, strlen(name));
}

blink_schema_t BLINK_Enum_getSymbolByValue(blink_schema_t self, int32_t value)
{
    BLINK_ASSERT(self != NULL)
    
    struct blink_schema *ptr = castEnum(self)->s;
    blink_schema_t retval = NULL;

    while(ptr != NULL){

        if(castSymbol(ptr)->value == value){

            retval = (blink_schema_t)ptr;
            break;
        }

        ptr = ptr->next;
    }

    return retval;
}

const char *BLINK_Symbol_getName(blink_schema_t self)
{
    return BLINK_Group_getName(self);
}

int32_t BLINK_Symbol_getValue(blink_schema_t self)
{
    BLINK_ASSERT(self != NULL)
    
    return castSymbol(self)->value;
}

bool BLINK_Group_isKindOf(blink_schema_t self, blink_schema_t group)
{
    bool retval = false;
    
    if(castGroup(self) == castGroup(group)){

        retval = true;
    }
    else{
                
        bool dynamic;
        struct blink_schema *ptr = getTerminal(castGroup(self)->s, &dynamic);

        while(!retval && (ptr != NULL)){

            if(ptr == group){

                retval = true;
                break;
            }
            else{

                ptr = getTerminal(castGroup(ptr)->s, &dynamic);
            }
        }
    }

    return retval;
}

size_t BLINK_Group_numberOfSuperGroup(blink_schema_t self)
{
    BLINK_ASSERT(self != NULL)

    size_t retval = 0U;
    bool dynamic;   

    struct blink_schema_group *ptr = castGroup(self);

    while(ptr->s != NULL){

        retval++;
        ptr = castGroup(getTerminal(ptr->s, &dynamic));
    };
    
    return retval;
}

struct blink_group_iterator BLINK_GroupIterator_init(blink_schema_t schema)
{
    return initDefinitionIterator(castSchema(schema)->ns);    
}

blink_schema_t BLINK_GroupIterator_next(struct blink_group_iterator *iter)
{
    blink_schema_t retval;

    do{
        
        retval = nextDefinition(iter);
    }
    while((retval != NULL) && (retval->type != BLINK_SCHEMA_GROUP));

    return retval;
}

/* static functions ***************************************************/

static struct blink_schema_namespace *getNamespace(struct blink_schema_base *self, const char *name, size_t nameLen)
{
    struct blink_schema_namespace *retval = castNamespace(searchListByName(self->ns, name, nameLen));

    if(retval == NULL){

        retval = castNamespace(newListElement(self->pool, &self->ns, BLINK_SCHEMA_NS));

        if(retval != NULL){

            retval->super.name = newString(self->pool, name, nameLen);
            retval->super.nameLen = nameLen;
            
            if(retval->super.name == NULL){

                BLINK_ERROR("calloc()")
                retval = NULL;
            }
        }
    }

    return retval;
}

static blink_schema_t takeAnnotes(blink_schema_t *annotes)
{
    BLINK_ASSERT(annotes != NULL)
    BLINK_ASSERT((*annotes == NULL) || ((*annotes)->type == BLINK_SCHEMA_ANNOTE))

    blink_schema_t retval = *annotes;

    *annotes = NULL;

    return retval;
}

static bool parseSchema(struct blink_schema_base *self, const struct blink_syntax *in)
{
    BLINK_ASSERT(self != NULL)
    BLINK_ASSERT(in != NULL)

    bool retval = true;

    enum blink_token tok;
    union blink_token_value value;
    bool shift;
    
    enum parse_state {
        P_START,
        P_NS,
        P_ANY,
        P_DEF_OR_ANNOTE,

        P_DEF_ANNOTE,
        P_TYPEDEF_ANNOTE,
        P_TYPE_ANNOTE,
        P_NAME_ANNOTE,
        P_SYMBOL_ANNOTE,
        
        P_DEF_ANNOTE_VALUE,
        P_TYPEDEF_ANNOTE_VALUE,
        P_TYPE_ANNOTE_VALUE,
        P_NAME_ANNOTE_VALUE,
        P_SYMBOL_ANNOTE_VALUE,
        
        P_DEF,
        P_DEF_NAME,
        P_DEF_TYPE,

        P_GROUP_ID,
        P_GROUP_SUPER,
        P_GROUP_RARROW,

        P_FIELD_OR_ANY,
        P_SUPER_OR_FIELD_OR_ANY,
        
        P_FIELD,
        P_FIELD_TYPE,
        P_FIELD_TYPE_LPAREN,
        P_FIELD_TYPE_LPAREN_OPTIONAL,
        P_FIELD_TYPE_SIZE,
        P_FIELD_TYPE_RPAREN,
        P_FIELD_TYPE_LBRACKET,
        P_FIELD_TYPE_RBRACKET,
        P_FIELD_TYPE_DYNAMIC,        
        P_FIELD_NAME_ANNOTES,        
        P_FIELD_NAME,
        P_FIELD_NAME_SLASH,
        P_FIELD_NAME_ID,
        P_FIELD_OPTIONAL,
        P_FIELD_NEXT,
        
        P_TYPEDEF,
        P_TYPEDEF_TYPE_LPAREN,
        P_TYPEDEF_TYPE_LPAREN_OPTIONAL,
        P_TYPEDEF_TYPE_SIZE,
        P_TYPEDEF_TYPE_RPAREN,
        P_TYPEDEF_TYPE_LBRACKET,
        P_TYPEDEF_TYPE_RBRACKET,
        P_TYPEDEF_TYPE_DYNAMIC,

        P_ENUM,
        P_ENUM_SINGLETON,

        P_ENUM_OR_TYPEDEF,
        
        P_SYMBOL,
        P_SYMBOL_ANNOTES,
        P_SYMBOL_NAME,
        P_SYMBOL_SLASH,
        P_SYMBOL_VALUE,
        P_SYMBOL_NEXT,

        P_SYMBOL_OR_REF
        
        
    } state = P_START;

    const char *name = NULL;
    size_t nameLen = 0U;

    struct blink_schema_group *g = NULL;
    struct blink_schema_field *f = NULL;
    struct blink_schema_enum *e = NULL;
    struct blink_schema_symbol *s = NULL;
    struct blink_schema_namespace *ns = NULL;
    blink_schema_t annotes = NULL;
    struct blink_schema_annote *annote = NULL;
    struct blink_schema_type *type = NULL;

    /* we need the following lookahead set to deal with the difference
     * between an enum and a type definition */
    const char *laName = NULL;
    size_t laNameLen = 0U;
    enum blink_itype_tag laIType = BLINK_ITYPE_REF;
    blink_schema_t laAnnotes = NULL;    

    char buffer[BLINK_TOKEN_MAX_SIZE];

    do{

        tok = BLINK_Lexer_getToken(in->in, buffer, sizeof(buffer), &value, NULL);
        
        if(tok == TOK_ENOMEM){

            BLINK_ERROR("token is too large to buffer")
            retval = false;
            break;
        }

        do{

            shift = true;   /* shift the token unless told otherwise */

            switch(state){
            default:
            case P_START:

                if(tok == TOK_NAMESPACE){

                    state = P_NS;
                }
                else{

                    ns = getNamespace(self, NULL, 0U);

                    if(ns == NULL){

                        retval = false;
                    }
                    else{

                        state = P_ANY;
                        shift = false;
                    }
                }
                break;

            case P_NS:

                if(tok == TOK_NAME){

                    ns = getNamespace(self, value.literal.ptr, value.literal.len);

                    if(ns == NULL){
                        
                        retval = false;
                    }
                    else{

                        state = P_ANY;
                    }
                }
                else{

                    BLINK_ERROR("expecting <name>")
                    retval = false;
                }
                break;

            case P_ANY:

                switch(tok){
                case TOK_NAMESPACE:
                    state = P_NS;
                    break;
                case TOK_AT:
                case TOK_NAME:
                case TOK_CNAME:
                    state = P_DEF_OR_ANNOTE;
                    shift = false;
                    break;
                case TOK_EOF:
                    break;
                default:
                    BLINK_ERROR("got '%s' but expecting 'namespace', '@', <name>, or <cname>", BLINK_Lexer_tokenToString(tok))
                    retval = false;
                    break;
                }
                break;

            case P_DEF_OR_ANNOTE:

                switch(tok){
                case TOK_AT:
                    state = P_DEF_ANNOTE;
                    break;
                case TOK_NAME:                    
                    name = newString(self->pool, value.literal.ptr, value.literal.len);
                    nameLen = value.literal.len;
                    if(name == NULL){

                        retval = false;
                    }
                    else{

                        state = P_DEF_TYPE;
                    }                
                    break;                        
                case TOK_CNAME:
                    BLINK_ERROR("not handling");
                    retval = false;
                    break;
                default:
                    BLINK_ERROR("expecting '@', <name>, or <cname>")
                    retval = false;
                }
                break;

            case P_DEF_ANNOTE:          /* annotes before a definition */
            case P_TYPEDEF_ANNOTE:      /* annotes before a typedef */
            case P_SYMBOL_ANNOTE:       /* annotes before a symbol */
            case P_TYPE_ANNOTE:         /* annotes before a field type */
            case P_NAME_ANNOTE:         /* annotes before a field name */

                switch(tok){
                case TOK_NAME:
                case TOK_CNAME:

                    annote = castAnnote(searchListByName(annotes, value.literal.ptr, value.literal.len)); 

                    if(annote == NULL){

                        annote = castAnnote(newListElement(self->pool, (state == P_TYPEDEF_ANNOTE) ? &laAnnotes : &annotes, BLINK_SCHEMA_ANNOTE));

                        if(annote == NULL){

                            retval = false;
                        }
                        else{

                            annote->super.name = newString(self->pool, value.literal.ptr, value.literal.len);
                            annote->super.nameLen = value.literal.len;

                            if(annote->super.name == NULL){
                                
                                retval = false;
                            }
                        }
                    }

                    if(retval){

                        switch(state){
                        default:
                        case P_DEF_ANNOTE:
                            state = P_DEF_ANNOTE_VALUE;
                            break;
                        case P_TYPEDEF_ANNOTE:
                            state = P_TYPEDEF_ANNOTE_VALUE;
                            break;
                        case P_TYPE_ANNOTE:
                            state = P_TYPE_ANNOTE_VALUE;
                            break;
                        case P_NAME_ANNOTE:
                            state = P_NAME_ANNOTE_VALUE;
                            break;
                        case P_SYMBOL_ANNOTE:
                            state = P_SYMBOL_ANNOTE_VALUE;
                            break;
                        }
                    }
                    break;
                    
                default:
                    BLINK_ERROR("got '%s' but expecting <name> or <cname>", BLINK_Lexer_tokenToString(tok))
                    retval = false;
                    break;
                }
                break;

            case P_DEF_ANNOTE_VALUE:
            case P_TYPEDEF_ANNOTE_VALUE:
            case P_SYMBOL_ANNOTE_VALUE:
            case P_TYPE_ANNOTE_VALUE:
            case P_NAME_ANNOTE_VALUE:
            
                switch(tok){
                case TOK_LITERAL:

                    annote->value = newString(self->pool, value.literal.ptr, value.literal.len);
                    annote->valueLen = value.literal.len;
                    if(annote->value == NULL){

                        retval = false;
                    }
                    else{

                        switch(state){
                        default:
                        case P_DEF_ANNOTE_VALUE:                    
                            state = P_DEF_OR_ANNOTE;
                            break;
                        case P_TYPEDEF_ANNOTE_VALUE:
                            state = P_ENUM_OR_TYPEDEF;
                            break;
                        case P_TYPE_ANNOTE_VALUE:
                            state = P_FIELD_TYPE;
                            break;
                        case P_NAME_ANNOTE_VALUE:
                            state = P_FIELD_NAME;
                            break;
                        case P_SYMBOL_ANNOTE_VALUE:
                            state = P_SYMBOL_NAME;
                            break;
                        }
                    }
                    break;
                    
                default:
                    BLINK_ERROR("expecting <literal>")
                    retval = false;
                    break;
                }
                break;
                    
            case P_DEF_TYPE:

                switch(tok){
                /* create a group */
                case TOK_SLASH:
                case TOK_COLON:
                case TOK_RARROW:
                case TOK_NAME:
                case TOK_CNAME:
                case TOK_AT:
                case TOK_NAMESPACE:
                case TOK_EOF:

                    if(searchListByName(ns->defs, name, nameLen) != NULL){

                        BLINK_ERROR("duplicate definition name")
                        retval = false;
                    }
                    else{

                        g = castGroup(newListElement(self->pool, &ns->defs, BLINK_SCHEMA_GROUP));

                        if(g == NULL){

                            retval = false;
                        }
                        else{
        
                            g->super.name = name;
                            g->super.nameLen = nameLen;
                            g->ns = ns;
                            g->a = takeAnnotes(&annotes);
                            
                            switch(tok){
                            case TOK_SLASH:                        
                                state = P_GROUP_ID;
                                break;
                            case TOK_COLON:
                                state = P_GROUP_SUPER;
                                break;
                            case TOK_RARROW:
                                state = P_FIELD;
                                break;
                            default:
                                state = P_ANY;
                                shift = false;
                                break;
                            }
                        }
                    }
                    break;

                case TOK_LARROW:
                    BLINK_ERROR("not handling <-")
                    retval = false;
                    break;

                case TOK_EQUAL:

                    if(searchListByName(ns->defs, name, nameLen) != NULL){
                
                        BLINK_ERROR("duplicate definition name")
                        retval = false;
                    }
                    else{

                        state = P_ENUM_OR_TYPEDEF;
                    }
                    break;
                
                default:
                    BLINK_ERROR("expecting 'namespace', '@', <cname>, <name>, '/', '=', ':', or '->'")
                    retval = false;
                    break;
                }
                break;

            case P_ENUM_OR_TYPEDEF:

                switch(tok){
                /* <annotes> <name> = | ... */
                case TOK_BAR:
                    state = P_ENUM_SINGLETON;                
                    break;
                /* <annotes> <name> = <laAnnotes> ... */
                case TOK_AT:                    
                    state = P_TYPEDEF_ANNOTE;
                    break;
                /* <annotes> <name> = <laAnnotes> <laName> ... */
                case TOK_NAME:
                    state = P_SYMBOL_OR_REF;                
                    laName = newString(self->pool, value.literal.ptr, value.literal.len);
                    laNameLen = value.literal.len;
                    laIType = BLINK_ITYPE_REF;                    
                    if(laName == NULL){

                        retval = false;
                    }
                    break;
                /* <name> = <laAnnotes> <laIType> */
                default:
                    
                    if(tokenToIType(tok, &laIType)){

                        state = P_TYPEDEF;
                    }
                    else{

                        BLINK_ERROR("expecting <type>")
                        retval = false;
                    }
                    break;
                }
                break;

            case P_SYMBOL_OR_REF:

                switch(tok){
                /* <annotes> <name> = <laAnnotes> <laName> '|' */
                case TOK_BAR:
                /* <annotes> <name> = <laAnnotes> <laName> '/' */
                case TOK_SLASH:
                    state = P_ENUM;
                    shift = false;
                    break;
                /* <annotes> <name> = <laAnnotes> <laName> ... */
                default:
                    state = P_TYPEDEF;
                    shift = false;
                    break;
                }
                break;

            case P_TYPEDEF:
            {
                struct blink_schema_type_def *t = castTypeDef(newListElement(self->pool, &ns->defs, BLINK_SCHEMA_TYPE_DEF));

                if(t == NULL){

                    retval = false;
                }
                else{

                    t->a = takeAnnotes(&annotes);

                    t->super.name = name;
                    t->super.nameLen = nameLen;

                    type = &t->type;

                    type->a = takeAnnotes(&laAnnotes);
                    type->tag = laIType;

                    switch(laIType){
                    case BLINK_ITYPE_REF:
                        type->name = laName;
                        type->nameLen = laNameLen;
                        state = P_TYPEDEF_TYPE_DYNAMIC;
                        break;
                    
                    default:
                        state = P_TYPEDEF_TYPE_LBRACKET;
                        break;
                    }

                    shift = false;                 
                }
            }
                break;
            case P_ENUM_SINGLETON:
            case P_ENUM:

                e = castEnum(newListElement(self->pool, &ns->defs, BLINK_SCHEMA_ENUM));

                if(e == NULL){

                    retval = false;                    
                }
                else{

                    e->a = takeAnnotes(&annotes);
                    
                    e->super.name = name;
                    e->super.nameLen = nameLen;

                    shift = false;

                    switch(state){
                    default:
                    case P_ENUM_SINGLETON:
                        state = P_SYMBOL_ANNOTES;                        
                        break;
                    case P_ENUM:
                        name = laName;
                        nameLen = laNameLen;
                        annotes = laAnnotes;
                        laAnnotes = NULL;
                        state = P_SYMBOL;
                        break;
                    }
                }
                break;

            case P_SYMBOL_ANNOTES:

                if(tok == TOK_AT){

                    state = P_SYMBOL_ANNOTE;
                }
                else{

                    shift = false;
                    state = P_SYMBOL_NAME;
                }
                break;                

            case P_SYMBOL_NAME:

                if(tok == TOK_NAME){

                    name = newString(self->pool, value.literal.ptr, value.literal.len);
                    nameLen = value.literal.len;
                    if(name == NULL){

                        retval = false;
                    }
                    else{
                    
                        state = P_SYMBOL;
                    }
                }
                else{

                    BLINK_ERROR("expecting <name>")
                    retval = false;
                }
                break;

            case P_SYMBOL:

                if(searchListByName(e->s, name, nameLen) != NULL){
                            
                    BLINK_ERROR("duplicate enum symbol name")
                    retval = false;
                }
                else{

                    s = castSymbol(newListElement(self->pool, &e->s, BLINK_SCHEMA_SYMBOL));

                    if(s == NULL){

                        retval = false;
                    }
                    else{

                        s->a = takeAnnotes(&annotes);
                        
                        s->super.name = name;
                        s->super.nameLen = nameLen;

                        state = P_SYMBOL_SLASH;
                        shift = false;                                            
                    }
                }                                    
                break;

            case P_SYMBOL_SLASH:

                if(tok == TOK_SLASH){

                    state = P_SYMBOL_VALUE;
                }
                else{

                    s->implicitValue = true;

                    if(castSymbol(e->s) != s){

                        struct blink_schema *ptr = e->s;
                        while(castSymbol(ptr->next) != s){
                            ptr = ptr->next;
                        }

                        if(castSymbol(ptr)->value == INT32_MAX){

                            BLINK_ERROR("no next implicit enum value possible")
                            retval = false;
                        }
                        else{

                            s->value = castSymbol(ptr)->value + 1;
                        }
                    }

                    state = P_SYMBOL_NEXT;
                    shift = false;
                }
                break;

            case P_SYMBOL_VALUE:

                if((tok == TOK_UINT) || (tok == TOK_INT)){

                    if(tok == TOK_UINT){

                        if(value.number > (uint64_t)INT32_MAX){

                            BLINK_ERROR("enum symbol value out of range")
                            retval = false;
                        }
                        else{
                            
                            s->value = (int32_t)value.number;
                        }
                    }
                    else{
                        
                        if((value.signedNumber > (int64_t)INT32_MAX) || (value.signedNumber < (int64_t)INT32_MIN)){

                            BLINK_ERROR("enum symbol value out of range")
                            retval = false;
                        }
                        else{

                            s->value = (int32_t)value.signedNumber;                            
                        }
                    }

                    if(retval != false){

                        if(castSymbol(e->s) != s){

                            struct blink_schema *ptr = e->s;
                            while(castSymbol(ptr->next) != s){
                                ptr = ptr->next;
                            }
                        

                            if(s->value <= castSymbol(ptr)->value){

                                BLINK_ERROR("enum value is ambiguous")
                                retval = false;
                            }
                        }
                        
                        state = P_SYMBOL_NEXT;
                    }
                }
                else{

                    BLINK_ERROR("expecting <uint> or <int>")
                    retval = false;
                }
                break;

            case P_SYMBOL_NEXT:

                if(tok == TOK_BAR){
            
                    state = P_SYMBOL_ANNOTES;
                }
                else{

                    state = P_ANY;
                    shift = false;
                }
                break;

            case P_SUPER_OR_FIELD_OR_ANY:

                switch(tok){
                case TOK_COLON:
                    state = P_GROUP_SUPER;
                    break;
                case TOK_RARROW:
                    state = P_FIELD;
                    break;
                case TOK_AT:
                case TOK_NAMESPACE:
                case TOK_NAME:
                case TOK_CNAME:
                case TOK_EOF:
                    state = P_ANY;
                    shift = false;
                    break;
                default:
                    BLINK_ERROR("expecting ':', '->', 'namespace', '@', <name>, or <cname>")
                    retval = false;
                    break;
                }
                break;

            case P_FIELD_OR_ANY:

                switch(tok){
                case TOK_RARROW:
                    state = P_FIELD;
                    break;
                case TOK_AT:
                case TOK_NAMESPACE:
                case TOK_NAME:
                case TOK_CNAME:
                case TOK_EOF:
                    state = P_ANY;
                    shift = false;
                    break;
                default:
                    BLINK_ERROR("expecting '->', 'namespace', '@', <name>, or <cname>")
                    retval = false;
                    break;
                }
                break;

            case P_GROUP_ID:

                if(tok == TOK_UINT){

                    g->hasID = true;
                    g->id = value.number;
                    state = P_SUPER_OR_FIELD_OR_ANY;
                }
                else{

                    BLINK_ERROR("expecting <uint>")
                    retval = false;
                }
                break;
                    
            case P_GROUP_SUPER:

                switch(tok){
                case TOK_CNAME:
                case TOK_NAME:

                    g->superGroup = newString(self->pool, value.literal.ptr, value.literal.len);
                    g->superGroupLen = value.literal.len;
                    if(g->superGroup == NULL){

                        retval = false;
                    }
                    else{

                        state = P_FIELD_OR_ANY;
                    }
                    break;
                    
                default:
                    BLINK_ERROR("expecting <name> or <cname> of supergroup")
                    retval = false;
                    break;
                }
                break;

            case P_GROUP_RARROW:

                if(tok == TOK_RARROW){

                    state = P_FIELD;
                }
                else{

                    BLINK_ERROR("expecting '->'")
                    retval = false;
                }
                break;
            
            case P_FIELD:

                f = castField(newListElement(self->pool, &g->f, BLINK_SCHEMA_FIELD));

                if(f == NULL){

                    retval = false;
                }
                else{

                    type = &f->type;

                    switch(tok){                    
                    case TOK_AT:
                        state = P_TYPE_ANNOTE;
                        break;
                    default:
                        state = P_FIELD_TYPE;
                        break;
                    }

                    shift = false;
                }
                break;

            case P_FIELD_TYPE:

                type->a = takeAnnotes(&annotes);

                if(tokenToIType(tok, &type->tag)){

                    switch(tok){
                    case TOK_STRING:
                    case TOK_BINARY:

                        state = P_FIELD_TYPE_LPAREN_OPTIONAL;
                        break;
                        
                    case TOK_FIXED:

                        state = P_FIELD_TYPE_LPAREN;
                        break;
                    
                    default:

                        state = P_FIELD_NAME_ANNOTES;
                        break;
                    }
                }
                else{

                    switch(tok){
                    case TOK_NAME:
                    case TOK_CNAME:

                        type->name = newString(self->pool, value.literal.ptr, value.literal.len);
                        type->nameLen = value.literal.len;
                        if(type->name == NULL){

                            retval = false;
                        }
                        else{
                            
                            type->tag = BLINK_ITYPE_REF;
                            state = P_FIELD_TYPE_DYNAMIC;
                        }
                        break;
                    default:
                        BLINK_ERROR("unexpected token")
                        retval = false;
                        break;
                    }
                }
                break;

            case P_FIELD_TYPE_DYNAMIC:
            case P_TYPEDEF_TYPE_DYNAMIC:

                if(tok == TOK_STAR){

                    type->isDynamic = true;
                }
                else{

                    shift = false;

                    switch(state){
                    default:
                    case P_FIELD_TYPE_DYNAMIC:
                        state = P_FIELD_TYPE_LBRACKET;
                        break;
                    case P_TYPEDEF_TYPE_DYNAMIC:
                        state = P_TYPEDEF_TYPE_LBRACKET;
                        break;
                    }
                }
                break;

            case P_FIELD_TYPE_LBRACKET:
            case P_TYPEDEF_TYPE_LBRACKET:

                if(tok == TOK_LBRACKET){

                    switch(state){
                    default:
                    case P_FIELD_TYPE_LBRACKET:
                        state = P_FIELD_TYPE_RBRACKET;
                        break;
                    case P_TYPEDEF_TYPE_LBRACKET:
                        state = P_TYPEDEF_TYPE_RBRACKET;
                        break;
                    }
                }
                else{

                    shift = false;

                    switch(state){
                    default:
                    case P_FIELD_TYPE_LBRACKET:
                        state = P_FIELD_NAME_ANNOTES;
                        break;
                    case P_TYPEDEF_TYPE_LBRACKET:
                        state = P_ANY;
                        break;
                    }
                }
                break;

            case P_FIELD_TYPE_RBRACKET:
            case P_TYPEDEF_TYPE_RBRACKET:

                if(tok == TOK_RBRACKET){

                    type->isSequence = true;

                    switch(state){
                    default:
                    case P_FIELD_TYPE_RBRACKET:
                        state = P_FIELD_NAME_ANNOTES;
                        break;
                    case P_TYPEDEF_TYPE_RBRACKET:
                        state = P_ANY;
                        break;
                    }
                }
                else{

                    BLINK_ERROR("expecting ']'");
                    retval = false;
                }
                break;

            case P_FIELD_TYPE_LPAREN:
            case P_TYPEDEF_TYPE_LPAREN:

                if(tok == TOK_LPAREN){

                    switch(state){
                    default:
                    case P_FIELD_TYPE_LPAREN:
                        state = P_FIELD_TYPE_SIZE;
                        break;
                    case P_TYPEDEF_TYPE_LPAREN:
                        state = P_TYPEDEF_TYPE_SIZE;
                        break;
                    }
                }
                else{

                    BLINK_ERROR("expecting '('")
                    retval = false;
                }
                break;
                
            case P_FIELD_TYPE_LPAREN_OPTIONAL:
            case P_TYPEDEF_TYPE_LPAREN_OPTIONAL:

                if(tok == TOK_LPAREN){

                    switch(state){
                    default:
                    case P_FIELD_TYPE_LPAREN_OPTIONAL:
                        state = P_FIELD_TYPE_SIZE;
                        break;
                    case P_TYPEDEF_TYPE_LPAREN_OPTIONAL:
                        state = P_TYPEDEF_TYPE_SIZE;
                        break;
                    }
                }
                else{

                    type->size = UINT32_MAX;
                    shift = false;

                    switch(state){
                    default:
                    case P_FIELD_TYPE_LPAREN_OPTIONAL:
                        state = P_FIELD_NAME_ANNOTES;
                        break;
                    case P_TYPEDEF_TYPE_LPAREN_OPTIONAL:
                        state = P_ANY;
                        break;
                    }
                }
                break;
                
            case P_FIELD_TYPE_SIZE:
            case P_TYPEDEF_TYPE_SIZE:

                if(tok == TOK_UINT){

                    if(value.number > 0xffffffffU){
                        BLINK_ERROR("size decode but is out of range")
                        retval = false;
                    }
                    else{

                        type->size = value.number;

                        switch(state){
                        default:
                        case P_FIELD_TYPE_SIZE:
                            state = P_FIELD_NAME_ANNOTES;
                            break;
                        case P_TYPEDEF_TYPE_SIZE:
                            state = P_ANY;
                            break;
                        }
                    }
                }
                else{

                    BLINK_ERROR("expecting <uint>")
                    retval = false;
                }
                break;
            
            case P_FIELD_TYPE_RPAREN:
            case P_TYPEDEF_TYPE_RPAREN:
                
                if(tok == TOK_RPAREN){

                    switch(state){
                    default:
                    case P_FIELD_TYPE_RPAREN:
                        state = P_FIELD_NAME_ANNOTES;
                        break;
                    case P_TYPEDEF_TYPE_RPAREN:
                        state = P_ANY;
                        break;
                    }
                }
                else{

                    BLINK_ERROR("expecting ')'")
                    retval = false;
                }
                break;

            case P_FIELD_NAME_ANNOTES:

                if(tok == TOK_AT){

                    state = P_NAME_ANNOTE;
                }
                else{

                    shift = false;
                    state = P_FIELD_NAME;
                }
                break;
                
            case P_FIELD_NAME:

                f->a = takeAnnotes(&annotes);

                if(tok == TOK_NAME){

                    if(searchListByName(g->f, value.literal.ptr, value.literal.len) != NULL){

                        BLINK_ERROR("duplicate field name")
                        retval = false;
                    }
                    else{

                        f->super.name = newString(self->pool, value.literal.ptr, value.literal.len);
                        f->super.nameLen = value.literal.len;
                        
                        if(f->super.name == NULL){

                            retval = false;
                        }
                        else{
            
                            state = P_FIELD_OPTIONAL;
                        }
                    }
                }
                else{

                    BLINK_ERROR("expecting <name>")
                    retval = false;
                }
                break;

            case P_FIELD_NAME_SLASH:

                if(tok == TOK_SLASH){
                    
                    state = P_FIELD_NAME_ID;
                }
                else{

                    state = P_FIELD_OPTIONAL;
                    shift = false;
                }
                break;

            case P_FIELD_NAME_ID:

                if(tok == TOK_UINT){

                    state = P_FIELD_NAME_ID;                    
                }
                else{

                    BLINK_ERROR("expecting <uint>")
                    retval = false;
                }
                break;

            case P_FIELD_OPTIONAL:

                if(tok == TOK_QUESTION){

                    f->isOptional = true;
                }
                else{
    
                    shift = false;
                }

                state = P_FIELD_NEXT;
                break;

            case P_FIELD_NEXT:

                if(tok == TOK_COMMA){

                    state = P_FIELD;
                }
                else{

                    state = P_ANY;
                    shift = false;
                }
                break;                
            }
        }
        while(retval && (shift == false));
        
    }
    while(retval && (tok != TOK_EOF));

    return retval;
}

static bool tokenToIType(enum blink_token tok, enum blink_itype_tag *type)
{
   bool retval = false;

    static const enum blink_itype_tag translate[] = {
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

    if((size_t)tok < (sizeof(translate)/sizeof(*translate))){
    
        *type = translate[tok];
        retval = true;
    }

    return retval;
}

static bool resolveDefinitions(struct blink_schema_base *self)
{
    BLINK_ASSERT(self != NULL)
    
    struct blink_schema_group *g;
    struct blink_schema_field *f;
    struct blink_schema_type_def *t;
    struct blink_schema *fieldPtr;
    struct blink_group_iterator iter = initDefinitionIterator(self->ns);
    struct blink_schema *defPtr = peekDefinition(&iter);

    while(defPtr != NULL){
        
        switch(defPtr->type){
        case BLINK_SCHEMA_TYPE_DEF:

            t = castTypeDef(defPtr);
        
            if(t->type.tag == BLINK_ITYPE_REF){

                t->type.resolved = resolve(self, t->type.name, t->type.nameLen);

                if(castTypeDef(defPtr)->type.resolved == NULL){

                    BLINK_ERROR("unresolved")
                    return false;
                }
            }
            break;
            
        case BLINK_SCHEMA_GROUP:

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

                    f->type.resolved = resolve(self, f->type.name, f->type.nameLen);

                    if(f->type.resolved == NULL){

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

static struct blink_schema *resolve(struct blink_schema_base *self, const char *cName, size_t cNameLen)
{
    BLINK_ASSERT(self != NULL)
    BLINK_ASSERT(cName != NULL)

    const char *nsName;
    size_t nsNameLen;
    const char *name;
    size_t nameLen;
    struct blink_schema *nsPtr;

    splitCName(cName, cNameLen, &nsName, &nsNameLen, &name, &nameLen);

    nsPtr = searchListByName(self->ns, nsName, nsNameLen);
    
    return (nsPtr != NULL) ? searchListByName(castNamespace(nsPtr)->defs, name, nameLen) : NULL;
}

static bool testConstraints(struct blink_schema_base *self)
{   
    BLINK_ASSERT(self != NULL)

    /* test reference constraints */

    struct blink_group_iterator iter = initDefinitionIterator(self->ns);
    struct blink_schema *defPtr = peekDefinition(&iter);

    while(defPtr != NULL){

        if(!testReferenceConstraint(self, defPtr)){

            return false;
        }
        
        defPtr = nextDefinition(&iter);
    }

    /* test super group constraints */

    iter = initDefinitionIterator(self->ns);
    defPtr = peekDefinition(&iter);

    while(defPtr != NULL){

        if(defPtr->type == BLINK_SCHEMA_GROUP){

            struct blink_schema_group *group = castGroup(defPtr);

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

static bool testReferenceConstraint(struct blink_schema_base *self, struct blink_schema *reference)
{
    BLINK_ASSERT(self != NULL)
    BLINK_ASSERT(reference != NULL)

    bool retval = true;
    struct blink_schema *ptr = reference;
    struct blink_schema *stack[BLINK_LINK_DEPTH];
    bool dynamic = false;
    bool sequence = false;
    size_t depth = 0U;
    size_t i;

    (void)memset(stack, 0, sizeof(stack));

    while(retval && (ptr->type == BLINK_SCHEMA_TYPE_DEF) && (castTypeDef(ptr)->type.tag == BLINK_ITYPE_REF)){    /*lint !e9007 no side effect */

        for(i=0U; i < depth; i++){

            if(stack[i] == ptr){

                BLINK_ERROR("reference cycle detected")
                retval = false;
                break;
            }
        }

        if(i == depth){
        
            if(castTypeDef(ptr)->type.isDynamic){

                /* dynamic reference to dynamic reference */
                if(dynamic){

                    BLINK_ERROR("dynamic reference must resolve to a group")
                    retval = false;
                }
                else{

                    dynamic = true;
                }
            }

            if(retval && castTypeDef(ptr)->type.isSequence){   /*lint !e9007 no side effect */

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
                    ptr = castTypeDef(ptr)->type.resolved;
                }
            }
        }
    }

    if(dynamic && (ptr->type != BLINK_SCHEMA_GROUP)){

        BLINK_ERROR("dynamic reference must resolve to a group")
        retval = false;
    }

    return retval;
}

static bool testSuperGroupReferenceConstraint(struct blink_schema_base *self, struct blink_schema_group *group)
{
    BLINK_ASSERT(self != NULL)
    BLINK_ASSERT(group != NULL)

    bool retval = true;
    struct blink_schema *ptr = group->s;
    struct blink_schema *stack[BLINK_LINK_DEPTH];
    size_t depth = 0U;
    size_t i;

    (void)memset(stack, 0, sizeof(stack));

    while(retval && (ptr->type == BLINK_SCHEMA_TYPE_DEF) && (castTypeDef(ptr)->type.tag == BLINK_ITYPE_REF)){ /*lint !e9007 no side effect */

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
                        ptr = castTypeDef(ptr)->type.resolved;
                    }
                }
            }
        }
    }

    if(retval){

        if(ptr->type != BLINK_SCHEMA_GROUP){

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

static bool testSuperGroupShadowConstraint(struct blink_schema_base *self, struct blink_schema_group *group)
{
    BLINK_ASSERT(self != NULL)
    BLINK_ASSERT(group != NULL)

    size_t depth = BLINK_Group_numberOfSuperGroup((blink_schema_t)group);

    if(depth > 0U){

        /* ancestor iterator will be for each ancestor + this group */
        depth++;
        blink_schema_t aStack[depth];
        blink_schema_t gStack[1U];
        struct blink_field_iterator a = BLINK_FieldIterator_init(aStack, depth, (blink_schema_t)group);
        blink_schema_t aField = BLINK_FieldIterator_next(&a);
        
        while(aField != NULL){
            
            struct blink_field_iterator g = BLINK_FieldIterator_init(gStack, sizeof(gStack)/sizeof(*gStack), (blink_schema_t)group);
            blink_schema_t gField = BLINK_FieldIterator_next(&g);

            while(gField != NULL){

                if(gField == aField){

                    /* exit now since we have compared all the ancestor fields */
                    return true;
                }
                else{

                    if(gField->nameLen == aField->nameLen){

                        if(memcmp(gField->name, aField->name, gField->nameLen) == 0){

                            BLINK_ERROR("field name shadowed in subgroup")
                            return false;
                        }
                    }
                }

                gField = BLINK_FieldIterator_next(&g);
            }
                    
            aField = BLINK_FieldIterator_next(&a);
        }
    }

    return true;
}

static struct blink_schema *newListElement(blink_pool_t pool, struct blink_schema **head, enum blink_schema_subclass type)
{
    BLINK_ASSERT(pool != NULL)
    BLINK_ASSERT(head != NULL)

    struct blink_schema *retval = NULL;

    if(type != BLINK_SCHEMA){

        static const size_t sizes[] = {
            sizeof(struct blink_schema_base),
            sizeof(struct blink_schema_namespace),
            sizeof(struct blink_schema_group),
            sizeof(struct blink_schema_field),
            sizeof(struct blink_schema_enum),
            sizeof(struct blink_schema_symbol),
            sizeof(struct blink_schema_type_def),
            sizeof(struct blink_schema_annote),
            sizeof(struct blink_schema_incr_annote),
        };

        BLINK_ASSERT(type < sizeof(sizes)/sizeof(*sizes))

        retval = (struct blink_schema *)BLINK_Pool_calloc(pool, sizes[type]);
        
        if(retval == NULL){

            BLINK_ERROR("calloc()")
        }
        else{

            retval->type = type;

            if(*head == NULL){

                *head = retval;
            }
            else{

                struct blink_schema *ptr = *head;

                while(ptr->next != NULL){

                    ptr = ptr->next;
                }

                ptr->next = retval;                
            }            
        }
    }
    
    return retval;
}

static struct blink_schema *searchListByName(struct blink_schema *head, const char *name, size_t nameLen)
{
    struct blink_schema *ptr = head;
    struct blink_schema *retval = NULL;

    while(ptr != NULL){

        if(ptr->nameLen == nameLen){

            if(memcmp(ptr->name, name, nameLen) == 0){

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

static struct blink_schema *getTerminal(struct blink_schema *element, bool *dynamic)
{
    BLINK_ASSERT(dynamic != NULL)

    struct blink_schema *ptr = element;
    *dynamic = false;

    if(ptr != NULL){

        while((ptr->type == BLINK_SCHEMA_TYPE_DEF) && (castTypeDef(ptr)->type.tag == BLINK_ITYPE_REF)){   /*lint !e9007 no side effect */

            if(castTypeDef(ptr)->type.isDynamic){

                *dynamic = true;
            }
            
            ptr = castTypeDef(ptr)->type.resolved;
        }
    }

    return ptr;
}

static struct blink_group_iterator initDefinitionIterator(struct blink_schema *ns)
{
    struct blink_group_iterator iter;

    (void)memset(&iter, 0, sizeof(iter));
    iter.ns = ns;
    
    while(iter.ns != NULL){
        iter.def = castNamespace(iter.ns)->defs;
        if(iter.def != NULL){

            break;
        }
        else{

            iter.ns = iter.ns->next;
        }
    }

    return iter;
}

static struct blink_schema *nextDefinition(struct blink_group_iterator *iter)
{
    BLINK_ASSERT(iter != NULL)
    
    struct blink_schema *retval = iter->def;

    if(iter->def != NULL){

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

static struct blink_schema *peekDefinition(struct blink_group_iterator *iter)
{
    return iter->def;
}

static struct blink_schema_enum *castEnum(struct blink_schema *self)
{
    BLINK_ASSERT((self == NULL) || (self->type == BLINK_SCHEMA_ENUM))
    return (struct blink_schema_enum *)self;
}

static struct blink_schema_symbol *castSymbol(struct blink_schema *self)
{
    BLINK_ASSERT((self == NULL) || (self->type == BLINK_SCHEMA_SYMBOL))
    return (struct blink_schema_symbol *)self;
}

static struct blink_schema_field *castField(struct blink_schema *self)
{
    BLINK_ASSERT((self == NULL) || (self->type == BLINK_SCHEMA_FIELD))
    return (struct blink_schema_field *)self;
}

static struct blink_schema_group *castGroup(struct blink_schema *self)
{
    BLINK_ASSERT((self == NULL) || (self->type == BLINK_SCHEMA_GROUP))
    return (struct blink_schema_group *)self;
}

static struct blink_schema_namespace *castNamespace(struct blink_schema *self)
{
    BLINK_ASSERT((self == NULL) || (self->type == BLINK_SCHEMA_NS))
    return (struct blink_schema_namespace *)self;
}

static struct blink_schema_type_def *castTypeDef(struct blink_schema *self)
{
    BLINK_ASSERT((self == NULL) || (self->type == BLINK_SCHEMA_TYPE_DEF))
    return (struct blink_schema_type_def *)self;
}

static struct blink_schema_annote *castAnnote(struct blink_schema *self)
{
    BLINK_ASSERT((self == NULL) || (self->type == BLINK_SCHEMA_ANNOTE))
    return (struct blink_schema_annote *)self;
}
#if 0
static struct blink_schema_incr_annote *castIncrAnnote(struct blink_schema *self)
{
    BLINK_ASSERT((self == NULL) || (self->type == BLINK_SCHEMA_INCR_ANNOTE))
    return (struct blink_schema_incr_annote *)self;
}
#endif

static struct blink_schema_base *castSchema(struct blink_schema *self)
{
    BLINK_ASSERT((self == NULL) || (self->type == BLINK_SCHEMA))
    return (struct blink_schema_base *)self;
}

static const char *newString(blink_pool_t pool, const char *ptr, size_t len)
{
    char *retval = BLINK_Pool_calloc(pool, (len + 1U));

    if(retval != NULL){

        (void)memcpy(retval, ptr, len);
    }

    return retval;
}
