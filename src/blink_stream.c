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

#include "blink_stream.h"
#include "blink_debug.h"

#include <string.h>

/* static function prototypes *****************************************/

static bool adjustOffset(int32_t *pos, int32_t max, int32_t offset);

/* functions **********************************************************/

bool BLINK_Stream_write(blink_stream_t self, const void *in, size_t bytesToWrite)
{
    BLINK_ASSERT(self != NULL)
    BLINK_ASSERT((bytesToWrite == 0U) || (in != NULL))

    bool retval = false;

    if(bytesToWrite <= (size_t)INT32_MAX){

        switch(self->type){
        case BLINK_STREAM_BUFFER:
        
            if(self->value.buffer.out != NULL){
                if((self->value.buffer.max - self->value.buffer.pos) >= (uint32_t)bytesToWrite){
                    
                    (void)memcpy(&self->value.buffer.out[self->value.buffer.pos], in, bytesToWrite);
                    self->value.buffer.pos += (uint32_t)bytesToWrite;
                    retval = true;
                }
            }
            break;
            
        case BLINK_STREAM_USER:

            if(self->value.user.writer != NULL){

                retval = self->value.user.writer(self->value.user.state, in, bytesToWrite);
            }
            break;                
        
        default:
            /* no action */
            break;
        }
    }

    return retval;
}

bool BLINK_Stream_read(blink_stream_t self, void *out, size_t bytesToRead)
{
    BLINK_ASSERT(self != NULL)
    BLINK_ASSERT((bytesToRead == 0U) || (out != NULL))

    bool retval = false;

    if(bytesToRead <= (size_t)INT32_MAX){

        switch(self->type){
        case BLINK_STREAM_BUFFER:
        
            if(self->value.buffer.in != NULL){
                if((self->value.buffer.max - self->value.buffer.pos) >= (uint32_t)bytesToRead){
                    
                    (void)memcpy(out, &self->value.buffer.in[self->value.buffer.pos], bytesToRead);
                    self->value.buffer.pos += (uint32_t)bytesToRead;
                    retval = true;
                }                
            }
            break;

        case BLINK_STREAM_USER:

            if(self->value.user.reader != NULL){

                retval = self->value.user.reader(self->value.user.state, out, bytesToRead);
            }
            break;                
    
        default:
            /* no action */
            break;
        }
    }

    return retval;
}

bool BLINK_Stream_peek(blink_stream_t self, void *out)
{
    BLINK_ASSERT(self != NULL)
    BLINK_ASSERT(out != NULL)

    bool retval = false;

    switch(self->type){
    case BLINK_STREAM_BUFFER:
    
        if(self->value.buffer.in != NULL){
            
            if((self->value.buffer.max - self->value.buffer.pos) >= 1U){

                *((uint8_t *)out) = self->value.buffer.in[self->value.buffer.pos];
                retval = true;                
            }
        }
        break;

    case BLINK_STREAM_USER:
    default:
        /* no action */
        break;
    }
    

    return retval;
}

blink_stream_t BLINK_Stream_initBufferReadOnly(struct blink_stream *self, const uint8_t *buf, uint32_t max)
{
    BLINK_ASSERT(self != NULL)
    BLINK_ASSERT((max == 0) || (buf != NULL))

    blink_stream_t retval = NULL;

    if(max <= (size_t)INT32_MAX){

        (void)memset(self, 0, sizeof(*self));
        self->type = BLINK_STREAM_BUFFER;
        self->value.buffer.in = buf;        
        self->value.buffer.max = max;
        retval = (blink_stream_t)self;
    }

    return retval;
}

blink_stream_t BLINK_Stream_initBuffer(struct blink_stream *self, uint8_t *buf, uint32_t max)
{
    BLINK_ASSERT(self != NULL)
    BLINK_ASSERT((max == 0) || (buf != NULL))

    blink_stream_t retval = NULL;

    if(max <= (uint32_t)INT32_MAX){

        (void)memset(self, 0, sizeof(*self));
        self->type = BLINK_STREAM_BUFFER;
        self->value.buffer.out = buf;
        self->value.buffer.in = buf;
        self->value.buffer.max = max;
        retval = (blink_stream_t)self;
    }

    return retval;
}

blink_stream_t BLINK_Stream_initUser(struct blink_stream *self, void *state, blink_stream_read_t reader, blink_stream_write_t writer)
{
    BLINK_ASSERT(self != NULL)

    (void)memset(self, 0, sizeof(*self));
    self->type = BLINK_STREAM_USER;
    self->value.user.state = state;
    self->value.user.reader = reader;
    self->value.user.writer = writer;
    return (blink_stream_t)self;
}

uint32_t BLINK_Stream_tell(blink_stream_t self)
{
    uint32_t retval = 0U;
    
    switch(self->type){
    case BLINK_STREAM_BUFFER:
        retval = (uint32_t)self->value.buffer.pos;
        break;
    default:
        /* no action */
        break;
    }

    return retval;
}

bool BLINK_Stream_seekSet(blink_stream_t self, uint32_t offset)
{
    bool retval = false;

    if(offset < (uint32_t)INT32_MAX){

        switch(self->type){
        case BLINK_STREAM_BUFFER:
            if(self->value.buffer.max >= offset){

                self->value.buffer.pos = offset;
                retval = true;
            }    
            break;
        default:
            /* no action */
            BLINK_ERROR("this stream cannot seek")
            break;
        }
    }

    return retval;
}

bool BLINK_Stream_seekCur(blink_stream_t self, int32_t offset)
{
    bool retval = false;
    
    switch(self->type){
    case BLINK_STREAM_BUFFER:
        retval = adjustOffset((int32_t *)&self->value.buffer.pos, (int32_t)self->value.buffer.max, offset);
        break;
    default:
        /* no action */
        BLINK_ERROR("this stream cannot seek")
        break;
    }

    return retval;
}

blink_stream_t BLINK_Stream_dup(struct blink_stream *self, blink_stream_t existing)
{
    BLINK_ASSERT(self != NULL)
    BLINK_ASSERT(existing != NULL)

    (void)memset(self, 0, sizeof(*self));

    *self = *existing;

    return self;
}

/* static functions ***************************************************/

static bool adjustOffset(int32_t *pos, int32_t max, int32_t offset)
{
    bool retval = false;

    if(offset > 0){

        if(((*pos + offset) > *pos) && ((*pos + offset) <= max)){

            *pos += offset;
            retval = true;
        }
    }
    else if(offset > 0){
    
        if((*pos - offset) < *pos){

            *pos -= offset;
            retval = true;
        }
    }
    else{

        retval = true;
    }

    return retval;
}
