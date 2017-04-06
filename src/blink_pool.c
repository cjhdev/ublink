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

#include "blink_pool.h"
#include "blink_debug.h"

#include <string.h>

/* functions **********************************************************/

blink_pool_t BLINK_Pool_init(struct blink_pool *self, uint8_t *heap, size_t size)
{
    BLINK_ASSERT(self != NULL)
    BLINK_ASSERT(heap != NULL)
    BLINK_ASSERT(((size_t)heap % sizeof(long)) == 0U)

    (void)memset(self, 0, sizeof(*self));
    (void)memset(heap, 0, size);
    self->heap = heap;
    self->size = size;

    return self;
}

void *BLINK_Pool_calloc(blink_pool_t self, size_t size)
{
    BLINK_ASSERT(self != NULL)

    void *retval = NULL;
    size_t actualSize = size + (((size % sizeof(long)) == 0) ? 0 : (sizeof(long) - (size % sizeof(long))));

    if(size > 0){

        if((self->size - self->pos) >= actualSize){

            retval = &self->heap[self->pos];
            self->pos += actualSize;

        }
        else{

            /* calloc() */
            BLINK_DEBUG("insufficient memory (asking for %lu of a possible %lu bytes)", actualSize, BLINK_Pool_getFreeSpace(self))
        }
    }
    
    return retval;
}

size_t BLINK_Pool_getFreeSpace(blink_pool_t self)
{
    BLINK_ASSERT(self != NULL)

    return (self->size - self->pos);
}
