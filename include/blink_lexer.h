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

#ifndef BLINK_LEXER_H
#define BLINK_LEXER_H

/**
 * @defgroup blink_lexer Lexer
 *
 * Convert syntax to a token.
 * 
 * @{
 * */

#ifdef __cplusplus
extern "C" {
#endif

/* includes ***********************************************************/

#include <stddef.h>
#include <stdint.h>

/* enums **************************************************************/

/** The following tokens are returned by #BLINK_GetToken */
enum blink_token {

    T_EQUAL,            /**< `=` */
    T_COMMA,            /**< `,` */
    T_PERIOD,           /**< `. ` */
    T_QUESTION,         /**< `?` */    
    T_LEFT_BRACKET,     /**< `[` */
    T_RIGHT_BRACKET,    /**< `]` */
    T_LEFT_PAREN,       /**< `(` */
    T_RIGHT_PAREN,      /**< `)` */
    T_STAR,             /**< `*` */
    T_BAR,              /**< `|` */
    T_SLASH,            /**< `/` */
    T_AMP,              /**< `@` */
    T_COLON,            /**< `:` */
    
    T_RIGHT_ARROW,      /**< `->` */
    T_LEFT_ARROW,       /**< `<-` */

    T_BOOLEAN           /**< `bool` */
    T_I8,               /**< `i8` */
    T_I16,              /**< `i16` */
    T_I32,              /**< `i32` */
    T_I64,              /**< `i64` */
    T_U8,               /**< `u8` */
    T_U16,              /**< `u16` */
    T_U32,              /**< `u32` */
    T_U64,              /**< `u64` */
    T_F64,              /**< `f64` */
    T_DECIMAL,          /**< `decimal` */    
    T_DATE,             /**< `date` */
    T_MILLI_TIME,       /**< `milltime` */
    T_NANO_TIME,        /**< `nanotime` */
    T_MILLI_TIME_OF_DAY,    /**< `milliTimeOfDay` */
    T_NANO_TIME_OF_DAY,     /**< `nanoTimeOfDay` */
    T_OBJECT,               /**< `object` */
    T_FIXED,                /**< `fixed` */

    T_NUMBER,           /**< `0x[0-9a-fA-F][0-9a-fA-F]+ | [0-9]+` */
    T_NAME,             /**< `[\\]?[A-Za-z_][A-Za-z_0-9]+` */
    T_QNAME,            /**< `[A-Za-z_][A-Za-z_0-9]+:[A-Za-z_][A-Za-z_0-9] | [\\]?[A-Za-z_][A-Za-z_0-9]+` */
                        
    T_UNKNOWN,          /**< <no match> */
    T_EOF               /**< <end of file> */
};

/* unions *************************************************************/

/** Some tokens have a value returned by #BLINK_GetToken */
union blink_token_value {    
    /** Initialised for #T_NAME and #T_QNAME */
    struct {
        const char *ptr;    /**< pointer to printable string */
        size_t len;         /**< byte length of `ptr` */
    } literal;
    /** Initialised for #T_NUMBER */
    uint64_t number;            
};

/* function prototypes ************************************************/

/** Return next token syntax
 *
 * @param[in] in input buffer
 * @param[in] inLen byte length of `in`
 * @param[out] read bytes read from `in`
 * @param[out] value value of token (only initialised for T_NAME, T_QNAME, T_NUMBER)
 *
 * @return token parsed
 *
 * @see enum blink_token
 *
 * */
enum blink_token BLINK_GetToken(const char *in, size_t inLen, size_t *read, union blink_token_value *value);

#ifdef __cplusplus
}
#endif

/** @} */
#endif
