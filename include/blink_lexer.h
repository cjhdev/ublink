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
 * @defgroup blink_lexer
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
    TOK_STRING = 0,         /**< `string` */
    TOK_BINARY,             /**< `binary` */
    TOK_FIXED,            /**< `fixed` */
    TOK_BOOL,             /**< `bool` */            
    TOK_U8,               /**< `u8` */
    TOK_U16,              /**< `u16` */
    TOK_U32,              /**< `u32` */
    TOK_U64,              /**< `u64` */
    TOK_I8,               /**< `i8` */
    TOK_I16,              /**< `i16` */
    TOK_I32,              /**< `i32` */
    TOK_I64,              /**< `i64` */
    TOK_F64,              /**< `f64` */
    TOK_DATE,             /**< `date` */
    TOK_TIME_OF_DAY_MILLI,    /**< `timeOfDayMilli` */
    TOK_TIME_OF_DAY_NANO,     /**< `timeOfDayNano` */
    TOK_MILLI_TIME,       /**< `milltime` */
    TOK_NANO_TIME,        /**< `nanotime` */    
    TOK_DECIMAL,          /**< `decimal` */
    TOK_OBJECT,           /**< `object` */
    TOK_NAME,             /**< `[\\]?[A-Za-z_][A-Za-z_0-9]+` */
    TOK_CNAME,            /**< `[A-Za-z_][A-Za-z_0-9]+:[A-Za-z_][A-Za-z_0-9]` */                        
    TOK_EQUAL,            /**< `=` */
    TOK_COMMA,            /**< `,` */
    TOK_PERIOD,           /**< period */
    TOK_QUESTION,         /**< `?` */    
    TOK_LBRACKET,         /**< `[` */
    TOK_RBRACKET,         /**< `]` */
    TOK_LPAREN,           /**< `(` */
    TOK_RPAREN,           /**< `)` */
    TOK_STAR,             /**< `*` */
    TOK_BAR,              /**< `|` */
    TOK_SLASH,            /**< `/` */
    TOK_AT,               /**< `@` */
    TOK_COLON,            /**< `:` */    
    TOK_RARROW,           /**< `->` */
    TOK_LARROW,           /**< `<-` */
    TOK_NAMESPACE,        /**< `namespace` */
    TOK_SCHEMA,           /**< `schema` */    
    TOK_TYPE,             /**< `type` */    
    TOK_NUMBER,           /**< `0x[0-9a-fA-F][0-9a-fA-F]+ | [0-9]+` */
    TOK_LITERAL,          /**< a string within double or single quotation marks */
    TOK_UNKNOWN,          /**< no match */
    TOK_EOF              /**< end of file */    
};

/* unions *************************************************************/

/** Some tokens have a value returned by #BLINK_GetToken */
union blink_token_value {    
    /** Initialised for #TOK_NAME, #TOK_CNAME, and #TOK_LITERAL */
    struct {
        const char *ptr;    /**< pointer to printable string */
        size_t len;         /**< byte length of `ptr` */
    } literal;
    /** Initialised for #TOK_NUMBER */
    uint64_t number;            
};

/* structs ************************************************************/

struct blink_token_location {
    size_t row;
    size_t col;
};

/* function prototypes ************************************************/

/** Return next token from syntax
 * 
 * @param[in] in input buffer
 * @param[in] inLen byte length of `in`
 * @param[out] read bytes read from `in`
 * @param[out] value value of token (only initialised for TOK_NAME, TOK_CNAME, TOK_NUMBER)
 * @param[out] location optional location information
 *
 * @return enum #blink_token
 *
 * */
enum blink_token BLINK_GetToken(const char *in, size_t inLen, size_t *read, union blink_token_value *value, struct blink_token_location *location);

/** Try to convert a token to its string representation
 *
 * @note not all tokens can be converted to strings (e.g. #TOK_EOF)
 * 
 * @param[in] token token to convert to string
 * @param[out] len byte length of string
 *
 * @return pointer to token string
 *
 * @retval NULL no string exists for this token
 *
 * */
const char *BLINK_TokenToString(enum blink_token token, size_t *len);

#ifdef __cplusplus
}
#endif

/** @} */
#endif
