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

/* functions **********************************************************/

bool BLINK_Stream_write(blink_stream_t self, const uint8_t *in, size_t bytesToWrite)
{
    bool retval = false;

    switch(self->type){
    case BLINK_STREAM_OUTPUT_BUFFER:
    {
        struct blink_stream_output_buffer *s = (struct blink_stream_output_buffer *)self;

        if((s->outMax - s->pos) >= bytesToWrite){
            
            (void)memcpy(&s->out[s->pos], in, bytesToWrite);
            s->pos += bytesToWrite;
            retval = true;
        }
        else{

            /* buffer too short */
            BLINK_ERROR("EOF")
        }        
    }
        break;
    default:
        /* no action */
        break;
    }

    return retval;
}

bool BLINK_Stream_read(blink_stream_t self, uint8_t *out, size_t bytesToRead)
{
    bool retval = false;

    switch(self->type){
    case BLINK_STREAM_INPUT_BUFFER:
    {
        struct blink_stream_input_buffer *s = (struct blink_stream_input_buffer *)self;

        if((s->inLen - s->pos) >= bytesToRead){
            
            (void)memcpy(out, &s->in[s->pos], bytesToRead);
            s->pos += bytesToRead;
            retval = true;
        }
        else{

            /* buffer too short */
            BLINK_ERROR("EOF")
        }        
    }
        break;
    default:        
        /* no action */
        break;
    }

    return retval;
}

blink_stream_t BLINK_Stream_initInputBuffer(struct blink_stream_input_buffer *self, const uint8_t *in, size_t inLen)
{
    BLINK_ASSERT(self != NULL)
    BLINK_ASSERT((inLen == 0) || (in != NULL))

    (void)memset(self, 0, sizeof(*self));
    self->type.type = BLINK_STREAM_INPUT_BUFFER;
    self->in = in;
    self->inLen = inLen;

    return (blink_stream_t)self;
}

blink_stream_t BLINK_Stream_initOutputBuffer(struct blink_stream_output_buffer *self, uint8_t *out, size_t outMax)
{
    BLINK_ASSERT(self != NULL)
    BLINK_ASSERT((outMax == 0) || (out != NULL))

    (void)memset(self, 0, sizeof(*self));
    self->type.type = BLINK_STREAM_OUTPUT_BUFFER;
    self->out = out;
    self->outMax = outMax;

    return (blink_stream_t)self;
}

