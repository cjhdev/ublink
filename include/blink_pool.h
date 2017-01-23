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

#ifndef BLINK_POOL_H
#define BLINK_POOL_H

/**
 * @defgroup blink_pool blink_pool
 * @ingroup ublink
 *
 * A linear memory allocator for uBlink modules.
 *
 * ### Example Workflow
 *
 * A blink_pool must be initialised (or reinitialised) before use:
 *
 * @code
 * uint8_t heap[1024U];
 * struct blink_pool pool;
 * (void)BLINK_PoolInit(&pool, heap, sizeof(heap));
 * @endcode
 *
 * Allocate a word aligned block:
 *
 * @code
 * long *memory = (long *)BLINK_Pool_calloc(&pool, 42U);
 *
 * // a null pointer means the pool is exhausted
 * assert(memory != NULL);
 *
 * // note that BLINK_Pool_calloc interface will return zeroed memory
 * assert(*memory == 0);
 * 
 * *memory = 42;
 * @endcode
 *
 * Find out how much memory can still be allocated:
 * 
 * @code
 * printf("free space: %u\n", BLINK_Pool_getFreeSpace(&pool)); 
 * @endcode
 * 
 * @{
 * */

#ifdef __cplusplus
extern "C" {
#endif

/* includes ***********************************************************/

#include <stdint.h>
#include <stddef.h>

/* types **************************************************************/

struct blink_pool {
    uint8_t *heap;              /**< pointer to assigned heap */
    size_t size;                /**< size of heap in bytes */
    size_t pos;                 /**< free memory offset */
};

/** This type shall be used by uBlink modules to refer to initialised pools */ 
typedef struct blink_pool * blink_pool_t;

/* function prototypes ************************************************/

/** This function will initialise a blink_pool_static structure.
 *
 * @param[in] self
 * 
 * @param[in] heap
 *
 * A pointer to word aligned memory block that will be used as the heap.
 * 
 * @param[in] size
 *
 * The size of the heap memory block in bytes.
 *
 * @return intialised blink_pool_static
 *
 * */
blink_pool_t BLINK_Pool_init(struct blink_pool *self, uint8_t *heap, size_t size);

/** Allocate word aligned memory from the pool
 *
 * @note memory shall be set to zero at allocation time
 * 
 * @param[in] self initialsed pool
 * @param[in] size number of bytes to allocate
 *
 * @return pointer to allocated memory
 *
 * @retval NULL insufficient free memory
 *
 * */
void *BLINK_Pool_calloc(blink_pool_t self, size_t size);

/** Discover how much free memory is left in the pool
 * 
 * @param[in] self initialised pool
 *
 * @return free space in bytes
 *
 * */
size_t BLINK_Pool_getFreeSpace(blink_pool_t self);

#ifdef __cplusplus
}
#endif

/** @} */
#endif
