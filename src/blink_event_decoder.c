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

#include "blink_parser.h"
#include "blink_compact_form.h"
#include "blink_event_decoder.h"
#include "blink_debug.h"

#include <string.h>

/* definitions ********************************************************/

#ifndef BLINK_NEST_DEPTH
    #define BLINK_NEST_DEPTH    10U
#else
    #if BLINK_NEST_DEPTH < 1
        #error "BLINK_NEST_DEPTH must be greater than 0"
    #endif
#endif

/* static function prototypes *****************************************/

static uint32_t decode(const struct blink_decoder *self, const uint8_t *in, uint32_t inLen);
static uint32_t decodeGroupHeader(const uint8_t *in, uint32_t inLen, uint64_t *id, const uint8_t **fields, uint32_t *fieldsLen, bool *isNull);

/* functions **********************************************************/

struct blink_decoder *BLINK_InitDecoder(struct blink_decoder *decoder, void *user, const struct blink_schema *schema, const struct blink_decoder_events *events)
{
    BLINK_ASSERT(self != NULL)
    BLINK_ASSERT(schema != NULL)

    (void)memset(self, 0, sizeof(*self));

    self->schema = schema;
    self->user = user;
    
    if(events != NULL){

        self->events = *events; /* copy */
    }

    return self;
}

uint32_t BLINK_DecodeCompact(const struct blink_decoder *self, const uint8_t *in, uint32_t inLen)
{
    return decode(self, in, inLen);
}

/* static functions ***************************************************/

static uint32_t decode(const struct blink_decoder *self, const uint8_t *in, uint32_t inLen)
{
    BLINK_ASSERT(self != NULL)
    BLINK_ASSERT(in != NULL)

    struct blink_decoder_state state;

    struct stack_frame {
        const uint8_t *in;
        uint32_t inLen;
        uint32_t pos;
        const struct blink_group *g;    /* current group */
        struct blink_field_iterator iter; 
        const struct blink_field *f;    /* current field */
        uint32_t sequenceSize;     
        uint32_t sequenceCount;
        bool isSequence;
    };

    uint32_t retval = 0U;
    uint32_t ret;
    uint32_t pos = 0U;
    bool isNull;            
    uint64_t id;
    struct stack_frame stack[BLINK_NEST_DEPTH];    
    size_t depth = 0U;

    (void)memset(stack, 0, sizeof(stack));

    stack[depth].in = in;
    stack[depth].inLen = inLen;

    (void)memset(&state, 0, sizeof(state));

    ret = decodeGroupHeader(in, inLen, &id, &s->in, s->inLen, &isNull);

    if(ret == 0U){
        return 0U;
    }

    s->pos += ret;

    s->g = BLINK_GetGroupByID(self->schema, id);

    if(s->g == NULL){
        BLINK_ERROR("W2: ID is unknown")
        return 0U;
    }

    BLINK_InitFieldIterator(&s->iter, s->g);

    s->f = BLINK_NextField(stack[depth].iter);

    while(s->f != NULL){

        if(self->events.beginField != NULL){

            size_t nameLen;
            self->events.beginField(self->user, BLINK_GetFieldName(s->f, &nameLen), nameLen, BLINK_GetFieldIsOptional(s->f));
        }

        isOptional = BLINK_GetFieldIsOptional(s->f);
        s->isSequence = BLINK_GetFieldisSequence(s->f);

        s->sequenceCount = 0U;
        
        if(s->isSequence){

            ret = BLINK_DecodeU32(&s->in[s->pos], s->inLen - s->pos, &s->sequenceSize, &isNull);

            if(ret == 0U){
                return 0U;
            }

            s->pos += ret;

            if(isNull){
                
                if(!isOptional){

                    BLINK_ERROR("W5: sequence cannot be NULL")
                    return 0U;
                }
                else{
                    
                    s->sequenceSize = 0U;
                }
            }
        }
        else{

            s->sequenceSize = 0U;
        }

        while(s->sequenceCount < s->sequenceSize){

            switch(BLINK_GetFieldType(s->f)){
            case BLINK_TYPE_STRING:
            {
                uint8_t *value;
                uint32_t valueLen;

                ret = BLINK_DecodeString(&s->in[s->pos], s->inLen - s->pos, &value, &valueLen, &isNull);
    
                if(ret > 0U){
                    
                    if(isNull && (!isOptional || !){

                        BLINK_ERROR("W5: string cannot be NULL")
                        return 0U;
                    }
                    else if(valueLen > BLINK_GetFieldSize(s->f)){

                        BLINK_ERROR("too big")
                        return 0U;
                    }
                    else{

                        if(!isNull && (events->fnString != NULL)){

                            self->events.fnString(self->user, value, valueLen);
                        }

                        s->pos += ret;
                    }
                }            
            }            
                break;
            case BLINK_TYPE_BINARY:
            {
                uint8_t *value;
                uint32_t valueLen;

                ret = BLINK_DecodeBinary(&s->in[s->pos], s->inLen - s->pos, &value, &valueLen, &isNull);
    
                if(ret > 0U){
                    
                    if(isNull && !isOptional){

                        BLINK_ERROR("W5: binary cannot be NULL")
                        return 0U;
                    }
                    else if(valueLen > BLINK_GetFieldSize(s->f)){

                        BLINK_ERROR("too big")
                        return 0U;
                    }
                    else{

                        if(!isNull && (events->binary != NULL)){

                            self->events.binary(self->user, value, valueLen);
                        }

                        s->pos += ret;
                    }
                }            
            }            
                break;
            case BLINK_TYPE_FIXED:
            {
                uint8_t *value;
                uint32_t valueLen = BLINK_GetFieldSize(s->f);

                ret = BLINK_DecodeFixed(&s->in[s->pos], s->inLen - s->pos, &value, valueLen, &isNull);
    
                if(ret > 0U){
                    
                    if(isNull && !isOptional){

                        BLINK_ERROR("W5: binary cannot be NULL")
                        return 0U;
                    }
                    else{

                        if(!isNull && (events->fixed != NULL)){

                            self->events.fixed(self->user, value, valueLen);
                        }

                        s->pos += ret;
                    }
                }            
            }            
                break;
            case BLINK_TYPE_BOOL:
            {
                bool value;
                
                ret = BLINK_DecodeBool(&s->in[s->pos], s->inLen - s->pos, &value, &isNull);

                if(ret > 0U){
                    
                    if(isNull && !isOptional){

                        BLINK_ERROR("W5: bool cannot be NULL")
                        return 0U;           
                    }
                    else{

                        if(self->events.bool != NULL){

                            self->events.bool(self->user, value);
                        }

                        s->pos += ret;
                    }
                }
            }
                break;
            case BLINK_TYPE_DECIMAL:
            {
                int64_t mantissa;
                int8_t exponent;

                ret = BLINK_DecodeDecimal(&s->in[s->pos], s->inLen - s->pos, &mantissa, &exponent, &isNull);
                
                if(ret > 0U){
                    
                    if(isNull && !isOptional){

                        BLINK_ERROR("W5: decimal cannot be NULL")
                        return 0U;           
                    }
                    else{

                        if(self->events.decimal != NULL){

                            self->events.decimal(self->user, mantissa, exponent);
                        }

                        s->pos += ret;
                    }
                }
            }
                break;                
            case BLINK_TYPE_U8:
            {
                uint8_t value;

                ret = BLINK_DecodeU8(&s->in[s->pos], s->inLen - s->pos, &value, &isNull);
                
                if(ret > 0U){

                    if(isNull && !isOptional){

                        BLINK_ERROR("W5: u8 cannot be NULL")
                        return 0U;
                    }
                    else{

                        if(events->u8 != NULL){

                            self->events.u8(self->user, value);
                        }

                        pos = ret;
                    }
                }
            }
                break;                
            case BLINK_TYPE_U16:
            {
                uint16_t value;

                ret = BLINK_DecodeU16(&s->in[s->pos], s->inLen - s->pos, &value, &isNull);
                
                if(ret > 0U){

                    if(isNull && !isOptional){

                        BLINK_ERROR("W5: u16 cannot be NULL")
                        return 0U;  
                    }
                    else{

                        if(events->u16 != NULL){

                            self->events.u16(self->user, value);
                        }

                        s->pos += ret;
                    }
                }
            }
                break;                                
            case BLINK_TYPE_U32:
            {
                uint32_t value;

                ret = BLINK_DecodeU16(&s->in[s->pos], s->inLen - s->pos, &value, &isNull);
                
                if(ret > 0U){

                    if(isNull && !isOptional){

                        BLINK_ERROR("W5: u16 cannot be NULL")
                        return 0U;
                    }
                    else{

                        if(events->u32 != NULL){

                            self->events.u32(self->user, value);
                        }

                        s->pos += ret;
                    }
                }
            }
                break;            
            case BLINK_TYPE_U64:
            {
                uint64_t value;

                ret = BLINK_DecodeU16(&s->in[s->pos], s->inLen - s->pos, &value, &isNull);
                
                if(ret > 0U){

                    if(isNull && !isOptional){

                        BLINK_ERROR("W5: u16 cannot be NULL")
                        return 0U;
                    }
                    else{

                        if(events->u8 != NULL){

                            self->events.u64(self->user, value);
                        }

                        s->pos += ret;
                    }
                }
            }
                break;                
            case BLINK_TYPE_I8:
            {
                int8_t value;

                ret = BLINK_DecodeU16(&s->in[s->pos], s->inLen - s->pos, &value, &isNull);
                
                if(ret > 0U){

                    if(isNull && !isOptional){

                        BLINK_ERROR("W5: u16 cannot be NULL")
                        return 0U;
                    }
                    else{

                        if(events->i8 != NULL){

                            self->events.i8(self->user, value);
                        }

                        s->pos += ret;
                    }
                }
            }
                break;            
            case BLINK_TYPE_I16:
            {
                int16_t value;

                ret = BLINK_DecodeU16(&s->in[s->pos], s->inLen - s->pos, &value, &isNull);
                
                if(ret > 0U){

                    if(isNull && !isOptional){

                        BLINK_ERROR("W5: u16 cannot be NULL")
                        return 0U;
                    }
                    else{

                        if(events->i16 != NULL){

                            self->events.i16(self->user, value);
                        }

                        s->pos += ret;
                    }
                }
            }
                break;            
            case BLINK_TYPE_I32:
            {
                int32_t value;

                ret = BLINK_DecodeU16(&s->in[s->pos], s->inLen - s->pos, &value, &isNull);
                
                if(ret > 0U){

                    if(isNull && !isOptional){

                        BLINK_ERROR("W5: u16 cannot be NULL")
                        return 0U;
                    }
                    else{

                        if(events->i32 != NULL){

                            self->events.i32(self->user, value);
                        }

                        s->pos += ret;
                    }
                }
            }
                break;            
            case BLINK_TYPE_I64:
            {
                int64_t value;

                ret = BLINK_DecodeU16(&s->in[s->pos], s->inLen - s->pos, &value, &isNull);
                
                if(ret > 0U){

                    if(isNull && !isOptional){

                        BLINK_ERROR("W5: u16 cannot be NULL")
                        return 0U;
                    }
                    else{

                        if(events->i64 != NULL){

                            self->events.i64(self->user, value);
                        }

                        s->pos += ret;
                    }
                }
            }
                break;            
            case BLINK_TYPE_F64:
            {
                double value;

                ret = BLINK_DecodeF64(&s->in[s->pos], s->inLen - s->pos, &value, &isNull);
                
                if(ret > 0U){

                    if(isNull && !isOptional){

                        BLINK_ERROR("W5: f64 cannot be NULL")
                        return 0U; 
                    }
                    else{

                        if(events->f64 != NULL){

                            self->events.f64(self->user, value);
                        }

                        s->pos += ret;
                    }
                }
            }
                break;
            case BLINK_TYPE_ENUM:
            {
                int32_t value;
                
                ret = BLINK_DecodeI32(&s->in[s->pos], s->inLen - s->pos, &value, &isNull);
                
                if(ret > 0U){

                    if(isNull && !optional){

                        BLINK_ERROR("W5: enum cannot be NULL")
                        return 0U;
                    }
                    else{

                        s->pos += ret;

                        if(!isNull){

                            const char *name;
                            size_t nameLen;

                            if(BLINK_GetSymbolName(f->value.e, value, &name, &nameLen) != NULL){

                                if(self->events.fnEnum != NULL){

                                    self->events.fnEnum(self->user, name, nameLen);
                                }
                            }
                            else{

                                BLINK_ERROR("W10: value does not correspond to any symbol")
                                ret = 0U;
                            }
                        }                        
                    }
                }
            }
                break;                
            case BLINK_TYPE_DATE:
            {
                int32_t value;

                ret = BLINK_DecodeI32(&s->in[s->pos], s->inLen - s->pos, &value, &isNull);
                
                if(ret > 0U){

                    if(isNull && !isOptional){

                        BLINK_ERROR("W5: date cannot be NULL")
                        return 0U;
                    }
                    else{

                        if(events->date != NULL){

                            self->events.date(self->user, value);
                        }

                        s->pos += ret;
                    }
                }  
            }
                break;     
            case BLINK_TYPE_MILLI_TIME:
            {
                int64_t value;
                
                ret = BLINK_DecodeI64(&s->in[s->pos], s->inLen - s->pos, &value, &isNull);
                
                if(ret > 0U){

                    if(isNull && !isOptional){

                        BLINK_ERROR("W5: millitime cannot be NULL")
                        return 0U;     
                    }
                    else{

                        if(events->millitime != NULL){

                            self->events.millitime(self->user, value);
                        }

                        s->pos += ret;
                    }
                }
            }
                break;     
            case BLINK_TYPE_NANO_TIME:
            {
                int64_t value;
                
                ret = BLINK_DecodeI64(&s->in[s->pos], s->inLen - s->pos, &value, &isNull);
                
                if(ret > 0U){

                    if(isNull && !isOptional){

                        BLINK_ERROR("W5: nanotime cannot be NULL")
                        return 0U; 
                    }
                    else{

                        if(events->nanotime != NULL){

                            self->events.nanotime(self->user, value);
                        }

                        s->pos += ret;
                    }
                }
            }
                break;     
            case BLINK_TYPE_TIME_OF_DAY_MILLI:
            {
                uint32_t value;
                
                ret = BLINK_DecodeU32(&s->in[s->pos], s->inLen - s->pos, &value, &isNull);
                
                if(ret > 0U){

                    if(isNull && !isOptional){

                        BLINK_ERROR("W5: timeOfDayNano cannot be NULL")
                        return 0U;
                    }
                    else{

                        if(events->timeOfDayMilli != NULL){

                            self->events.timeOfDayMilli(self->user, value);
                        }

                        s->pos += ret;
                    }
                }
            }
                break;     
            case BLINK_TYPE_TIME_OF_DAY_NANO:
            {
                uint64_t value;
                
                ret = BLINK_DecodeU64(&s->in[s->pos], s->inLen - s->pos, &value, &isNull);
                
                if(ret > 0U){

                    if(isNull && !isOptional){

                        BLINK_ERROR("W5: timeOfDayNano cannot be NULL")
                        return 0U;
                    }
                    else{

                        if(events->timeOfDayNano != NULL){

                            self->events.timeOfDayNano(self->user, value);
                        }

                        s->pos += ret;
                    }
                }
            }
                break;                
            case BLINK_TYPE_OBJECT:
            case BLINK_TYPE_DYNAMIC_GROUP:
            {
                const uint8_t *value;
                uint32_t valueLen;

                ret = decodeGroupHeader(&s->in[pos], s->inLen - s->pos, &id, &value, &valueLen, &isNull);

                if(ret > 0U){

                    s->pos += ret;

                    if(isNull && !isOptional){

                        BLINK_ERROR("W5: object cannot be NULL")
                        return 0U;               
                    }
                    else{

                        if(!isNull){

                            if(depth == ((sizeof(stack)/sizeof(*stack))-1U)){

                                BLINK_ERROR("too deep!")
                                return 0U;
                            }

                            depth++;
                            s = &stack[depth];
                            s->pos = 0U;
                            
                            s->in = value;
                            s->inLen = valueLen;

                            s->g = BLINK_GetGroupByID(self->schema, id);

                            if(s->g == NULL){
                                BLINK_ERROR("W2: ID is unknown")
                                return 0U;
                            }

                            if(s->type == BLINK_TYPE_DYNAMIC_GROUP){

                                //test that group is OK
                            }

                            BLINK_InitFieldIterator(&s->iter, s->g);
                            s->f = BLINK_NextField(stack[depth].iter);                        
                        }
                    }
                }
            }
                break;            
            case BLINK_TYPE_STATIC_GROUP:
            {
                bool present = true;

                if(isOptional){

                    ret = BLINK_DecodePresent(&s->in[s->pos], s->inLen - s->pos, &present);

                    if(ret < 0U){

                        return 0U;
                    }

                    s->pos += ret;
                }

                if(present){

                    if(depth == ((sizeof(stack)/sizeof(*stack))-1U)){

                        BLINK_ERROR("too deep!")
                        return 0U;
                    }
                    
                    depth++;
                    s = &stack[depth];
                    s->pos = 0U;

                    s->g = BLINK_GetFieldGroup(stack[depth-1U].f);
                    BLINK_InitFieldIterator(&s->iter, s->g);
                    s->f = BLINK_NextField(s->iter);
                }
            }
                break;            
            default:
                break;                                    
            }

            s->sequenceCount++;
        }

        if(ret == 0U){
            return 0U;
        }
        
        if(self->events.endField != NULL){

            size_t nameLen;
            self->events.endField(self->user, BLINK_GetFieldName(s->f, &nameLen), nameLen);
        }

        /* next field? */
        do{

            s->f = BLINK_NextField(s->iter);

            if(s->f != NULL){
                break;
            }
            else{
                        
                if(s->type == BLINK_TYPE_DYNAMIC_GROUP){

                    if(s->group[]
                    

                }

                if(depth == 0U){

                    /* no more fields or extensions, break out*/
                    break;
                }
                else{

                    /* pop stack, repeat looking for next field or extension */
                    depth--;
                    s = &stack[depth];                    
                }
            }            
        }
        while(true);            
    }
    
    return retval;
}

static uint32_t decodeGroupHeader(const uint8_t *in, uint32_t inLen, uint64_t *id, const uint8_t **fields, uint32_t *fieldsLen, bool *isNull)
{
    const uint8_t *inner;
    uint32_t innerLen;
    uint32_t retval = 0U;
    bool idIsNull;

    uint32_t ret = BLINK_DecodeBinary(in, inLen, &inner, &innerLen, isNull);

    if(ret > 0U){

        pos += ret;

        if(!isNull){

            if(innerLen != 0U){

                ret = BLINK_DecodeU64(inner, innerLen, id, &idIsNull);

                if(ret > 0U){

                    pos += ret;

                    *fields = inner[ret];
                    *fieldsLen = innerLen
                }
                else{

                    BLINK_ERROR("Group ID cannot be NULL")
                }
            }
            else{

                BLINK_ERROR("W1: Group cannot have size of zero")
            }
        }
        else{
            
            retval = ret;
        }
    }

    return retval;
}
