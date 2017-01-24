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

bool BLINK_Stream_write(blink_stream_t self, const void *buf, size_t nbyte)
{
    BLINK_ASSERT(self != NULL)
    BLINK_ASSERT((nbyte == 0U) || (buf != NULL))

    bool retval = false;

    if(nbyte <= (size_t)INT32_MAX){

        switch(self->type){
        case BLINK_STREAM_BUFFER:
        
            if(self->value.buffer.out != NULL){
                if((self->value.buffer.max - self->value.buffer.pos) >= (uint32_t)nbyte){
                    
                    (void)memcpy(&self->value.buffer.out[self->value.buffer.pos], buf, nbyte);
                    self->value.buffer.pos += (uint32_t)nbyte;
                    retval = true;
                }
            }
            break;
            
        case BLINK_STREAM_USER:

            if(self->value.user.fn.write != NULL){

                retval = self->value.user.fn.write(self->value.user.state, buf, nbyte);
            }
            break;                
        
        default:
            /* no action */
            break;
        }
    }

    return retval;
}

bool BLINK_Stream_read(blink_stream_t self, void *buf, size_t nbyte)
{
    BLINK_ASSERT(self != NULL)
    BLINK_ASSERT((nbyte == 0U) || (buf != NULL))

    bool retval = false;

    if(nbyte <= (size_t)INT32_MAX){

        switch(self->type){
        case BLINK_STREAM_BUFFER:
        
            if(self->value.buffer.in != NULL){
                if((self->value.buffer.max - self->value.buffer.pos) >= (uint32_t)nbyte){
                    
                    (void)memcpy(buf, &self->value.buffer.in[self->value.buffer.pos], nbyte);
                    self->value.buffer.pos += (uint32_t)nbyte;
                    retval = true;
                }                
            }
            break;

        case BLINK_STREAM_USER:

            if(self->value.user.fn.read != NULL){

                retval = self->value.user.fn.read(self->value.user.state, buf, nbyte);
            }
            break;                
    
        default:
            /* no action */
            break;
        }
    }

    return retval;
}

bool BLINK_Stream_peek(blink_stream_t self, void *buf)
{
    BLINK_ASSERT(self != NULL)
    BLINK_ASSERT(buf != NULL)

    bool retval = false;

    switch(self->type){
    case BLINK_STREAM_BUFFER:
    
        if(self->value.buffer.in != NULL){
            
            if((self->value.buffer.max - self->value.buffer.pos) >= 1U){

                *((uint8_t *)buf) = self->value.buffer.in[self->value.buffer.pos];
                retval = true;                
            }
        }
        break;

    case BLINK_STREAM_USER:

        if(self->value.user.fn.peek != NULL){

            retval = self->value.user.fn.peek(self->value.user.state, buf);
        }
        break;
        
    default:
        /* no action */
        break;
    }
    

    return retval;
}

blink_stream_t BLINK_Stream_initBufferReadOnly(struct blink_stream *self, const void *buf, uint32_t max)
{
    BLINK_ASSERT(self != NULL)
    BLINK_ASSERT((max == 0) || (buf != NULL))

    blink_stream_t retval = NULL;

    if(max <= (size_t)INT32_MAX){

        (void)memset(self, 0, sizeof(*self));
        self->type = BLINK_STREAM_BUFFER;
        self->value.buffer.in = (uint8_t *)buf;        
        self->value.buffer.max = max;
        retval = (blink_stream_t)self;
    }

    return retval;
}

blink_stream_t BLINK_Stream_initBuffer(struct blink_stream *self, void *buf, uint32_t max)
{
    BLINK_ASSERT(self != NULL)
    BLINK_ASSERT((max == 0) || (buf != NULL))

    blink_stream_t retval = NULL;

    if(max <= (uint32_t)INT32_MAX){

        (void)memset(self, 0, sizeof(*self));
        self->type = BLINK_STREAM_BUFFER;
        self->value.buffer.out = buf;
        self->value.buffer.in = (uint8_t *)buf;
        self->value.buffer.max = max;
        retval = (blink_stream_t)self;
    }

    return retval;
}

blink_stream_t BLINK_Stream_initUser(struct blink_stream *self, void *state,  struct blink_stream_user fn)
{
    BLINK_ASSERT(self != NULL)

    (void)memset(self, 0, sizeof(*self));
    self->type = BLINK_STREAM_USER;
    self->value.user.state = state;
    self->value.user.fn = fn;    
    return (blink_stream_t)self;
}

uint32_t BLINK_Stream_tell(blink_stream_t self)
{
    BLINK_ASSERT(self != NULL)

    uint32_t retval = 0U;

    switch(self->type){
    case BLINK_STREAM_BUFFER:
    
        retval = (uint32_t)self->value.buffer.pos;
        break;

    case BLINK_STREAM_USER:

        if(self->value.user.fn.tell != NULL){

            retval = self->value.user.fn.tell(self->value.user.state);
        }
        break;
        
    default:
        /* no action */
        break;
    }    

    return retval;
}

bool BLINK_Stream_seekSet(blink_stream_t self, uint32_t offset)
{
    BLINK_ASSERT(self != NULL)

    bool retval = false;

    if(offset < (uint32_t)INT32_MAX){

        switch(self->type){
        case BLINK_STREAM_BUFFER:
            if(self->value.buffer.max >= offset){

                self->value.buffer.pos = offset;
                retval = true;
            }    
            break;
        case BLINK_STREAM_USER:
            if(self->value.user.fn.seekSet != NULL){

                retval = self->value.user.fn.seekSet(self->value.user.state, offset);
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
    BLINK_ASSERT(self != NULL)
    
    bool retval = false;
    
    switch(self->type){
    case BLINK_STREAM_BUFFER:
        retval = adjustOffset((int32_t *)&self->value.buffer.pos, (int32_t)self->value.buffer.max, offset);
        break;
    case BLINK_STREAM_USER:
        if(self->value.user.fn.seekCur != NULL){

            retval = self->value.user.fn.seekCur(self->value.user.state, offset);
        }
        break;
    default:
        /* no action */
        BLINK_ERROR("this stream cannot seek")
        break;
    }

    return retval;
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
