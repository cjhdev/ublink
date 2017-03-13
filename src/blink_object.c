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

/* includes ***********************************************************/

#include "blink_object.h"
#include "blink_compact.h"
#include "blink_stream.h"
#include "blink_schema.h"
#include "blink_debug.h"

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

/* defines ************************************************************/

#ifndef BLINK_OBJECT_NEST_DEPTH
#define BLINK_OBJECT_NEST_DEPTH 10U
#endif

/* types **************************************************************/

union blink_object_value {
    bool boolean;   
    uint64_t u64;   /**< unsigned integer */
    int64_t i64;    /**< signed integer */
    double f64;
    struct blink_string {
        const uint8_t *data;
        uint32_t len;
    } string;       /**< binary, string, and fixed */
    struct blink_decimal {
        int64_t mantissa;   
        int8_t exponent;
    } decimal;
    blink_object_t group;   /**< static and dynamic groups */
};

struct sequence_elem {
    union blink_object_value value;
    struct sequence_elem *next;
};

struct blink_object_field {
    blink_schema_t definition;           /**< field definition */
    union {
        union blink_object_value value;
        struct sequence_type {
            struct sequence_elem *head;
            struct sequence_elem *tail;
            uint32_t size;
        } sequence;
    } data;                         /**< field data may be singular or sequence */
    bool initialised;               /**< true if data has been initialised */
};

struct blink_object {
    uint32_t size;                      /** encoded size cache */
    struct blink_allocator alloc;
    blink_schema_t definition;          /**< group definition */    
    struct blink_object_field *fields;  /**< array of fields */
    size_t numberOfFields;
};

/* used to share scope with helper functions */
struct decode_state {

    struct stack_element {

        uint32_t i;
        uint32_t j;
        uint32_t max;
        blink_object_t g;
        struct blink_object_field *f;

    } stack[BLINK_OBJECT_NEST_DEPTH];

    struct stack_element *top;
    struct blink_stream bounded;
    blink_schema_t schema;
    const struct blink_allocator *alloc;
    union blink_object_value *value;
    bool *initialised;
    
    #if BLINK_OBJECT_NEST_DEPTH > UINT8_MAX
    #error "BLINK_OBJECT_NEST_DEPTH will overflow depth index"
    #endif
    uint8_t depth;
};

typedef bool (* handler)(struct decode_state *);

/* static function prototypes *****************************************/

static bool decodeCompact_groupHeader(blink_stream_t in, struct decode_state *self);
static bool decodeCompact_bool(struct decode_state *self);
static bool decodeCompact_i8(struct decode_state *self);
static bool decodeCompact_i16(struct decode_state *self);
static bool decodeCompact_i32(struct decode_state *self);
static bool decodeCompact_i64(struct decode_state *self);
static bool decodeCompact_u8(struct decode_state *self);
static bool decodeCompact_u16(struct decode_state *self);
static bool decodeCompact_u32(struct decode_state *self);
static bool decodeCompact_u64(struct decode_state *self);
static bool decodeCompact_decimal(struct decode_state *self);
static bool decodeCompact_f64(struct decode_state *self);
static bool decodeCompact_string(struct decode_state *self);
static bool decodeCompact_fixed(struct decode_state *self);
static bool decodeCompact_enum(struct decode_state *self);
static bool decodeCompact_staticGroup(struct decode_state *self);
static bool decodeCompact_dynamicGroup(struct decode_state *self);

static bool BLINK_Object_set(blink_object_t group, const char *fieldName, const union blink_object_value *value);
static union blink_object_value BLINK_Object_get(blink_object_t group, const char *fieldName);

static struct blink_object_field *lookupField(struct blink_object *group, const char *name, size_t nameLen);
static size_t countFields(blink_schema_t group);
static bool encodeBody(const blink_object_t g, blink_stream_t out);
static bool cacheSize(blink_object_t group);

/* functions **********************************************************/

void BLINK_Object_destroyGroup(blink_object_t *group)
{
    BLINK_ASSERT(group != NULL)
    
    uint32_t i;

    if(*group != NULL){
    
        for(i=0U; i < (*group)->numberOfFields; i++){
            
            enum blink_type_tag type = BLINK_Field_getType((*group)->fields[i].definition);
            struct blink_object_field *f = &(*group)->fields[i];
            bool isSequence = BLINK_Field_isSequence(f->definition);
            struct sequence_elem *elem = (isSequence) ? f->data.sequence.head : NULL;
            
            if(!isSequence || (elem != NULL)){

                do{

                    union blink_object_value *value = (isSequence) ? &elem->value : &f->data.value;

                    switch(type){
                    case BLINK_TYPE_STRING:            
                    case BLINK_TYPE_BINARY:
                    case BLINK_TYPE_FIXED:
                        if((*group)->alloc.free != NULL){                
                            (*group)->alloc.free((void *)value->string.data);
                            value->string.data = NULL;
                        }
                        break;
                    case BLINK_TYPE_DYNAMIC_GROUP:
                    case BLINK_TYPE_STATIC_GROUP:
                        BLINK_Object_destroyGroup(&value->group);
                        break;
                    default:
                        break;
                    }

                    if(isSequence){

                        struct sequence_elem *cur = elem;
                        elem = elem->next;

                        if((*group)->alloc.free != NULL){                

                            (*group)->alloc.free(cur);
                        }
                    }
                }
                while(elem != NULL);
            }                        
        }

        if((*group)->alloc.free != NULL){
            
            (*group)->alloc.free(*group);
        }
                    
        *group = NULL;
    }
}

blink_object_t BLINK_Object_newGroup(const struct blink_allocator *alloc, blink_schema_t group)
{
    BLINK_ASSERT(group != NULL)
    BLINK_ASSERT(alloc != NULL)

    blink_object_t retval = NULL;

    if(alloc->calloc != NULL){
    
        struct blink_object *self = alloc->calloc(1U, sizeof(struct blink_object));

        if(self != NULL){

            self->alloc = *alloc;
            self->definition = group;
            self->numberOfFields = countFields(group);

            if(self->numberOfFields > 0U){

                self->fields = self->alloc.calloc(self->numberOfFields, sizeof(struct blink_object_field));

                if(self->fields != NULL){

                    size_t stackDepth = BLINK_Group_numberOfSuperGroup(group) + 1U;
                    blink_schema_t stack[stackDepth];
                    struct blink_field_iterator iter = BLINK_FieldIterator_init(stack, stackDepth, group);
                    blink_schema_t f = BLINK_FieldIterator_next(&iter);
                    uint32_t i = 0U;

                    while(f != NULL){
                    
                        self->fields[i].definition = f;
                        f = BLINK_FieldIterator_next(&iter);
                        i++;
                    }

                    retval = (blink_object_t)self;
                }
                else{

                    BLINK_ERROR("calloc()")

                    if(alloc->free != NULL){

                        alloc->free(self);
                    }
                }
            }
            else{
            
                retval = (blink_object_t)self;
            }
        }
        else{

            BLINK_ERROR("calloc()")
        }
    }
    else{

        BLINK_ERROR("alloc struct must define a pointer to a calloc-like function")
    }

    return retval;    
}

blink_object_t BLINK_Object_decodeCompact(blink_stream_t in, blink_schema_t schema, const struct blink_allocator *alloc)
{
    const static handler decoder[] = {
        decodeCompact_string,       /* BLINK_TYPE_STRING */
        decodeCompact_string,       /* BLINK_TYPE_BINARY */
        decodeCompact_fixed,        /* BLINK_TYPE_FIXED */
        decodeCompact_bool,         /* BLINK_TYPE_BOOL */
        decodeCompact_u8,           /* BLINK_TYPE_U8 */
        decodeCompact_u16,          /* BLINK_TYPE_U16 */
        decodeCompact_u32,          /* BLINK_TYPE_U32 */
        decodeCompact_u64,          /* BLINK_TYPE_U64 */
        decodeCompact_i8,           /* BLINK_TYPE_I8 */
        decodeCompact_i16,          /* BLINK_TYPE_I16 */
        decodeCompact_i32,          /* BLINK_TYPE_I32 */
        decodeCompact_i64,          /* BLINK_TYPE_I64 */
        decodeCompact_f64,          /* BLINK_TYPE_F64 */
        decodeCompact_i32,          /* BLINK_TYPE_DATE */
        decodeCompact_u32,          /* BLINK_TYPE_TIME_OF_DAY_MILLI */
        decodeCompact_u64,          /* BLINK_TYPE_TIME_OF_DAY_NANO */
        decodeCompact_i64,          /* BLINK_TYPE_NANO_TIME */
        decodeCompact_i64,          /* BLINK_TYPE_MILLI_TIME */
        decodeCompact_decimal,      /* BLINK_TYPE_DECIMAL */
        decodeCompact_dynamicGroup, /* BLINK_TYPE_OBJECT */
        decodeCompact_enum,         /* BLINK_TYPE_ENUM */
        decodeCompact_staticGroup,   /* BLINK_TYPE_STATIC_GROUP */
        decodeCompact_dynamicGroup  /* BLINK_TYPE_DYNAMIC_GROUP */
    };

    blink_object_t retval = NULL;    
    struct decode_state self;
    bool isNull;
    bool error = false;
    
    (void)memset(&self, 0, sizeof(self));

    self.top = self.stack;
    self.alloc = alloc;
    self.schema = schema;

    if(decodeCompact_groupHeader(in, &self)){

        while(!error){

            if(self.top->i < self.top->g->numberOfFields){

                self.top->f = &self.top->g->fields[self.top->i];

                enum blink_type_tag type = BLINK_Field_getType(self.top->f->definition);

                if(BLINK_Field_isSequence(self.top->f->definition)){

                    if(self.top->j == 0){

                        if(BLINK_Compact_decodeU32(&self.bounded, &self.top->f->data.sequence.size, &isNull)){

                            self.top->j++;

                            if(isNull){

                                if(BLINK_Field_isOptional(self.top->f->definition)){

                                    self.top->j = 0U;
                                    self.top->i++;
                                }
                                else{

                                    BLINK_ERROR("cannot be NULL")
                                    error = true;
                                }             
                            }
                        }
                        else{

                            error = true;
                        }
                    }
                    else{

                        if(self.top->j <= self.top->f->data.sequence.size){
                        
                            struct sequence_elem *elem = alloc->calloc(1, sizeof(struct sequence_elem));

                            if(elem == NULL){

                                BLINK_ERROR("calloc()")
                                error = true;
                            }
                            else{

                                if(self.top->f->data.sequence.tail == NULL){

                                    self.top->f->data.sequence.head = elem;
                                    self.top->f->data.sequence.tail = elem;
                                }
                                else{

                                    self.top->f->data.sequence.tail->next = elem;
                                    self.top->f->data.sequence.tail = elem;                            
                                }

                                self.value = &elem->value;
                                self.top->j++;
                                self.initialised = NULL;
                                
                                //callout
                                error = (decoder[type](&self)) ? false : true;
                            }
                        }
                        else{

                            self.top->j = 0U;
                            self.top->i++;
                        }
                    }
                }
                else{

                    //callout
                    self.value = &self.top->f->data.value;
                    self.initialised = &self.top->f->initialised;
                    self.top->i++;
                    error = (decoder[type](&self)) ? false : true;                                
                }          
            }

            if(!error){
                
                if(self.top->i == self.top->g->numberOfFields){

                    if(
                        (
                            (self.top == self.stack)
                            ||
                            (
                                (BLINK_Field_getType(self.top[-1].f->definition) == BLINK_TYPE_DYNAMIC_GROUP)
                                ||
                                (BLINK_Field_getType(self.top[-1].f->definition) == BLINK_TYPE_OBJECT)
                            )
                        )
                        &&
                        (BLINK_Stream_tell(&self.bounded) < BLINK_Stream_max(&self.bounded))
                    ){
                                
                        BLINK_ERROR("additional bytes at end of group are not allowed...for now")
                        error = true;                        
                    }
                    /* unwind */
                    else{

                        if(self.top == self.stack){

                            /* finished */
                            retval = self.stack->g;
                            break;
                        }
                        else{

                            self.top = &self.top[-1];
                            (void)BLINK_Stream_setMax(&self.bounded, self.top->max);
                        }
                    }
                }
            }            
        }
    }
    else{

        error = true;
    }

    if(error){

        if(BLINK_Stream_eof(in)){

            BLINK_ERROR("S1: group ended prematurely")
        }
        else{

            if(BLINK_Stream_eof(&self.bounded)){

                BLINK_ERROR("S1: nested group ended prematurely")
            }            
        }

        BLINK_Object_destroyGroup(&self.stack->g);        
    }

    return retval;
}

bool BLINK_Object_encodeCompact(blink_object_t group, blink_stream_t out)
{
    bool retval = false;

    if(BLINK_Group_hasID(group->definition)){

        uint64_t id = BLINK_Group_getID(group->definition);

        if(cacheSize(group)){

            if(BLINK_Compact_encodeU32(group->size + BLINK_Compact_sizeofUnsigned(id), out)){

                if(BLINK_Compact_encodeU64(id, out)){

                    retval = encodeBody(group, out);
                }
            }
        }
    }
    else{

        BLINK_ERROR("cannot encode group without an ID")
    }

    return retval;
}

bool BLINK_Object_append(blink_object_t group, const char *fieldName, const union blink_object_value *value)
{
    BLINK_ASSERT(group != NULL)

    bool retval = false;
    struct blink_object_field *field = lookupField(group, fieldName, strlen(fieldName));

    if(field != NULL){

        if(BLINK_Field_isSequence(field->definition)){


        }
        else{

            BLINK_ERROR("cannot append to a field which is not a sequence")
        }
    }
    
    return retval;
}

void BLINK_Object_iterate(blink_object_t group, const char *fieldName, void *user, bool (*each)(void *user, const char *fieldName, const union blink_object_value *value))
{
    BLINK_ASSERT(group != NULL)

    struct sequence_elem *seq;
    struct blink_object_field *field = lookupField(group, fieldName, strlen(fieldName));

    if(field != NULL){

        if(BLINK_Field_isSequence(field->definition)){

            seq = field->data.sequence.head;

            while(seq != NULL){

                if(!each(user, fieldName, &seq->value)){

                    break;
                }            
            }
        }
    }
}
    
bool BLINK_Object_clear(blink_object_t group, const char *fieldName)
{
    BLINK_ASSERT(group != NULL)

    bool retval = false;
    struct blink_object_field *field = lookupField(group, fieldName, strlen(fieldName));

    if(field != NULL){

        field->initialised = false;
        retval = true;
    }

    return retval;
}

bool BLINK_Object_setEnum(blink_object_t group, const char *fieldName, const char *symbol)
{
    union blink_object_value value = {.string = {.data = (uint8_t *)symbol}};

    return BLINK_Object_set(group, fieldName, &value);
}

bool BLINK_Object_setBool(blink_object_t group, const char *fieldName, bool value)
{
    return BLINK_Object_set(group, fieldName, (union blink_object_value *)value);    
}

bool BLINK_Object_setDecimal(blink_object_t group, const char *fieldName, int64_t mantissa, int8_t exponent)
{
    union blink_object_value value = {.decimal = {.mantissa = mantissa, .exponent = exponent}};

    return BLINK_Object_set(group, fieldName, &value);    
}

bool BLINK_Object_setUint(blink_object_t group, const char *fieldName, uint64_t value)
{
    return BLINK_Object_set(group, fieldName, (union blink_object_value *)&value);    
}

bool BLINK_Object_setInt(blink_object_t group, const char *fieldName, int64_t value)
{
    return BLINK_Object_set(group, fieldName, (union blink_object_value *)&value);    
}

bool BLINK_Object_setF64(blink_object_t group, const char *fieldName, double value)
{
    return BLINK_Object_set(group, fieldName, (union blink_object_value *)&value);    
}

bool BLINK_Object_setString(blink_object_t group, const char *fieldName, const char *str, uint32_t len)
{
    union blink_object_value value = {.string = {.data = (uint8_t *)str, .len = len}};

    return BLINK_Object_set(group, fieldName, &value);        
}

bool BLINK_Object_setString2(blink_object_t group, const char *fieldName, const char *str)
{
    union blink_object_value value = {.string = {.data = (uint8_t *)str, .len = (uint32_t)strlen(str)}};
    
    return BLINK_Object_set(group, fieldName, &value);        
}

bool BLINK_Object_setBinary(blink_object_t group, const char *fieldName, const uint8_t *data, uint32_t len)
{
    union blink_object_value value = {.string = {.data = data, .len = len}};
    
    return BLINK_Object_set(group, fieldName, &value);        
}

bool BLINK_Object_setFixed(blink_object_t group, const char *fieldName, const uint8_t *data, uint32_t len)
{
    union blink_object_value value = {.string = {.data = data, .len = len}};
    
    return BLINK_Object_set(group, fieldName, &value);        
}

bool BLINK_Object_setGroup(blink_object_t group, const char *fieldName, blink_object_t value)
{
    return BLINK_Object_set(group, fieldName, (union blink_object_value *)value);        
}

bool BLINK_Object_fieldIsNull(blink_object_t group, const char *fieldName)
{
    BLINK_ASSERT(group != NULL)

    bool retval = false;
    struct blink_object_field *field = lookupField(group, fieldName, strlen(fieldName));

    if(field != NULL){

        retval = (false == field->initialised);
    }

    return retval;
}

const char *BLINK_Object_getEnum(blink_object_t group, const char *fieldName)
{
    return (const char *)BLINK_Object_get(group, fieldName).string.data;
}

bool BLINK_Object_getBool(blink_object_t group, const char *fieldName)
{
    return BLINK_Object_get(group, fieldName).boolean;
}

void BLINK_Object_getDecimal(blink_object_t group, const char *fieldName, int64_t *mantissa, int8_t *exponent)
{
    union blink_object_value value = BLINK_Object_get(group, fieldName);

    *mantissa = value.decimal.mantissa;
    *exponent = value.decimal.exponent;
}

uint64_t BLINK_Object_getUint(blink_object_t group, const char *fieldName)
{
    return BLINK_Object_get(group, fieldName).u64;
}

int64_t BLINK_Object_getInt(blink_object_t group, const char *fieldName)
{
    return BLINK_Object_get(group, fieldName).i64;
}

double BLINK_Object_getF64(blink_object_t group, const char *fieldName)
{
    return BLINK_Object_get(group, fieldName).f64;
}

void BLINK_Object_getString(blink_object_t group, const char *fieldName, const char **str, uint32_t *len)
{
    union blink_object_value value = BLINK_Object_get(group, fieldName);

    *str = (char *)value.string.data;
    *len = value.string.len;    
}

void BLINK_Object_getBinary(blink_object_t group, const char *fieldName, const uint8_t **data, uint32_t *len)
{
    union blink_object_value value = BLINK_Object_get(group, fieldName);

    *data = value.string.data;
    *len = value.string.len;
}

void BLINK_Object_getFixed(blink_object_t group, const char *fieldName, const uint8_t **data, uint32_t *len)
{
    union blink_object_value value = BLINK_Object_get(group, fieldName);

    *data = value.string.data;
    *len = value.string.len;
}   

blink_object_t BLINK_Object_getGroup(blink_object_t group, const char *fieldName)
{
    return BLINK_Object_get(group, fieldName).group;
}

/* static functions ***************************************************/

static bool decodeCompact_groupHeader(blink_stream_t in, struct decode_state *self)
{
    bool retval = false;
    bool isNull;
    uint64_t id;    
    
    if(BLINK_Compact_decodeU32(in, &self->top->max, &isNull)){
    
        if(isNull || (self->top->max == 0U)){

            BLINK_ERROR("W1: Top level group size is NULL or zero")
        }
        else{

            (void)BLINK_Stream_initBounded(&self->bounded, in, self->top->max);

            if(BLINK_Compact_decodeU64(&self->bounded, &id, &isNull)){

                if(isNull){

                    BLINK_ERROR("W1: unknown group ID")                
                }
                else{

                    blink_schema_t groupDef = BLINK_Schema_getGroupByID(self->schema, id);

                    if(groupDef == NULL){

                        BLINK_ERROR("W1: unknown group ID")
                    }
                    else{

                        self->top->g = BLINK_Object_newGroup(self->alloc, groupDef);

                        if(self->top->g != NULL){

                            retval = true;
                        }                        
                    }
                }
            }
        }
    }

    return retval;
}

static bool decodeCompact_fixed(struct decode_state *self)
{
    bool retval = true;
    bool isPresent = true;
    struct stack_element *top = self->top;
                
    if(BLINK_Field_isOptional(top->f->definition)){

        if(!BLINK_Compact_decodePresent(&self->bounded, &isPresent)){

            retval = false;
        }
    }

    if(retval && isPresent){

        retval = false;
        
        uint32_t size = BLINK_Field_getSize(top->f->definition);        
        uint8_t *data = self->alloc->calloc(1, size);

        if(data != NULL){

            self->value->string.data = data;
            self->value->string.len = size;

            if(BLINK_Stream_read(&self->bounded, data, size)){

               retval = true;

               if(self->initialised != NULL){

                    *self->initialised = true;
               }               
            }
        }
        else{

            BLINK_ERROR("calloc()")
        }        
    }
    
    return retval;
}

static bool decodeCompact_string(struct decode_state *self)
{
    bool retval = false;
    uint32_t size;
    bool isNull;
    struct stack_element *top = self->top;

    if(BLINK_Compact_decodeU32(&self->bounded, &size, &isNull)){

        if(isNull){

            if(BLINK_Field_isOptional(top->f->definition)){
    
                retval = true;
            }            
        }
        else{
            
            if(size > 0U){

                uint8_t *data = self->alloc->calloc(1U, size);

                if(data != NULL){

                    self->value->string.data = data;                        
                    self->value->string.len = size;        
                
                    if(BLINK_Stream_read(&self->bounded, data, size)){

                        retval = true;

                        if(self->initialised != NULL){

                            *self->initialised = true;
                       }               
                    }
                }
                else{

                    BLINK_ERROR("calloc()")
                }
            }
            else{

                retval = true;
            }
        }
    }
    
    return retval;
}

static bool decodeCompact_bool(struct decode_state *self)
{
    bool retval = false;
    bool isNull;
    bool value;
    struct stack_element *top = self->top;
    
    if(BLINK_Compact_decodeBool(&self->bounded, &value, &isNull)){

        if(isNull){

            if(BLINK_Field_isOptional(top->f->definition)){
    
                retval = true;
            }            
        }
        else{
        
            self->value->boolean = value;
            if(self->initialised != NULL){

                *self->initialised = true;
            }
            retval = true;
        }                                                        
    }
    
    return retval;
}

static bool decodeCompact_u8(struct decode_state *self)
{
    bool retval = false;
    bool isNull;
    uint8_t value;
    struct stack_element *top = self->top;
    
    if(BLINK_Compact_decodeU8(&self->bounded, &value, &isNull)){

        if(isNull){

            if(BLINK_Field_isOptional(top->f->definition)){
    
                retval = true;
            }            
        }
        else{

            self->value->u64 = (uint64_t)value;
            if(self->initialised != NULL){

                *self->initialised = true;
            }
            retval = true;
        }                    
    }

    return retval;
}

static bool decodeCompact_u16(struct decode_state *self)
{
    bool retval = false;
    bool isNull;
    uint16_t value;
    struct stack_element *top = self->top;
    
    if(BLINK_Compact_decodeU16(&self->bounded, &value, &isNull)){

        if(isNull){

            if(BLINK_Field_isOptional(top->f->definition)){
    
                retval = true;
            }            
        }
        else{

            self->value->u64 = (uint64_t)value;
            retval = true;
        }                    
    }

    return retval;
}
static bool decodeCompact_u32(struct decode_state *self)
{
    bool retval = false;
    bool isNull;
    uint32_t value;
    struct stack_element *top = self->top;
    
    if(BLINK_Compact_decodeU32(&self->bounded, &value, &isNull)){

        if(isNull){

            if(BLINK_Field_isOptional(top->f->definition)){
    
                retval = true;
            }            
        }
        else{

            if(self->initialised != NULL){

                *self->initialised = true;
            }
            self->value->u64 = (uint64_t)value;
            retval = true;
        }                    
    }

    return retval;
}
static bool decodeCompact_u64(struct decode_state *self)
{
    bool retval = false;
    bool isNull;
    uint64_t value;
    struct stack_element *top = self->top;
    
    if(BLINK_Compact_decodeU64(&self->bounded, &value, &isNull)){

        if(isNull){

            if(BLINK_Field_isOptional(top->f->definition)){
    
                retval = true;
            }            
        }
        else{

            if(self->initialised != NULL){

                *self->initialised = true;
            }
            self->value->u64 = value;
            retval = true;
        }                    
    }

    return retval;
}
static bool decodeCompact_i8(struct decode_state *self)
{
    bool retval = false;
    bool isNull;
    int8_t value;
    struct stack_element *top = self->top;
    
    if(BLINK_Compact_decodeI8(&self->bounded, &value, &isNull)){

        if(isNull){

            if(BLINK_Field_isOptional(top->f->definition)){
    
                retval = true;
            }            
        }
        else{

            if(self->initialised != NULL){

                *self->initialised = true;
            }
            self->value->i64 = (int64_t)value;
            retval = true;
        }                    
    }

    return retval;
}
static bool decodeCompact_i16(struct decode_state *self)
{
    bool retval = false;
    bool isNull;
    int16_t value;
    struct stack_element *top = self->top;
    
    if(BLINK_Compact_decodeI16(&self->bounded, &value, &isNull)){

        if(isNull){

            if(BLINK_Field_isOptional(top->f->definition)){
    
                retval = true;
            }            
        }
        else{

            if(self->initialised != NULL){

                *self->initialised = true;
            }
            self->value->i64 = (int64_t)value;
            retval = true;        
        }                    
    }

    return retval;
}
static bool decodeCompact_i32(struct decode_state *self)
{
    bool retval = false;
    bool isNull;
    int32_t value;
    struct stack_element *top = self->top;
    
    if(BLINK_Compact_decodeI32(&self->bounded, &value, &isNull)){

        if(isNull){

            if(BLINK_Field_isOptional(top->f->definition)){
    
                retval = true;
            }            
        }
        else{

            if(self->initialised != NULL){

                *self->initialised = true;
            }
            self->value->i64 = (int64_t)value;
            retval = true;
        }                    
    }

    return retval;
}
static bool decodeCompact_i64(struct decode_state *self)
{
    bool retval = false;
    bool isNull;
    int64_t value;
    struct stack_element *top = self->top;
    
    if(BLINK_Compact_decodeI64(&self->bounded, &value, &isNull)){

        if(isNull){

            if(BLINK_Field_isOptional(top->f->definition)){
    
                retval = true;
            }            
        }
        else{

            if(self->initialised != NULL){

                *self->initialised = true;
            }
            self->value->i64 = value;
            retval = true;
        }                    
    }

    return retval;
}

static bool decodeCompact_enum(struct decode_state *self)
{
    bool retval = false;
    bool isNull;
    int32_t value;
    struct stack_element *top = self->top;
    
    if(BLINK_Compact_decodeI32(&self->bounded, &value, &isNull)){

        if(isNull){

            if(BLINK_Field_isOptional(top->f->definition)){
    
                retval = true;
            }            
        }
        else{

            if(BLINK_Enum_getSymbolByValue(top->f->definition, value) != NULL){

                if(self->initialised != NULL){

                    *self->initialised = true;
                }
                self->value->i64 = (int64_t)value;            
                retval = true;
            }
            else{

                BLINK_ERROR("symbol not found in enum")
            }                        
        }                    
    }

    return retval;
}

static bool decodeCompact_f64(struct decode_state *self)
{
    bool retval = false;
    bool isNull;
    double value;
    struct stack_element *top = self->top;
    
    if(BLINK_Compact_decodeF64(&self->bounded, &value, &isNull)){

        if(isNull){

            if(BLINK_Field_isOptional(top->f->definition)){
    
                retval = true;
            }            
        }
        else{

            if(self->initialised != NULL){

                *self->initialised = true;
            }
            self->value->f64 = value;
            retval = true;
        }
    }

    return retval;
}

static bool decodeCompact_decimal(struct decode_state *self)
{
    bool retval = false;
    bool isNull;
    int64_t mantissa;
    int8_t exponent;
    struct stack_element *top = self->top;
    
    if(BLINK_Compact_decodeDecimal(&self->bounded, &mantissa, &exponent, &isNull)){

        if(isNull){

            if(BLINK_Field_isOptional(top->f->definition)){
    
                retval = true;
            }            
        }
        else{

            if(self->initialised != NULL){

                *self->initialised = true;
            }
            self->value->decimal.mantissa = mantissa;
            self->value->decimal.exponent = exponent;
            retval = true;
        }
    }

    return retval;
}

static bool decodeCompact_staticGroup(struct decode_state *self)
{
    bool retval = true;
    bool isPresent = true;
    struct stack_element *top = self->top;
                
    if(BLINK_Field_isOptional(top->f->definition)){

        if(!BLINK_Compact_decodePresent(&self->bounded, &isPresent)){

            retval = false;
        }
    }

    if(retval && isPresent){

        retval = false;

        if(top == &self->stack[(BLINK_OBJECT_NEST_DEPTH-1U)]){

            BLINK_ERROR("too much nesting")
        }
        else{

            (void)memset(&top[1], 0, sizeof(*self->stack));

            top[1].g = BLINK_Object_newGroup(self->alloc, BLINK_Field_getGroup(top->f->definition));

            if(top[1].g != NULL){

                top = &top[1];
                retval = true;
            }
            else{

                BLINK_ERROR("calloc()")
            }            
        }
    }

    return retval;
}

static bool decodeCompact_dynamicGroup(struct decode_state *self)
{
    bool retval = false;
    bool isNull;
    uint32_t size;
    uint64_t id;
    blink_schema_t groupDef;
    struct stack_element *top = self->top;
    
    if(BLINK_Compact_decodeU32(&self->bounded, &size, &isNull)){

        if(isNull){

            if(BLINK_Field_isOptional(top->f->definition)){

                retval = true;
            }            
        }
        else if(size == 0U){

            BLINK_ERROR("W1: Group cannot have size of zero")
        }
        else if((BLINK_Stream_max(&self->bounded) - BLINK_Stream_tell(&self->bounded)) < size){

            BLINK_ERROR("S1: nested group will overrun parent group")
        }
        else{

            if(top == &self->stack[(BLINK_OBJECT_NEST_DEPTH-1U)]){

                BLINK_ERROR("too much nesting")
            }
            else{

                (void)memset(&top[1], 0, sizeof(*self->stack));

                BLINK_Stream_setMax(&self->bounded, top->max);
                        
                if(BLINK_Compact_decodeU64(&self->bounded, &id, &isNull)){

                    if(!isNull){

                        groupDef = BLINK_Schema_getGroupByID(self->schema, id);
                    }

                    if(isNull || (groupDef == NULL)){

                        BLINK_ERROR("W14: Group is unknown")
                    }
                    else{

                        if((BLINK_Field_getType(top->f->definition) == BLINK_TYPE_OBJECT) || BLINK_Group_isKindOf(groupDef, BLINK_Field_getGroup(top->f->definition))){

                            top = &top[1];
                            top->max = size;

                            top->g = BLINK_Object_newGroup(self->alloc, groupDef);

                            if(top->g != NULL){

                                retval = true;
                            }
                            else{

                                BLINK_ERROR("calloc()")
                            }
                        }
                        else{

                            BLINK_ERROR("not what we expect")
                        }
                    }
                }
            }
        }
    }
     
    return retval;
}

static bool BLINK_Object_set(blink_object_t group, const char *fieldName, const union blink_object_value *value)
{
    BLINK_ASSERT(group != NULL)

    bool retval = false;
    struct blink_object_field *field = lookupField(group, fieldName, strlen(fieldName));

    if(field != NULL){
    
        enum blink_type_tag type = BLINK_Field_getType(field->definition);

        switch(type){
        case BLINK_TYPE_STRING:            
        case BLINK_TYPE_BINARY:         

            if(value->string.len <= BLINK_Field_getSize(field->definition)){

                uint8_t *data = group->alloc.calloc(1, value->string.len);

                if(data != NULL){

                    (void)memcpy(data, value->string.data, value->string.len);
                    field->data.value.string.data = data;
                    field->data.value.string.len = value->string.len;
                    retval = true;                    
                }
                else{

                    BLINK_ERROR("calloc()");
                }        
            }
            else{

                BLINK_ERROR("string too large for definition")
            }
            break;
        case BLINK_TYPE_FIXED:
            if(value->string.len == BLINK_Field_getSize(field->definition)){

                uint8_t *data = group->alloc.calloc(1, value->string.len);

                if(data != NULL){

                    (void)memcpy(data, value->string.data, value->string.len);
                    field->data.value.string.data = data;
                    field->data.value.string.len = value->string.len;
                    retval = true;
                }
                else{

                    BLINK_ERROR("calloc()");
                }            
            }
            else{

                BLINK_ERROR("wrong size fixed field")
            }
            break;
        case BLINK_TYPE_BOOL:
            field->data.value.boolean = value->boolean;
            retval = true;
            break;
        case BLINK_TYPE_U8:
        case BLINK_TYPE_U16:
        case BLINK_TYPE_U32:
        case BLINK_TYPE_U64:
        case BLINK_TYPE_TIME_OF_DAY_MILLI:
        case BLINK_TYPE_TIME_OF_DAY_NANO:
            field->data.value.u64 = value->u64;
            retval = true;
            break;        
        case BLINK_TYPE_I8:        
        case BLINK_TYPE_I16:
        case BLINK_TYPE_I32:
        case BLINK_TYPE_DATE:
        case BLINK_TYPE_NANO_TIME:
        case BLINK_TYPE_MILLI_TIME:
        case BLINK_TYPE_I64:
            field->data.value.i64 = value->i64;
            retval = true;
            break;        
        case BLINK_TYPE_ENUM:
        {
            blink_schema_t s = BLINK_Enum_getSymbolByName(field->definition, (char *)value->string.data);

            if(s != NULL){

                field->data.value.i64 = (int64_t)BLINK_Symbol_getValue(s);
                retval = true;
            }
            else{

                BLINK_ERROR("enum symbol \"%s\" is undefined", value->string.data)
            }
        }
            break;            
        case BLINK_TYPE_F64:
            field->data.value.f64 = value->f64;
            retval = true;
            break;                
        case BLINK_TYPE_DECIMAL:
            field->data.value.decimal = value->decimal;
            retval = true;
            break;        
        case BLINK_TYPE_DYNAMIC_GROUP:
            if(BLINK_Group_hasID(value->group->definition)){

                field->data.value.group = value->group;
                retval = true;
            }
            else{

                BLINK_ERROR("expecting a Group that can be encoded dynamically")                
            }
            break;
        case BLINK_TYPE_STATIC_GROUP:
            field->data.value.group = value->group;
            retval = true;
            break;        
        default:
            break;
        }

        if(retval == true){

            field->initialised = true;
        }
    }

    return retval;    
}

static union blink_object_value BLINK_Object_get(blink_object_t group, const char *fieldName)
{
    BLINK_ASSERT(group != NULL)

    union blink_object_value retval;
    (void)memset(&retval, 0, sizeof(retval));
    struct blink_object_field *field = lookupField(group, fieldName, strlen(fieldName));

    if(field != NULL){
    
        enum blink_type_tag type = BLINK_Field_getType(field->definition);

        if(field->initialised){

            if(type == BLINK_TYPE_ENUM){

                blink_schema_t s = BLINK_Enum_getSymbolByValue(field->definition, (int32_t)field->data.value.i64);

                if(s != NULL){

                    retval.string.data = (uint8_t *)BLINK_Symbol_getName(s);                    
                }                            
            }
            else{

                retval = field->data.value;
            }
        }
    }

    return retval;    
}

static struct blink_object_field *lookupField(struct blink_object *group, const char *name, size_t nameLen)
{
    size_t i;
    struct blink_object_field *retval = NULL;

    for(i=0; i < group->numberOfFields; i++){

        const char *fname = BLINK_Field_getName(group->fields[i].definition);
        if(strcmp(fname, name) == 0){

            retval = &group->fields[i];
            break;
        }
    }

    if(i == group->numberOfFields){

        /* field not found */
        BLINK_DEBUG("field name %s does not exist for group %s", name, BLINK_Group_getName(group->definition))
    }

    return retval;
}

static size_t countFields(blink_schema_t group)
{
    size_t retval = 0U;
    size_t stackSize = BLINK_Group_numberOfSuperGroup(group) + 1U;
    blink_schema_t stack[stackSize];
    struct blink_field_iterator iter = BLINK_FieldIterator_init(stack, stackSize, group);

    while(BLINK_FieldIterator_next(&iter) != NULL){

        retval++;
    }

    return retval;
}

/* recursively walks group and calculates encoded size and determines
 * if all mandatory fields have been initialised */
static bool cacheSize(blink_object_t group)
{
    struct sequence_elem *seq;
    uint32_t i;
    group->size = 0U;
    
    for(i=0U; i < group->numberOfFields; i++){

        enum blink_type_tag type = BLINK_Field_getType(group->fields[i].definition);

        struct blink_object_field *f = &group->fields[i];

        bool isSequence = BLINK_Field_isSequence(f->definition);

        if(f->initialised){

            if(isSequence){

                group->size += BLINK_Compact_sizeofUnsigned(f->data.sequence.size);
                seq = f->data.sequence.head;
            }
            else{

                seq = NULL;
            }

            if(!isSequence || (seq != NULL)){

                do{

                    union blink_object_value *value;

                    if(isSequence){

                        value = &seq->value;
                        seq = seq->next;
                    }
                    else{

                        value = &f->data.value;
                    }
                
                    switch(type){
                    case BLINK_TYPE_STRING:            
                    case BLINK_TYPE_BINARY:
                        group->size += BLINK_Compact_sizeofUnsigned(value->string.len);
                        group->size += value->string.len;
                        break;            
                    case BLINK_TYPE_FIXED:
                        group->size += value->string.len;
                        break;            
                    case BLINK_TYPE_BOOL:
                        group->size += 1U;            
                        break;
                    case BLINK_TYPE_U8:
                    case BLINK_TYPE_U16:
                    case BLINK_TYPE_U32:
                    case BLINK_TYPE_U64:
                    case BLINK_TYPE_TIME_OF_DAY_MILLI:
                    case BLINK_TYPE_TIME_OF_DAY_NANO:
                    case BLINK_TYPE_F64:
                        group->size += BLINK_Compact_sizeofUnsigned(value->u64);                
                        break;        
                    case BLINK_TYPE_I8:        
                    case BLINK_TYPE_I16:
                    case BLINK_TYPE_I32:
                    case BLINK_TYPE_DATE:
                    case BLINK_TYPE_NANO_TIME:
                    case BLINK_TYPE_MILLI_TIME:
                    case BLINK_TYPE_I64:
                    case BLINK_TYPE_ENUM:
                        group->size += BLINK_Compact_sizeofSigned(value->i64);                
                        break;        
                    case BLINK_TYPE_DECIMAL:
                        group->size += BLINK_Compact_sizeofSigned(value->decimal.exponent);
                        group->size += BLINK_Compact_sizeofSigned(value->decimal.mantissa);
                        break;        
                    case BLINK_TYPE_DYNAMIC_GROUP:
                    case BLINK_TYPE_STATIC_GROUP:
                        if(cacheSize(value->group)){

                            group->size += value->group->size;

                            if(type == BLINK_TYPE_DYNAMIC_GROUP){

                                uint32_t sizeID = BLINK_Compact_sizeofUnsigned(BLINK_Group_getID(value->group->definition));
                                group->size += sizeID;
                                group->size += BLINK_Compact_sizeofUnsigned(sizeID + value->group->size);
                                //extensions go here
                            }                    
                        }
                        else{

                            return false;
                        }
                        break;        
                    default:
                        /* impossible */
                        break;
                    }
                }
                while(seq != NULL);
            }            
        }
        else if(BLINK_Field_isOptional(f->definition)){
            
            group->size += 1U;
        }
        else{

            BLINK_ERROR("uninitialised field")
            return false;
        }
    }

    return true;
}

static bool encodeBody(const blink_object_t g, blink_stream_t out)
{
    struct sequence_elem *seq;
    bool retval = false;
    uint32_t i;
    
    for(i=0U; i < g->numberOfFields; i++){

        struct blink_object_field *f = &g->fields[i];

        if(f->initialised){

            bool isSequence = BLINK_Field_isSequence(f->definition);
            
            if(isSequence){

                if(!BLINK_Compact_encodeU32(f->data.sequence.size, out)){

                    break;
                }

                seq = f->data.sequence.head;
            }
            else{

                seq = NULL;
            }

            if(!isSequence || (seq != NULL)){

                do{                

                    union blink_object_value *value;

                    if(isSequence){

                        value = &seq->value;
                        seq = seq->next;
                    }
                    else{

                        value = &f->data.value;
                    }

                    bool isOptional = BLINK_Field_isOptional(f->definition);
                    enum blink_type_tag type = BLINK_Field_getType(f->definition);
                    
                    switch(type){
                    case BLINK_TYPE_STRING:            
                    case BLINK_TYPE_BINARY:
                    case BLINK_TYPE_FIXED:

                        if(type == BLINK_TYPE_FIXED){

                            if(isOptional){

                                if(!BLINK_Compact_encodePresent(out)){

                                    return NULL;
                                }
                            }
                        }
                        else{

                            if(!BLINK_Compact_encodeU32(value->string.len, out)){

                                return NULL;
                            }
                        }

                        if(!BLINK_Stream_write(out, value->string.data, value->string.len)){

                            return NULL;
                        }                    
                        break;
        
                    case BLINK_TYPE_BOOL:

                        if(!BLINK_Compact_encodeBool(value->boolean, out)){

                            return NULL;
                        }
                        break;

                    case BLINK_TYPE_U8:
                    case BLINK_TYPE_U16:
                    case BLINK_TYPE_U32:
                    case BLINK_TYPE_TIME_OF_DAY_MILLI:
                    case BLINK_TYPE_U64:            
                    case BLINK_TYPE_TIME_OF_DAY_NANO:

                        if(!BLINK_Compact_encodeU64(value->u64, out)){

                            return NULL;
                        }
                        break;
                        
                    case BLINK_TYPE_I8:        
                    case BLINK_TYPE_I16:
                    case BLINK_TYPE_I32:
                    case BLINK_TYPE_DATE:
                    case BLINK_TYPE_NANO_TIME:
                    case BLINK_TYPE_MILLI_TIME:
                    case BLINK_TYPE_I64:
                    case BLINK_TYPE_ENUM:
                
                        if(!BLINK_Compact_encodeI64(value->i64, out)){

                            return NULL;
                        }
                        break;

                    case BLINK_TYPE_F64:

                        if(!BLINK_Compact_encodeF64(value->f64, out)){

                            return NULL;
                        }
                        break;

                    case BLINK_TYPE_DECIMAL:

                        if(!BLINK_Compact_encodeDecimal(value->decimal.mantissa, value->decimal.exponent, out)){

                            return NULL;
                        }
                        break;

                    case BLINK_TYPE_STATIC_GROUP:

                        if(isOptional){

                            if(!BLINK_Compact_encodePresent(out)){

                                return NULL;
                            }
                        }
                    
                        if(!encodeBody(value->group, out)){

                            return NULL;
                        }
                        break;
                    
                    case BLINK_TYPE_DYNAMIC_GROUP:
                    
                        if(!BLINK_Compact_encodeU32(value->group->size, out)){

                            return NULL;
                        }

                        if(!BLINK_Compact_encodeU64(BLINK_Group_getID(value->group->definition), out)){

                            return NULL;
                        }

                        if(!encodeBody(value->group, out)){

                            return NULL;
                        }

                        //we aren't doing extensions
                        break;
                    
                    default:
                        /* impossible */
                        break;
                    }
                }
                while(seq != NULL);
            }
        }
        else{

            if(!BLINK_Compact_encodeNull(out)){

                break;
            }
        }
    }

    return retval;
}
