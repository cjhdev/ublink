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

    /*lint -e(9026) Allow functionality to be removed completely */
    #define ERROR(...) ;

    /*lint -e(9026) Allow functionality to be removed completely */
    #define TRACE(...) ;

    /*lint -e(9026) Allow functionality to be removed completely */
    #define ASSERT(X) ;

#else

    #include <assert.h>
    #include <stdio.h>

    /*lint -e(9026) Allow functionality to be removed completely */
    #define ERROR(...)  do{fprintf(stderr, __VA_ARGS__);fprintf(stderr, "\n");}while(0);

    /*lint -e(9026) Allow functionality to be removed completely */
    #define TRACE(...)  do{fprintf(stderr, "trace: %s(): ", __FUNCTION__);fprintf(stderr, __VA_ARGS__);fprintf(stderr, "\n");}while(0);

    /*lint -e(9026) Allow functionality to be removed completely */
    #define ASSERT(X)   /*lint -e(9034) Call to assert */assert((X));
    
#endif

#endif
