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

/* static function prototypes *****************************************/

static uint32_t decode(const struct blink_decoder *self, const uint8_t *in, uint32_t inLen);

static uint32_t decodeString(const struct blink_decoder_events *events, const uint8_t *in, uint32_t inLen, bool isOptional, uint32_t size);
static uint32_t decodeBinary(const struct blink_decoder_events *events, const uint8_t *in, uint32_t inLen, bool isOptional, uint32_t size);
static uint32_t decodeDecimal(const struct blink_decoder_events *events, const uint8_t *in, uint32_t inLen, bool isOptional);
static uint32_t decodeBool(const struct blink_decoder_events *events, const uint8_t *in, uint32_t inLen, bool isOptional);
static uint32_t decodeU8(const struct blink_decoder_events *events, const uint8_t *in, uint32_t inLen, bool isOptional);
static uint32_t decodeU16(const struct blink_decoder_events *events, const uint8_t *in, uint32_t inLen, bool isOptional);
static uint32_t decodeU32(const struct blink_decoder_events *events, const uint8_t *in, uint32_t inLen, bool isOptional);
static uint32_t decodeU64(const struct blink_decoder_events *events, const uint8_t *in, uint32_t inLen, bool isOptional);
static uint32_t decodeI8(const struct blink_decoder_events *events, const uint8_t *in, uint32_t inLen, bool isOptional);
static uint32_t decodeI16(const struct blink_decoder_events *events, const uint8_t *in, uint32_t inLen, bool isOptional);
static uint32_t decodeI32(const struct blink_decoder_events *events, const uint8_t *in, uint32_t inLen, bool isOptional);
static uint32_t decodeI64(const struct blink_decoder_events *events, const uint8_t *in, uint32_t inLen, bool isOptional);
static uint32_t decodeF64(const struct blink_decoder_events *events, const uint8_t *in, uint32_t inLen, bool isOptional);

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
    
    struct stack_frame {
        const uint8_t *in;
        uint32_t inLen;
        const uint8_t *group;
        uint32_t groupLen;
        uint32_t pos;
        const struct blink_group *g;    /* current group */
        struct blink_field_iterator iter; 
        const struct blink_field *f;    /* current field */
        uint32_t sequenceSize;     
        uint32_t sequenceCount;
        bool isSequence;
    };
    
    uint32_t retval = 0U;
    const uint8_t *group;
    bool isNull;    
    uint32_t ret;
    uint32_t pos = 0U;
    uint64_t id;
    struct stack_frame stack[10U];
    
    uint16_t depth = 0U;

    (void)memset(stack, 0, sizeof(stack));

    stack[depth].in = in;
    stack[depth].inLen = inLen;

    ret = BLINK_DecodeGroup(s->in, s->inLen, &id, &s->group, s->groupLen, &isNull);

    if(ret == 0U){
        return 0U;
    }

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
        isSequence = BLINK_GetFieldisSequence(s->f);

        s->sequenceCount = 0U;
    
        if(isSequence){

            ret = BLINK_DecodeU32(&in[pos], inLen - pos, &s->sequenceSize, &isNull);

            if(ret == 0U){
                return 0U;
            }

            pos += ret;

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
                ret = decodeString(&self->events, &inLen[pos], inLen - pos, isOptional, BLINK_GetFieldSize(s->f));
                break;
            case BLINK_TYPE_BINARY:
                ret = decodeBinary(&self->events, &inLen[pos], inLen - pos, isOptional, BLINK_GetFieldSize(s->f));
                break;
            case BLINK_TYPE_FIXED:
                ret = decodeFixed(&self->events, &inLen[pos], inLen - pos, isOptional, BLINK_GetFieldSize(s->f));
                break;
            case BLINK_TYPE_BOOL:
                ret = decodeBool(&self->events, &inLen[pos], inLen - pos, isOptional);
                break;
            case BLINK_TYPE_DECIMAL:
                ret = decodeDecimal(&self->events, &inLen[pos], inLen - pos, isOptional);
                break;                
            case BLINK_TYPE_U8:
                ret = decodeU8(&self->events, &inLen[pos], inLen - pos, isOptional);
                break;                
            case BLINK_TYPE_U16:
                ret = decodeU16(&self->events, &inLen[pos], inLen - pos, isOptional);
                break;                
            case BLINK_TYPE_U32:
                ret = decodeU32(&self->events, &inLen[pos], inLen - pos, isOptional);
                break;                
            case BLINK_TYPE_U64:
                ret = decodeU64(&self->events, &inLen[pos], inLen - pos, isOptional);
                break;                
            case BLINK_TYPE_I8:
                ret = decodeI8(&self->events, &inLen[pos], inLen - pos, isOptional);
                break;            
            case BLINK_TYPE_I16:
                ret = decodeI16(&self->events, &inLen[pos], inLen - pos, isOptional);
                break;            
            case BLINK_TYPE_I32:
                ret = decodeI32(&self->events, &inLen[pos], inLen - pos, isOptional);
                break;            
            case BLINK_TYPE_I64:
                ret = decodeI64(&self->events, &inLen[pos], inLen - pos, isOptional);
                break;            
            case BLINK_TYPE_F64:
                ret = decodeF64(&self->events, &inLen[pos], inLen - pos, isOptional);
                break;            
            case BLINK_TYPE_ENUM:
            {
                int32_t out;
                
                ret = BLINK_DecodeI32(&in[pos], inLen - pos, &out, &isNull);
                
                if(ret > 0U){

                    if(isNull){

                        if(!isOptional){

                            BLINK_ERROR("W5: enum cannot be NULL")
                            return 0U;
                        }
                    }
                    else{

                        const char *name;
                        size_t nameLen;

                        if(BLINK_GetSymbolName(f->value.e, out, &name, &nameLen) != NULL){

                            if(self->events.fnEnum != NULL){

                                self->events.fnEnum(self->user, name, nameLen);
                            }
                        }
                        else{

                            BLINK_ERROR("W10: value does not correspond to any symbol")
                            return 0U;
                        }
                    }

                    pos += ret;
                }
            }
                break;
                
            case BLINK_TYPE_DATE:
            {
                int32_t out;
                
                ret = BLINK_DecodeI32(&in[pos], inLen - pos, &out, &isNull);
                
                if(ret > 0U){

                    if(isNull){

                        if(!isOptional){

                            BLINK_ERROR("W5: i32 cannot be NULL")
                        }
                    }
                    else{

                        if(self->events.fnDate != NULL){

                            self->events.fnDate(self->user, out);
                        }
                    }

                    pos += ret;
                }
            }
                break;
            
            case BLINK_TYPE_MILLI_TIME:
            {
                int64_t out;
                
                ret = BLINK_DecodeI64(&in[pos], inLen - pos, &out, &isNull);
                
                if(ret > 0U){

                    if(isNull){

                        if(!isOptional){

                            BLINK_ERROR("W5: i64 cannot be NULL")
                        }
                    }
                    else{

                        if(self->events.fnMilliTime != NULL){

                            self->events.fnMilliTime(self->user, out);
                        }
                    }

                    pos += ret;
                }
            }
                break;
            
            case BLINK_TYPE_NANO_TIME:
            {
                int64_t out;
                
                ret = BLINK_DecodeI64(&in[pos], inLen - pos, &out, &isNull);
                
                if(ret > 0U){

                    if(isNull){

                        if(!isOptional){

                            BLINK_ERROR("W5: i64 cannot be NULL")
                        }
                    }
                    else{

                        if(self->events.fnNanoTime != NULL){

                            self->events.fnNanoTime(self->user, out);
                        }
                    }

                    pos += ret;
                }
            }
                break;
                
            case BLINK_TYPE_TIME_OF_DAY_MILLI:
            {
                uint32_t out;
                
                ret = BLINK_DecodeU32(&in[pos], inLen - pos, &out, &isNull);
                
                if(ret > 0U){

                    if(isNull){

                        if(!isOptional){

                            BLINK_ERROR("W5: i64 cannot be NULL")
                        }
                    }
                    else{

                        if(self->events.fnTimeOfDayMilli != NULL){

                            self->events.fnTimeOfDayMilli(self->user, out);
                        }
                    }

                    pos += ret;
                }
            }
                break;
                
            case BLINK_TYPE_TIME_OF_DAY_NANO:
            {
                uint64_t out;
                
                ret = BLINK_DecodeU64(&in[pos], inLen - pos, &out, &isNull);
                
                if(ret > 0U){

                    if(isNull){

                        if(!isOptional){

                            BLINK_ERROR("W5: i64 cannot be NULL")
                        }
                    }
                    else{

                        if(self->events.fnTimeOfDayNano != NULL){

                            self->events.fnTimeOfDayNano(self->user, out);
                        }
                    }

                    pos += ret;
                }
            }
                break;
                
            case BLINK_TYPE_OBJECT:
            case BLINK_TYPE_DYNAMIC_GROUP:
            {
                if((depth+1U) == (sizeof(stack)/sizeof(*stack))){

                    BLINK_ERROR("too deep!")
                    return 0U;
                }

                depth++;
                s = &stack[depth];
            
                ret = BLINK_DecodeGroup(stack[depth-1U].in, stack[depth-1U].inLen, &id, &s->group, s->groupSize, &isNull);

                if(ret == 0U){
                    return 0U;
                }

                if(isNull){

                    if(!isOptional){

                        BLINK_ERROR("W5: object cannot be NULL")
                        return 0U;
                    }
                }
                else{

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
                break;
            
            case BLINK_TYPE_STATIC_GROUP:
            {
                if((depth+1U) == (sizeof(stack)/sizeof(*stack))){

                    BLINK_ERROR("too deep!")
                    return 0U;
                }

                depth++;
                s = &stack[depth];

                BLINK_InitFieldIterator(&s->iter, s->g);
                s->f = BLINK_NextField(stack[depth].iter);
                break;            
            }

            default:
                break;                                    
            }

            s->sequenceCount++;
        }
        
        if(self->events.endField != NULL){

            size_t nameLen;
            self->events.endField(self->user, BLINK_GetFieldName(f, &nameLen), nameLen);
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

static uint32_t decodeString(const struct blink_decoder_events *events, const uint8_t *in, uint32_t inLen, bool isOptional, uint32_t size)
{
    uint32_t pos = 0U;
    bool isNull;
    const char *out;    
    uint32_t outLen;
    uint32_t ret; = BLINK_DecodeString(&in[pos], inLen - pos, &out, &outLen, &isNull);
    
    if(ret > 0U){
        
        if(isNull && !isOptional)

            BLINK_ERROR("W5: string cannot be NULL")
        }
        else if(outLen > size){

            BLINK_ERROR("too big");
        }
        else{

            if(!isNull && (events->fnString != NULL)){

                self->events.fnString(self->user, out, outLen);
            }

            pos = ret;
        }
    }            

    return pos;
}

static uint32_t decodeBinary(const struct blink_decoder_events *events, const uint8_t *in, uint32_t inLen, bool isOptional, uint32_t size)
{
    uint32_t pos = 0U;
    bool isNull;
    const char *out;    
    uint32_t outLen;
    uint32_t ret = BLINK_DecodeBinary(&in[pos], inLen - pos, &out, &outLen, &isNull);
    
    if(ret > 0U){
        
        if(isNull && !isOptional)

            BLINK_ERROR("W5: string cannot be NULL")
        }
        else if(outLen > size){

            BLINK_ERROR("too big");
        }
        else{

            if(!isNull && (events->fnString != NULL)){

                self->events.fnString(self->user, out, outLen);
            }

            pos = ret;
        }
    }            

    return pos;
}

static uint32_t decodeDecimal(const struct blink_decoder_events *events, const uint8_t *in, uint32_t inLen, bool isOptional)
{
    bool isNull;
    uint32_t pos = 0U;
    int64_t mantissa;
    int8_t exponent;
    uint32_t ret = BLINK_DecodeDecimal(&in[pos], inLen - pos, &mantissa, &exponent, &isNull);
    
    if(ret > 0U){
        
        if(isNull && !isOptional){

            BLINK_ERROR("W5: decimal cannot be NULL")                
        }
        else{

            if(self->events.fnDecimal != NULL){

                self->events.fnDecimal(self->user, mantissa, exponent);
            }

            pos = ret;
        }
    }

    return pos;
}

static uint32_t decodeBool(const struct blink_decoder_events *events, const uint8_t *in, uint32_t inLen, bool isOptional)
{
    bool isNull;
    uint32_t pos = 0U;
    bool out;
    uint32_t ret = BLINK_DecodeBool(&in[pos], inLen - pos, &out, &isNull);

    if(ret > 0U){
        
        if(isNull && !isOptional){

            BLINK_ERROR("W5: bool cannot be NULL")                
        }
        else{

            if(self->events.fnDecimal != NULL){

                self->events.fnBool(self->user, out);
            }

            pos = ret;
        }
    }

    return pos;
}

static uint32_t decodeFixed(const struct blink_decoder_events *events, const uint8_t *in, uint32_t inLen, bool isOptional, uint32_t size)
{
    bool isNull = false;
    const char *out;    
    uint32_t outLen;
    uint32_t ret;
    
    if(isOptional){

        ret = BLINK_DecodeOptionalFixed(&in[pos], inLen - pos, &out, &outLen, &isNull);                                            
    }
    else{

        
        ret = BLINK_DecodeFixed(&in[pos], inLen - pos, &out, &outLen);                                            
    }

    if(ret > 0U){
        
        if(!isNull){

            if(self->events.fnString != NULL){

                self->events.fnString(self->user, out, outLen);
            }

            pos = ret;
        }
    }

    return pos;
}

static uint32_t decodeU8(const struct blink_decoder_events *events, const uint8_t *in, uint32_t inLen, bool isOptional)
{
    bool isNull;
    uint32_t pos = 0U;    
    uint8_t out;
    uint32_t ret = BLINK_DecodeU8(&in[pos], inLen - pos, &out, &isNull);
    
    if(ret > 0U){

        if(isNull && !isOptional){

            BLINK_ERROR("W5: u8 cannot be NULL")        
        }
        else{

            if(events->fnU8 != NULL){

                self->events.fnU8(self->user, out);
            }

            pos = ret;
        }
    }

    return pos;    
}

static uint32_t decodeU16(const struct blink_decoder_events *events, const uint8_t *in, uint32_t inLen, bool isOptional)
{
    bool isNull;
    uint32_t pos = 0U;    
    uint16_t out;
    uint32_t ret = BLINK_DecodeU16(&in[pos], inLen - pos, &out, &isNull);
    
    if(ret > 0U){

        if(isNull && !isOptional){

            BLINK_ERROR("W5: u16 cannot be NULL")        
        }
        else{

            if(events->fnU16 != NULL){

                self->events.fnU16(self->user, out);
            }

            pos = ret;
        }
    }

    return pos;    
}

static uint32_t decodeU32(const struct blink_decoder_events *events, const uint8_t *in, uint32_t inLen, bool isOptional)
{
    bool isNull;
    uint32_t pos = 0U;    
    uint32_t out;
    uint32_t ret = BLINK_DecodeU32(&in[pos], inLen - pos, &out, &isNull);
    
    if(ret > 0U){

        if(isNull && !isOptional){

            BLINK_ERROR("W5: u32 cannot be NULL")        
        }
        else{

            if(events->fnU32 != NULL){

                self->events.fnU32(self->user, out);
            }

            pos = ret;
        }
    }

    return pos;    
}

static uint32_t decodeU64(const struct blink_decoder_events *events, const uint8_t *in, uint32_t inLen, bool isOptional)
{
    bool isNull;
    uint32_t pos = 0U;    
    uint64_t out;
    uint32_t ret = BLINK_DecodeU64(&in[pos], inLen - pos, &out, &isNull);
    
    if(ret > 0U){

        if(isNull && !isOptional){

            BLINK_ERROR("W5: u64 cannot be NULL")        
        }
        else{

            if(events->fnU64 != NULL){

                self->events.fnU64(self->user, out);
            }

            pos = ret;
        }
    }

    return pos;    
}

static uint32_t decodeI8(const struct blink_decoder_events *events, const uint8_t *in, uint32_t inLen, bool isOptional)
{
    bool isNull;
    uint32_t pos = 0U;    
    int8_t out;
    uint32_t ret = BLINK_DecodeI8(&in[pos], inLen - pos, &out, &isNull);
    
    if(ret > 0U){

        if(isNull && !isOptional){

            BLINK_ERROR("W5: u8 cannot be NULL")        
        }
        else{

            if(events->fnI8 != NULL){

                self->events.fnI8(self->user, out);
            }

            pos = ret;
        }
    }

    return pos;    
}

static uint32_t decodeI16(const struct blink_decoder_events *events, const uint8_t *in, uint32_t inLen, bool isOptional)
{
    bool isNull;
    uint32_t pos = 0U;    
    int16_t out;
    uint32_t ret = BLINK_DecodeU16(&in[pos], inLen - pos, &out, &isNull);
    
    if(ret > 0U){

        if(isNull && !isOptional){

            BLINK_ERROR("W5: i16 cannot be NULL")        
        }
        else{

            if(events->fnI16 != NULL){

                self->events.fnI16(self->user, out);
            }

            pos = ret;
        }
    }

    return pos;    
}

static uint32_t decodeI32(const struct blink_decoder_events *events, const uint8_t *in, uint32_t inLen, bool isOptional)
{
    bool isNull;
    uint32_t pos = 0U;    
    uint32_t out;
    uint32_t ret = BLINK_DecodeI32(&in[pos], inLen - pos, &out, &isNull);
    
    if(ret > 0U){

        if(isNull && !isOptional){

            BLINK_ERROR("W5: i32 cannot be NULL")        
        }
        else{

            if(events->fnI32 != NULL){

                self->events.fnI32(self->user, out);
            }

            pos = ret;
        }
    }

    return pos;    
}

static uint32_t decodeI64(const struct blink_decoder_events *events, const uint8_t *in, uint32_t inLen, bool isOptional)
{
    bool isNull;
    uint32_t pos = 0U;    
    uint64_t out;
    uint32_t ret = BLINK_DecodeI64(&in[pos], inLen - pos, &out, &isNull);
    
    if(ret > 0U){

        if(isNull && !isOptional){

            BLINK_ERROR("W5: i64 cannot be NULL")        
        }
        else{

            if(events->fnI64 != NULL){

                self->events.fnI64(self->user, out);
            }

            pos = ret;
        }
    }

    return pos;    
}

static uint32_t decodeF64(const struct blink_decoder_events *events, const uint8_t *in, uint32_t inLen, bool isOptional)
{
    bool isNull;
    uint32_t pos = 0U;    
    double out;
    uint32_t ret = BLINK_DecodeF64(&in[pos], inLen - pos, &out, &isNull);
    
    if(ret > 0U){

        if(isNull && !isOptional){

            BLINK_ERROR("W5: f64 cannot be NULL")        
        }
        else{

            if(events->fnF64 != NULL){

                self->events.fnF64(self->user, out);
            }

            pos = ret;
        }
    }

    return pos;    
}


