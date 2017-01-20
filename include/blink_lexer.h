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
 * @defgroup blink_lexer blink_lexer
 * @ingroup ublink
 *
 * Used by blink_schema to convert schema syntax into tokens and values.
 * 
 * @{
 * */

#ifdef __cplusplus
extern "C" {
#endif

/* includes ***********************************************************/

#include <stdint.h>

/* types **************************************************************/

enum blink_token {
    TOK_STRING = 0,         /**< `string` */
    TOK_BINARY,             /**< `binary` */
    TOK_FIXED,              /**< `fixed` */
    TOK_BOOL,               /**< `bool` */            
    TOK_U8,                 /**< `u8` */
    TOK_U16,                /**< `u16` */
    TOK_U32,                /**< `u32` */
    TOK_U64,                /**< `u64` */
    TOK_I8,                 /**< `i8` */
    TOK_I16,                /**< `i16` */
    TOK_I32,                /**< `i32` */
    TOK_I64,                /**< `i64` */
    TOK_F64,                /**< `f64` */
    TOK_DATE,               /**< `date` */
    TOK_TIME_OF_DAY_MILLI,  /**< `timeOfDayMilli` */
    TOK_TIME_OF_DAY_NANO,   /**< `timeOfDayNano` */
    TOK_MILLI_TIME,         /**< `milltime` */
    TOK_NANO_TIME,          /**< `nanotime` */    
    TOK_DECIMAL,            /**< `decimal` */
    TOK_OBJECT,             /**< `object` */
    TOK_NAME,               /**< `[\\]?[A-Za-z_][A-Za-z_0-9]+` */
    TOK_CNAME,              /**< `[A-Za-z_][A-Za-z_0-9]+:[A-Za-z_][A-Za-z_0-9]` */                        
    TOK_EQUAL,              /**< `=` */
    TOK_COMMA,              /**< `,` */
    TOK_PERIOD,             /**< <period> */
    TOK_QUESTION,           /**< `?` */    
    TOK_LBRACKET,           /**< `[` */
    TOK_RBRACKET,           /**< `]` */
    TOK_LPAREN,             /**< `(` */
    TOK_RPAREN,             /**< `)` */
    TOK_STAR,               /**< `*` */
    TOK_BAR,                /**< `|` */
    TOK_SLASH,              /**< `/` */
    TOK_AT,                 /**< `@` */
    TOK_COLON,              /**< `:` */    
    TOK_RARROW,             /**< `->` */
    TOK_LARROW,             /**< `<-` */
    TOK_NAMESPACE,          /**< `namespace` */
    TOK_SCHEMA,             /**< `schema` */    
    TOK_TYPE,               /**< `type` */    
    TOK_UINT,               /**< <uint> `[0][x][0-9a-fA-F][0-9a-fA-F]+ | [0-9]+` */
    TOK_INT,                /**< <int> `[-][1-9][0-9]*` */
    TOK_LITERAL,            /**< <literal> a string within double or single quotation marks */
    TOK_UNKNOWN,            /**< <unknown> no match */
    TOK_EOF                 /**< <eof> end of file */    
};

union blink_token_value {    
    struct {
        const char *ptr;    /**< pointer to printable string */
        size_t len;         /**< byte length of `ptr` */
    } literal;
    /** Initialised for #TOK_UINT */
    uint64_t number;
    /** Initialised for #TOK_INT */
    int64_t signedNumber;            
};

struct blink_token_location {
    size_t row;
    size_t col;
};

/* function prototypes ************************************************/

/** Use this function to convert an octet string into a token
 *
 * @code
 * const char input[] = "emptyGroup";
 * size_t read;
 * union blink_token_value value;
 *
 * // will return TOK_NAME
 * enum blink_token tok = BLINK_GetToken(input, sizeof(input), &read, &value, NULL);
 * @endcode
 * 
 * @param[in] in input to convert into a token
 * @param[in] inLen byte length of input
 * @param[out] read bytes read from input
 * @param[out] value value of token (only initialised for TOK_NAME, TOK_CNAME, TOK_UINT, TOK_INT, TOK_LITERAL)
 * @param[out] location optional location information
 *
 * @return token
 *
 * */
enum blink_token BLINK_Lexer_getToken(const char *in, size_t inLen, size_t *read, union blink_token_value *value, struct blink_token_location *location);

/** Convert a token enum to a string representation
 * 
 * @param[in] token token to convert to string
 * @return null terminated string
 *
 * */
const char *BLINK_Lexer_tokenToString(enum blink_token token);

#ifdef __cplusplus
}
#endif

/** @} */
#endif
