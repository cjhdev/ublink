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

struct blink_object_sequence {  
    union blink_object_value value;
    struct blink_object_sequence *next;
};

struct blink_object_field {
    uint32_t size;
    blink_schema_t definition;           /**< field definition */
    union {
        union blink_object_value value;
        struct {
            struct blink_object_sequence *head;
            struct blink_object_sequence *tail;
            uint32_t size;
        } sequence;
    } data;                         /**< field data may be singular or sequence */
    bool initialised;               /**< true if data has been initialised */
};

struct blink_object {
    uint32_t size;                      /** encoded size cache */
    struct blink_allocator a;
    blink_schema_t definition;          /**< group definition */    
    struct blink_object_field *fields;  /**< array of fields */
    size_t numberOfFields;
};

/* static function prototypes *****************************************/

static struct blink_object_field *lookupField(struct blink_object *group, const char *name, size_t nameLen);
static size_t countFields(blink_schema_t group);
static bool encodeBody(const blink_object_t g, blink_stream_t out);
static bool cacheSize(blink_object_t group);

/* functions **********************************************************/

blink_object_t BLINK_Object_newGroup(const struct blink_allocator *alloc, blink_schema_t group)
{
    BLINK_ASSERT(group != NULL)

    blink_object_t retval = NULL;

    if(alloc->calloc != NULL){
    
        struct blink_object *self = alloc->calloc(1U, sizeof(struct blink_object));

        if(self != NULL){

            self->a = *alloc;
            self->definition = group;
            self->numberOfFields = countFields(group);
            self->fields = self->a.calloc(self->numberOfFields, sizeof(struct blink_object_field));

            if(self->fields != NULL){

                size_t stackDepth = BLINK_Group_numberOfSuperGroup(group) + 1U;
                blink_schema_t stack[stackDepth];
                struct blink_field_iterator iter = BLINK_FieldIterator_init(stack, stackDepth, group);
                blink_schema_t f = BLINK_FieldIterator_next(&iter);
                uint32_t i = 0U;

                while(f != NULL){
                
                    self->fields[i].definition = f;
                    i++;
                }

                retval = (blink_object_t)self;
            }
        }
    }

    return retval;    
}

#if 0
blink_object_t BLINK_Object_decodeCompact(blink_stream_t in, blink_schema_t schema, struct blink_allocator *alloc)
{
    blink_object_t retval = NULL;
    blink_schema_t definition;

    struct stack_element {

        uint32_t i;
        uint32_t size;
        blink_group_t g;

    } stack[BLINK_OBJECT_NEST_DEPTH];

    uint8_t depth = 0U;

    struct stack_element *s = &stack[depth];

    struct blink_stream stream;

    if(!BLINK_Compact_decodeU32(in, &size, &isNull)){

        return NULL;
    }

    if(isNull){

        return NULL;
    }

    if(size == 0U){

        return NULL;
    }

    if(!BLINK_Stream_initBounded(&stream, in, size)){

        return NULL;
    }

    if(!BLINK_Compact_decodeU64(&stream, &id, &isNull)){

        return NULL;
    }

    if(isNull){

        return NULL;
    }

    definition = BLINK_Schema_getGroupByID(schema, id);

    if(definition == NULL){

        BLINK_ERROR("W1: unknown group ID")
        return NULL;
    }

    retval = BLINK_Object_newGroup(alloc, definition);

    if(retval == NULL){

        return NULL;
    }

    s->i = 0U;
    s->g = retval;

    while(true){

        if(s->event == NEXT_FIELD_DEFINITION){
    
            while(true){

                
            }    
        }
        else{

            struct blink_object_field *f = &s->g->fields[i];
            bool optional = BLINK_Field_isOptional(f->definition);

            switch(BLINK_Field_getType(f->definition)){
            case BLINK_TYPE_FIXED:
            {
                bool present;
                
                if(optional){

                    if(!BLINK_Compact_decodePresent(in, &present)){

                        event = EVENT_EOF_OR_ERROR;
                        present = false;
                    }
                }
                else{

                    present = true;
                }

                if(present){

                    size = BLINK_Field_getSize(f->definition);
                    
                    uint8_t *data = alloc->calloc(1, size);

                    if(BLINK_Stream_read(&stream, data, size)){

                        f->initialised = true;                        
                        f->data.value.string.data = data;                        
                        f->data.value.string.len = size;                        
                    }
                    else{

                        f->data.value.string.data = data;

                        event = EVENT_EOF_OR_ERROR;
                    }
                }
            }
                break;

            case BLINK_TYPE_STRING:            
            case BLINK_TYPE_BINARY:
            
                if(BLINK_Compact_decodeU32(&stream, &size, &isNull)){

                    if(isNull){

                        if(!optional){

                            event = EVENT_MANDATORY_MISSING;
                        }
                    }
                    else if(size == 0U){

                        s->g->fields[s->i].initialised = true;                        
                    }
                    else{
                        
                        uint8_t *data = alloc->calloc(1, size);

                        if(data != NULL){
                        
                            if(BLINK_Stream_read(&stream, data, size)){

                                f->initialised = true;                        
                                f->data.value.string.data = data;                        
                                f->data.value.string.len = size;                        
                            }
                            else{

                                f->data.value.string.data = data;

                                event = EVENT_EOF_OR_ERROR;
                            }
                        }
                    }
                }
                else{

                    event = EVENT_EOF_OR_ERROR;
                }
                break;
            
            case BLINK_TYPE_BOOL:
            {
                bool value;
                if(BLINK_Compact_decodeBool(&stream, &value, &isNULL)){

                    if(isNull){

                        if(!optional){

                            event = EVENT_MANDATORY_MISSING;
                        }
                    }
                    else{

                        f->initialised = true;
                        f->data.value.boolean = value;
                    }                                                        
                }
                else{

                    event = EVENT_EOF_OR_ERROR;
                }
            }
                break;
                
            case BLINK_TYPE_U8:
            {
                uint8_t value;
                if(BLINK_Compact_decodeU8(&stream, &value, &isNull)){

                    if(isNull){

                        if(!optional){

                            event = EVENT_MANDATORY_MISSING;
                        }
                    }
                    else{
            
                        f->initialised = true;
                        f->data.value.u64 = (uint64_t)value;
                    }                    
                }
                else{

                    event = EVENT_EOF_OR_ERROR;
                }
            }
                break;
                
            case BLINK_TYPE_U16:
            {
                uint16_t value;
                if(BLINK_Compact_decodeU16(&stream, &value, &isNull)){

                    if(isNull){

                        if(!optional){

                            event = EVENT_MANDATORY_MISSING;
                        }
                    }
                    else{
            
                        f->initialised = true;
                        f->data.value.u64 = (uint64_t)value;
                    }                    
                }
                else{

                    event = EVENT_EOF_OR_ERROR;
                }
            }
                break;
                
            case BLINK_TYPE_U32:
            case BLINK_TYPE_TIME_OF_DAY_MILLI:
            {
                uint32_t value;
                if(BLINK_Compact_decodeU32(&stream, &value, &isNull)){

                    if(isNull){

                        if(!optional){

                            event = EVENT_MANDATORY_MISSING;
                        }
                    }
                    else{
            
                        f->initialised = true;
                        f->data.value.u64 = (uint64_t)value;
                    }                    
                }
                else{

                    event = EVENT_EOF_OR_ERROR;
                }
            }
                break;
            
            case BLINK_TYPE_U64:            
            case BLINK_TYPE_TIME_OF_DAY_NANO:
            {
                uint64_t value;
                if(BLINK_Compact_decodeU64(&stream, &value, &isNull)){

                    if(isNull){

                        if(!optional){

                            event = EVENT_MANDATORY_MISSING;
                        }
                    }
                    else{
            
                        f->initialised = true;
                        f->data.value.u64 = value;
                    }                    
                }
                else{

                    event = EVENT_EOF_OR_ERROR;
                }
            }
                break;
            
            case BLINK_TYPE_I8:
            {
                int8_t value;
                if(BLINK_Compact_decodeI8(&stream, &value, &isNull)){

                    if(isNull){

                        if(!optional){

                            event = EVENT_MANDATORY_MISSING;
                        }
                    }
                    else{
            
                        f->initialised = true;
                        f->data.value.i64 = (int64_t)value;
                    }                    
                }
                else{

                    event = EVENT_EOF_OR_ERROR;
                }
            }
                break;
                
            case BLINK_TYPE_I16:
            {
                int16_t value;
                if(BLINK_Compact_decodeI16(&stream, &value, &isNull)){

                    if(isNull){

                        if(!optional){

                            event = EVENT_MANDATORY_MISSING;
                        }
                    }
                    else{
            
                        f->initialised = true;
                        f->data.value.i64 = (int64_t)value;
                    }                    
                }
                else{

                    event = EVENT_EOF_OR_ERROR;
                }
            }
                break;
                
            case BLINK_TYPE_I32:            
            case BLINK_TYPE_DATE:
            {
                int32_t value;
                if(BLINK_Compact_decodeI32(&stream, &value, &isNull)){

                    if(isNull){

                        if(!optional){

                            event = EVENT_MANDATORY_MISSING;
                            
                        }
                    }
                    else{
            
                        f->initialised = true;
                        f->data.value.i64 = (int64_t)value;
                    }                    
                }
                else{

                    event = EVENT_EOF_OR_ERROR;
                }
            }
                break;
            
            case BLINK_TYPE_NANO_TIME:
            case BLINK_TYPE_MILLI_TIME:
            case BLINK_TYPE_I64:
            {
                int64_t value;
                if(BLINK_Compact_decodeI64(&stream, &value, &isNull)){

                    if(isNull){

                        if(!optional){

                            event = EVENT_MANDATORY_MISSING;
                        }
                    }
                    else{
            
                        f->initialised = true;
                        f->data.value.i64 = value;
                    }                    
                }
                else{

                    event = EVENT_EOF_OR_ERROR;
                }
            }
                break;
                        
            case BLINK_TYPE_ENUM:
            {
                int32_t value;
                if(BLINK_Compact_decodeI32(&stream, &value, &isNull)){

                    if(isNull){

                        if(!optional){

                            event = EVENT_MANDATORY_MISSING;
                        }
                    }
                    else{

                        blink_schema_t s = BLINK_Enum_getSymbolByValue(field->definition, value->enumeration);
                        
                        if(s != NULL){

                            value->enumeration = BLINK_Symbol_getName(s);
                            f->initialised = true;
                        }
                        else{

                            BLINK_ERROR("symbol not found in enum")
                        }                        
                    }                    
                }
                else{

                    event = EVENT_EOF_OR_ERROR;
                }
            }            
                break;
                
            case BLINK_TYPE_F64:
            {
                double value;
                if(BLINK_Compact_decodeF64(&stream, &value, &isNull)){

                    if(isNull){

                        if(!optional){

                            event = EVENT_MANDATORY_MISSING;
                        }
                    }
                    else{

                        f->initialised = true;
                        f->data.value.f64 = value;
                    }
                }
                else{

                    event = EVENT_EOF_OR_ERROR;
                }
            }
                break;
                    
            case BLINK_TYPE_DECIMAL:
            {
                int64_t mantissa;
                int8_t exponent;
                if(BLINK_Compact_decodeDecimal(&stream, &mantissa, &exponent, &isNull)){

                    if(isNull){

                        if(!optional){

                            event = EVENT_MANDATORY_MISSING;
                        }
                    }
                    else{

                        f->initialised = true;
                        f->data.value.decimal.mantissa = mantissa;
                        f->data.value.decimal.exponent = exponent;
                    }
                }
                else{

                    event = EVENT_EOF_OR_ERROR;
                }
            }
                break;

            case BLINK_TYPE_STATIC_GROUP:
            {
                bool present;
                
                if(optional){

                    if(!BLINK_Compact_decodePresent(in, &present)){

                        event = EVENT_EOF_OR_ERROR;
                        present = false;
                    }
                }
                else{

                    present = true;
                }

                if(present){

                    if(depth == ((sizeof(stack)/sizeof(*stack))-1U)){

                        s->event = EVENT_NEST_ERROR;
                    }
                    else{
                    
                        depth++;
                        s = &stack[depth];
                        s->i = 0U;
                        s->g = BLINK_Field_getGroup(stack[depth-1U].f);
                        s->event = NEXT_FIELD_DEFINITION;
                    }
                }                
            }
                break;
                
            case BLINK_TYPE_OBJECT:
            case BLINK_TYPE_DYNAMIC_GROUP:
            {
                if(BLINK_Compact_decodeU32(in, &size, &isNull)){

                   if(isNull){

                        if(!optional){

                            
                        }
                   }
                   else{

                        if(size == 0U){

                            BLINK_ERROR("W1: Group cannot have size of zero")
                        }
                        else{

                            if((s->size - (uint32_t)BLINK_Stream_tell(&stream)) >= size){

                                (void)BLINK_Stream_initBounded(&stream, size);

                                if(depth == ((sizeof(stack)/sizeof(*stack))-1U)){

                                    BLINK_ERROR("too deep!")
                                    event = EVENT_NEST_DEPTH;
                                }
                                else{

                                    depth++;
                                    s = &stack[depth];
                                    s->size = size;

                                    if(BLINK_Compact_decodeU64(in, &id, &isNull)){

                                        if(isNull){

                                            BLINK_ERROR("W14: ID is null (and therefore unknown)")
                                        }
                                        else{

                                            definition = BLINK_Schema_getGroupByID(schema, id);

                                            if(definition == NULL){

                                                BLINK_ERROR("W14: ID is unknown")                            
                                            }
                                            else{

                                                s->g = BLINK_Object_newGroup(alloc, definition);

                                                if(s->g == NULL){

                                                    //destroy everything?
                                                }
                                                else{

                                                    //next field?
                                                }
                                            }
                                        }
                                    }
                                    else{

                                        event = EVENT_EOF_OR_ERROR;
                                    }
                                }
                            }
                            else{

                                BLINK_ERROR("S1: nested group will overrun parent group")
                                event = EVENT_INNER_OVERRUN;
                            }
                        }                    
                   }
                }
                else{

                    event = EVENT_EOF_OR_ERROR;
                }
                break;
                
            default:
                break;
            }

            switch(s->event){
            case EVENT_NEST_DEPTH:

                BLINK_ERROR("too many nested groups!")
                return NULL;
                
            case EVENT_EOF_OR_ERROR:
        
                if(BLINK_Stream_eof(&stream)){

                    BLINK_ERROR("S1: nested group ended prematurely")
                }
                else if(BLINK_Stream_eof(in)){

                    BLINK_ERROR("S1: group ended prematurely")
                }
                else{

                    BLINK_ERROR("encoding error")
                }
                event = 
                break;

            case EVENT_MANDATORY_MISSING:

                BLINK_ERROR("W5: field \"%s\" may not be null", BLINK_Field_getName(f->definition))
                break;
                
            case NEXT_SEQUENCE_VALUE:
            case NEXT_EXTENSION_VALUE:
                s->sequenceCount++;
                if(s->sequenceCount == s->sequenceSize){
            
                    //raise event end-of-sequence
                    //raise event end-of-field

                    if(s->event == NEXT_EXTENSION_VALUE){

                        
                        

                        if((s->inLen - s->pos) > 0U){

                            BLINK_ERROR("extra bytes after sequence but before end of group")
                            return 0U;                            
                        }
                    }

                    s->event = NEXT_FIELD_DEFINITION;
                }
                break;
            case NEXT_FIELD_VALUE:
                //raise end-of-field
                break;
            default:
                /* nothing to be done */
                break;
            }
            

        }
    }
    
    return retval;
}
#endif

bool BLINK_Object_encodeCompact(blink_object_t group, blink_stream_t out)
{
    bool retval = false;

    if(BLINK_Group_hasID(group->definition)){

        if(cacheSize(group)){

            if(BLINK_Compact_encodeU32(group->size, out)){

                if(BLINK_Compact_encodeU64(BLINK_Group_getID(group->definition), out)){

                    retval = encodeBody(group, out);
                }
            }
        }
    }
    else{

        BLINK_ERROR("cannot encode group without and ID")
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

    struct blink_object_sequence *seq;
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
    
bool BLINK_Object_set(blink_object_t group, const char *fieldName, const union blink_object_value *value)
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

                uint8_t *data = group->a.calloc(1, value->string.len);

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

                uint8_t *data = group->a.calloc(1, value->string.len);

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
            blink_schema_t e = BLINK_Enum_getSymbolByName(field->definition, value->enumeration);

            if(e != NULL){

                field->data.value.i64 = (int64_t)BLINK_Symbol_getValue(e);
                retval = true;
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
    }

    return retval;    
}

union blink_object_value BLINK_Object_get(blink_object_t group, const char *fieldName)
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

                    retval.enumeration = BLINK_Symbol_getName(s);
                }                            
            }
            else{

                retval = field->data.value;
            }
        }
    }

    return retval;    
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

bool BLINK_Object_setEnum(blink_object_t group, const char *fieldName, const char *value)
{
    return BLINK_Object_set(group, fieldName, (union blink_object_value *)value);
}

bool BLINK_Object_setBool(blink_object_t group, const char *fieldName, bool value)
{
    return BLINK_Object_set(group, fieldName, (union blink_object_value *)value);    
}

bool BLINK_Object_setDecimal(blink_object_t group, const char *fieldName, struct blink_decimal *value)
{
    return BLINK_Object_set(group, fieldName, (union blink_object_value *)&value);    
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

bool BLINK_Object_setString(blink_object_t group, const char *fieldName, struct blink_string *value)
{
    return BLINK_Object_set(group, fieldName, (union blink_object_value *)value);        
}

bool BLINK_Object_setBinary(blink_object_t group, const char *fieldName, struct blink_string *value)
{
    
    return BLINK_Object_set(group, fieldName, (union blink_object_value *)value);        
}

bool BLINK_Object_setFixed(blink_object_t group, const char *fieldName, struct blink_string *value)
{
    return BLINK_Object_set(group, fieldName, (union blink_object_value *)value);        
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

struct blink_string BLINK_Object_getEnum(blink_object_t group, const char *fieldName)
{
    return BLINK_Object_get(group, fieldName).string;
}

bool BLINK_Object_getBool(blink_object_t group, const char *fieldName)
{
    return BLINK_Object_get(group, fieldName).boolean;
}

struct blink_decimal BLINK_Object_getDecimal(blink_object_t group, const char *fieldName)
{
    return BLINK_Object_get(group, fieldName).decimal;
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

struct blink_string BLINK_Object_getString(blink_object_t group, const char *fieldName)
{
    return BLINK_Object_get(group, fieldName).string;
}

struct blink_string  BLINK_Object_getBinary(blink_object_t group, const char *fieldName)
{
    return BLINK_Object_get(group, fieldName).string;
}

struct blink_string  BLINK_Object_getFixed(blink_object_t group, const char *fieldName)
{
    return BLINK_Object_get(group, fieldName).string;
}   

blink_object_t BLINK_Object_getGroup(blink_object_t group, const char *fieldName)
{
    return BLINK_Object_get(group, fieldName).group;
}

/* static functions ***************************************************/

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
    struct blink_object_sequence *seq;
    uint32_t i;
    group->size = 0U;
    
    for(i=0U; i < group->numberOfFields; i++){

        enum blink_type_tag type = BLINK_Field_getType(group->fields[i].definition);

        struct blink_object_field *f = &group->fields[i];

        bool isSequence = BLINK_Field_isSequence(f->definition);

        if(group->fields[i].initialised){

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
                        group->size += BLINK_Compact_sizeofSigned(value->i64);                
                        break;        
                    case BLINK_TYPE_ENUM:
                        group->size += BLINK_Compact_sizeofSigned(BLINK_Symbol_getValue(BLINK_Enum_getSymbolByName(BLINK_Field_getEnum(f->definition), value->enumeration)));
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
    struct blink_object_sequence *seq;
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

                    case BLINK_TYPE_ENUM:
                    {
                        blink_schema_t e = BLINK_Field_getEnum(f->definition);
                        blink_schema_t s = BLINK_Enum_getSymbolByName(e, value->enumeration);
                        if(!BLINK_Compact_encodeU32(BLINK_Symbol_getValue(s), out)){

                            return NULL;
                        }
                        break;
                    }

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
