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
#ifndef BLINK_DEBUG_H
#define BLINK_DEBUG_H

#ifdef NDEBUG
    #define BLINK_ASSERT(X) /* BLINK_ASSERT(X) */
#else
    #include <assert.h>
    #define BLINK_ASSERT(X)   assert((X));
#endif

#ifdef BLINK_DEBUG_INCLUDE
BLINK_DEBUG_INCLUDE
#endif

#ifdef BLINK_NO_DEBUG_MESSAGE
    #define BLINK_ERROR(...) /* BLINK_ERROR(...) */
    #define BLINK_DEBUG(...)  /* BLINK_DEBUG(...) */
#endif

#if !defined(BLINK_ERROR) || !defined(BLINK_DEBUG)
    #include <stdio.h>    
#endif

#ifndef BLINK_ERROR
    #define BLINK_ERROR(...)  do{fprintf(stderr, __VA_ARGS__);fprintf(stderr, "\n");}while(0);    
#endif

#ifndef BLINK_DEBUG
    #define BLINK_DEBUG(...)  do{fprintf(stdout, "%s: ", __FUNCTION__);fprintf(stdout, __VA_ARGS__);fprintf(stdout, "\n");}while(0);
#endif

#endif
